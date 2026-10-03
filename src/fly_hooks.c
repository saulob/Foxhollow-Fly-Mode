#include "fly_mode.h"

#include <string.h>

typedef struct Symbol {
  const char* name;
  void** address;
} Symbol;

typedef void (*HookFn)(void);

typedef struct Hook {
  const char* name;
  HookFn replacement;
  void** original;
  void* target;
} Hook;

FlyModeGame game;

static void (*origPlayerUpdateVelocityFromMotion)(GameObject*, PlayerState*, void*, float);
static void (*origPlayerUpdateKnockbackTimers)(GameObject*, PlayerState*);

/* The cheat menu ran Fly Mode from playerUpdate right after this call, before
   the velocity clamp and objMove. DR_EarthWarrior also calls it for its own
   object, which flyModeUpdate ignores because it is not the player. */
static void hookPlayerUpdateVelocityFromMotion(GameObject* obj, PlayerState* state, void* baddie, float dt) {
  origPlayerUpdateVelocityFromMotion(obj, state, baddie, dt);
  flyModeUpdate(obj);
}

/* playerUpdate is the only caller, shortly after the main objMove. */
static void hookPlayerUpdateKnockbackTimers(GameObject* obj, PlayerState* state) {
  flyModeAfterMove(obj);
  origPlayerUpdateKnockbackTimers(obj, state);
}

static const Symbol kSymbols[] = {
    {"getGameState", (void**)&game.getGameState},
    {"getCurUiDll", (void**)&game.getCurUiDll},
    {"getSaveGameLoadStatus", (void**)&game.getSaveGameLoadStatus},
    {"Obj_GetPlayerObject", (void**)&game.Obj_GetPlayerObject},
    {"getArwing", (void**)&game.getArwing},
    {"getCurSeqNo", (void**)&game.getCurSeqNo},
    {"isInBounds", (void**)&game.isInBounds},
    {"getCurMapLayer", (void**)&game.getCurMapLayer},
    {"playerTeleport", (void**)&game.playerTeleport},
    {"objSetPos", (void**)&game.objSetPos},
    {"Obj_SetParent", (void**)&game.Obj_SetParent},
    {"gArrivedWarpIndex", (void**)&game.gArrivedWarpIndex},
    {"gPendingWarpIndex", (void**)&game.gPendingWarpIndex},
    {"gGameLoopPendingMapId", (void**)&game.gGameLoopPendingMapId},
    {"timeDelta", (void**)&game.timeDelta},
};

static Hook sHooks[] = {
    {"playerUpdateVelocityFromMotion", (HookFn)hookPlayerUpdateVelocityFromMotion,
     (void**)&origPlayerUpdateVelocityFromMotion, NULL},
    {"playerUpdateKnockbackTimers", (HookFn)hookPlayerUpdateKnockbackTimers, (void**)&origPlayerUpdateKnockbackTimers,
     NULL},
};

#define COUNT_OF(array) ((int)(sizeof(array) / sizeof((array)[0])))

static int resolve_symbols(FhMod* mod, const FhModHost* host) {
  int ok = 1;
  int i;

  for (i = 0; i < COUNT_OF(kSymbols); i++) {
    *kSymbols[i].address = host->symbolAddress(mod, kSymbols[i].name);
    if (*kSymbols[i].address == NULL) {
      modLog(FH_LOG_ERROR, "could not resolve %s", kSymbols[i].name);
      ok = 0;
    }
  }
  return ok;
}

/* Only targets this mod patched are restored. A hook the host fails to remove
   keeps its original, so the still patched entry passes straight through
   while Fly Mode is off. */
static int remove_hooks(FhMod* mod, const FhModHost* host) {
  int ok = 1;
  int i;

  for (i = COUNT_OF(sHooks) - 1; i >= 0; i--) {
    if (sHooks[i].target == NULL) {
      continue;
    }
    if (host->hookRemove(mod, sHooks[i].target) != FH_MOD_OK) {
      modLog(FH_LOG_WARN, "could not unhook %s", sHooks[i].name);
      ok = 0;
      continue;
    }
    sHooks[i].target = NULL;
    *sHooks[i].original = NULL;
  }
  return ok;
}

/* The hooks go in only after every symbol resolved, and come out again if one
   fails, so a failed start never leaves a patched player function behind. */
int flyHooksInstall(FhMod* mod, const FhModHost* host) {
  int i;

  if (!resolve_symbols(mod, host)) {
    return 0;
  }
  for (i = 0; i < COUNT_OF(sHooks); i++) {
    void* target = host->symbolAddress(mod, sHooks[i].name);

    if (target == NULL) {
      modLog(FH_LOG_ERROR, "could not resolve %s", sHooks[i].name);
      remove_hooks(mod, host);
      return 0;
    }
    if (host->hookInstall(mod, target, (void*)sHooks[i].replacement, sHooks[i].original) != FH_MOD_OK) {
      modLog(FH_LOG_ERROR, "could not hook %s (no patch pad, or another mod already hooked it)", sHooks[i].name);
      *sHooks[i].original = NULL;
      remove_hooks(mod, host);
      return 0;
    }
    sHooks[i].target = target;
  }
  return 1;
}

void flyHooksRemove(FhMod* mod, const FhModHost* host) {
  if (remove_hooks(mod, host)) {
    memset(&game, 0, sizeof(game));
  }
}
