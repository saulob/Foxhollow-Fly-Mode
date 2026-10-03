#include "fly_mode.h"

#include <string.h>

#define GAME_STATE_RUNNING 1
#define UI_DLL_GAMEPLAY 1
#define UI_DLL_FRONTEND_FIRST 2
#define UI_DLL_FRONTEND_LAST 7

#define PLAYER_MODE_IDLE 1
#define PLAYER_MODE_MOVING 2
#define PLAYER_MODE_ON_CLOUDRUNNER 0x1a
#define PLAYER_FLAG_TELEPORT_HOLD 0x4000
#define PLAYER_FLAG_LOCKED 0x200000
#define OBJECT_OBJFLAG_PARENT_SLACK 0x1000
#define CAMERA_MODE_VIEWFINDER_RESOURCE_ID 0x44
#define CAMERA_MODE_WORLD_MAP_RESOURCE_ID 0x4E

#define FLY_VERTICAL_SPEED 1.25f
#define FLY_SWIM_SURFACE_DEPTH 22.0f
#define NO_FLOOR_Y -1e+05f

typedef struct SafePosition {
  int valid;
  Vec3f pos;
  Vec3s rot;
  int swimming;
  int mapId;
  int layer;
} SafePosition;

static const int16_t sFlyCombatModes[] = {0x1f, 0x23, 0x24, 0x25, 0x26, 0x27, 0x37,
                                          0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d};

static int sSessionActive;
static int sFly;
static int sFlyInput;
static int sFlyUpArmed;
static int sFlyDownArmed;
static int sFlyToggleWasDown;
static int sFlyReturnWasDown;
static SafePosition sSafe;

int flyModeGameplayActive(void) {
  return game.getGameState() == GAME_STATE_RUNNING && game.getCurUiDll() == UI_DLL_GAMEPLAY &&
         game.getSaveGameLoadStatus() == 0 && (game.Obj_GetPlayerObject() != NULL || game.getArwing() != NULL);
}

static int arwing_active(void) { return flyModeGameplayActive() && game.getArwing() != NULL; }

static void set_fly(int enabled) {
  sFly = enabled != 0;
  sFlyInput = 0;
  sFlyUpArmed = 0;
  sFlyDownArmed = 0;
}

/* Fly Mode belongs to the loaded save: it is cleared once the game stops
   running or a front-end screen (title, save select) takes over. */
void flyModeUpdateSession(void) {
  int uiDll;

  if (flyModeGameplayActive()) {
    sSessionActive = 1;
    return;
  }
  uiDll = game.getCurUiDll();
  if (sSessionActive && (game.getGameState() != GAME_STATE_RUNNING ||
                         (uiDll >= UI_DLL_FRONTEND_FIRST && uiDll <= UI_DLL_FRONTEND_LAST))) {
    sSessionActive = 0;
    if (sFly) {
      modLog(FH_LOG_INFO, "Disabled (save session ended)");
    }
    set_fly(0);
    sSafe.valid = 0;
  }
}

static int player_controllable_on_foot(GameObject* player, PlayerState* st) {
  if (!flyModeGameplayActive() || arwing_active() || st->controlMode == PLAYER_MODE_ON_CLOUDRUNNER) {
    return 0;
  }
  if (game.getCurSeqNo() != 0) {
    return 0;
  }
  if ((st->flags360 & PLAYER_FLAG_LOCKED) != 0 || st->characterId == -1) {
    return 0;
  }
  if (st->playerStatus == NULL || st->playerStatus->health <= 0) {
    return 0;
  }
  if (st->heldObj != NULL || st->verticalVel != 0.0f) {
    return 0;
  }
  return (player->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0;
}

static int safe_position_warp_active(void) { return *game.gArrivedWarpIndex != -1 || *game.gPendingWarpIndex != -1; }

static void safe_position_check_map(void) {
  if (sSafe.valid && (safe_position_warp_active() || sSafe.mapId != *game.gGameLoopPendingMapId ||
                      sSafe.layer != game.getCurMapLayer())) {
    sSafe.valid = 0;
  }
}

static int safe_position_allowed(GameObject* player, PlayerState* st) {
  if (safe_position_warp_active()) {
    return 0;
  }
  if (player->parent != NULL || st->focusObject != NULL) {
    return 0;
  }
  if ((st->flags360 & PLAYER_FLAG_TELEPORT_HOLD) != 0) {
    return 0;
  }
  if (game.isInBounds(player->localPosX, player->localPosZ) != 1) {
    return 0;
  }
  return !(st->resultFloorY <= NO_FLOOR_Y);
}

static void safe_position_update(GameObject* player, PlayerState* st) {
  safe_position_check_map();
  if (!safe_position_allowed(player, st)) {
    return;
  }
  sSafe.valid = 1;
  sSafe.pos.x = player->localPosX;
  sSafe.pos.y = player->localPosY;
  sSafe.pos.z = player->localPosZ;
  sSafe.rot.x = player->rotX;
  sSafe.rot.y = player->rotY;
  sSafe.rot.z = player->rotZ;
  sSafe.swimming = (st->flags3F0 & FLAGS3F0_SWIMMING) != 0;
  sSafe.mapId = *game.gGameLoopPendingMapId;
  sSafe.layer = game.getCurMapLayer();
}

static void return_to_safe_position(void) {
  GameObject* player;
  PlayerState* st;

  safe_position_check_map();
  if (sSafe.valid && game.isInBounds(sSafe.pos.x, sSafe.pos.z) != 1) {
    sSafe.valid = 0;
  }
  if (!sSafe.valid || !flyModeGameplayActive() || arwing_active() || (player = game.Obj_GetPlayerObject()) == NULL) {
    return;
  }
  st = player->extra;
  if (st == NULL || !player_controllable_on_foot(player, st)) {
    return;
  }
  if (player->parent != NULL) {
    game.Obj_SetParent(player, NULL, 1);
  }
  player->velocityX = 0.0f;
  player->velocityY = 0.0f;
  player->velocityZ = 0.0f;
  st->animSpeedA = 0.0f;
  st->animSpeedB = 0.0f;
  st->animSpeedC = 0.0f;
  st->flags3F0 &= (uint8_t)~FLAGS3F0_B04;
  st->flags3F0 &= (uint8_t)~FLAGS3F0_B08;
  st->staffHoldFrames = 0;
  if (!sSafe.swimming) {
    st->flags3F0 &= (uint8_t)~FLAGS3F0_SWIMMING;
  }
  game.playerTeleport(player, &sSafe.pos, &sSafe.rot, 0);
  game.objSetPos(player, sSafe.pos.x, sSafe.pos.y, sSafe.pos.z);
  game.playerTeleport(player, NULL, NULL, 0);
  modLog(FH_LOG_INFO, "Returned to safe position");
}

/* A held key only drives Fly Mode after it has been seen released while
   Fly Mode was active, so a key already down when Fly Mode turns on (or
   when the game regains focus) does nothing until it is pressed again. */
static int fly_key_held(int* armed, int keyDown, int active) {
  if (!active) {
    *armed = 0;
    return 0;
  }
  if (!keyDown) {
    *armed = 1;
    return 0;
  }
  return *armed;
}

/* Key state is passed in every frame, focused or not, so a key already held
   when the game regains focus is not seen as a new press. */
void flyModePoll(int toggleKeyDown, int upKeyDown, int downKeyDown, int returnKeyDown, int active) {
  int up;
  int down;

  safe_position_check_map();
  toggleKeyDown = toggleKeyDown != 0;
  returnKeyDown = returnKeyDown != 0;
  if (active && toggleKeyDown && !sFlyToggleWasDown && !arwing_active()) {
    set_fly(!sFly);
    modLog(FH_LOG_INFO, sFly ? "Enabled" : "Disabled");
  }
  sFlyToggleWasDown = toggleKeyDown;
  if (active && returnKeyDown && !sFlyReturnWasDown) {
    return_to_safe_position();
  }
  sFlyReturnWasDown = returnKeyDown;
  active = sFly && active;
  up = fly_key_held(&sFlyUpArmed, upKeyDown != 0, active);
  down = fly_key_held(&sFlyDownArmed, downKeyDown != 0, active);
  sFlyInput = up - down;
}

static int fly_mode_allowed(int16_t mode) {
  int i;

  if (mode == PLAYER_MODE_IDLE || mode == PLAYER_MODE_MOVING) {
    return 1;
  }
  for (i = 0; i < (int)(sizeof(sFlyCombatModes) / sizeof(sFlyCombatModes[0])); i++) {
    if (sFlyCombatModes[i] == mode) {
      return 1;
    }
  }
  return 0;
}

static int fly_allowed(GameObject* player, PlayerState* st) {
  if (!player_controllable_on_foot(player, st) || !fly_mode_allowed(st->controlMode)) {
    return 0;
  }
  return !((st->flags3F0 & FLAGS3F0_B04) && (st->flags3F1 & FLAGS3F1_B01));
}

static int fly_input(PlayerState* st) {
  if (st->curAnimId == CAMERA_MODE_VIEWFINDER_RESOURCE_ID || st->curAnimId == CAMERA_MODE_WORLD_MAP_RESOURCE_ID) {
    return 0;
  }
  return sFlyInput;
}

static int fly_at_swim_surface(PlayerState* st, int input) {
  return (st->flags3F0 & FLAGS3F0_SWIMMING) && input == 0 && st->waterDepth <= FLY_SWIM_SURFACE_DEPTH;
}

static int fly_crossed_water_surface(PlayerState* st, int input) {
  int16_t mode = st->controlMode;

  return (st->flags3F0 & FLAGS3F0_SWIMMING) && input > 0 && st->waterDepth < 0.0f &&
         (mode == PLAYER_MODE_IDLE || mode == PLAYER_MODE_MOVING);
}

/* The cheat menu scaled this step by its Fast Movement factor, which is 1
   whenever Fast Movement is off; this mod has no Fast Movement, so the
   prediction uses the normal step that playerUpdate passes to objMove. */
static void fly_keep_inside_map(GameObject* player, PlayerState* st) {
  float step;

  if (player->parent != NULL || st->focusObject != NULL) {
    return;
  }
  step = *game.timeDelta;
  if (game.isInBounds(player->localPosX + player->velocityX * step, player->localPosZ + player->velocityZ * step) ==
      0) {
    player->velocityX = 0.0f;
    player->velocityZ = 0.0f;
  }
}

/* Runs inside playerUpdate right after playerUpdateVelocityFromMotion, before
   the velocity clamp and the objMove that applies it. */
void flyModeUpdate(GameObject* player) {
  PlayerState* st;
  int input;

  if (!sFly || player == NULL || player != game.Obj_GetPlayerObject()) {
    return;
  }
  st = player->extra;
  if (st == NULL || !fly_allowed(player, st)) {
    return;
  }
  st->flags3F0 &= (uint8_t)~FLAGS3F0_B08;
  st->flags3F0 &= (uint8_t)~FLAGS3F0_B04;
  st->staffHoldFrames = 0;
  input = fly_input(st);
  if (!fly_at_swim_surface(st, input)) {
    player->velocityY = (float)input * FLY_VERTICAL_SPEED;
  }
  if (fly_crossed_water_surface(st, input)) {
    st->flags3F0 &= (uint8_t)~FLAGS3F0_SWIMMING;
  }
  fly_keep_inside_map(player, st);
  safe_position_update(player, st);
}

void flyModeReset(void) {
  set_fly(0);
  sSessionActive = 0;
  sFlyToggleWasDown = 0;
  sFlyReturnWasDown = 0;
  memset(&sSafe, 0, sizeof(sSafe));
}
