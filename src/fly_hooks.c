#include "fly_mode.h"

#include <string.h>

typedef struct Symbol {
  const char* name;
  void** address;
} Symbol;

FlyModeGame game;

static void* sHookTarget;
static void (*origPlayerUpdateVelocityFromMotion)(GameObject*, PlayerState*, void*, float);

/* The cheat menu ran Fly Mode from playerUpdate right after this call, before
   the velocity clamp and objMove. DR_EarthWarrior also calls it for its own
   object, which flyModeUpdate ignores because it is not the player. */
static void hookPlayerUpdateVelocityFromMotion(GameObject* obj, PlayerState* state, void* baddie, float dt) {
  origPlayerUpdateVelocityFromMotion(obj, state, baddie, dt);
  flyModeUpdate(obj);
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

/* The hook goes in only after every symbol resolved, so a failed start never
   leaves a patched player function behind. */
int flyHooksInstall(FhMod* mod, const FhModHost* host) {
  void* target;

  if (!resolve_symbols(mod, host)) {
    return 0;
  }
  target = host->symbolAddress(mod, "playerUpdateVelocityFromMotion");
  if (target == NULL) {
    modLog(FH_LOG_ERROR, "could not resolve playerUpdateVelocityFromMotion");
    return 0;
  }
  if (host->hookInstall(mod, target, (void*)hookPlayerUpdateVelocityFromMotion,
                        (void**)&origPlayerUpdateVelocityFromMotion) != FH_MOD_OK) {
    modLog(FH_LOG_ERROR,
           "could not hook playerUpdateVelocityFromMotion (no patch pad, or another mod already hooked it)");
    origPlayerUpdateVelocityFromMotion = NULL;
    return 0;
  }
  sHookTarget = target;
  return 1;
}

/* A hook the host fails to remove keeps its original, so the still patched
   entry passes straight through while Fly Mode is off. */
void flyHooksRemove(FhMod* mod, const FhModHost* host) {
  if (sHookTarget != NULL) {
    if (host->hookRemove(mod, sHookTarget) != FH_MOD_OK) {
      modLog(FH_LOG_WARN, "could not unhook playerUpdateVelocityFromMotion");
      return;
    }
    sHookTarget = NULL;
    origPlayerUpdateVelocityFromMotion = NULL;
  }
  memset(&game, 0, sizeof(game));
}
