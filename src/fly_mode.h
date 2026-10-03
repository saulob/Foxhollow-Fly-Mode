#ifndef FLY_MODE_H_
#define FLY_MODE_H_

#include <stddef.h>
#include <stdint.h>

#include "foxhollow_mod_api.h"

/* Partial x64 layouts of the game records Fly Mode touches. The game headers'
   STATIC_ASSERT offsets describe the 32-bit GameCube layout, so these offsets
   come from the headers compiled for x64 and match what playerUpdate,
   playerUpdateVelocityFromMotion, playerTeleport, objSetPos,
   playerStartWallTransition, the player getters, Obj_IsParentSlackClear and
   curves_resolveWaterFloorCeiling read and write in the Foxhollow build. */
typedef struct Vec3f {
  float x;
  float y;
  float z;
} Vec3f;

typedef struct Vec3s {
  int16_t x;
  int16_t y;
  int16_t z;
} Vec3s;

typedef struct GameObject GameObject;

struct GameObject {
  int16_t rotX;
  int16_t rotY;
  int16_t rotZ;
  uint8_t pad006[0x06];
  float localPosX;
  float localPosY;
  float localPosZ;
  uint8_t pad018[0x0C];
  float velocityX;
  float velocityY;
  float velocityZ;
  GameObject* parent;
  uint8_t pad038[0xC0];
  uint16_t objectFlags;
  uint8_t pad0FA[0x06];
  void* extra;
};

typedef struct PlayerStatus {
  int8_t health;
} PlayerStatus;

typedef struct PlayerState {
  uint8_t pad000[0x1D0];
  float resultFloorY;
  uint8_t pad1D4[0xDC];
  int16_t controlMode;
  uint8_t pad2B2[0x0E];
  float animSpeedA;
  float animSpeedB;
  uint8_t pad2C8[0x0C];
  float animSpeedC;
  uint8_t pad2D8[0xD8];
  PlayerStatus* playerStatus;
  uint32_t flags360;
  uint8_t pad3BC[0x8C];
  uint8_t flags3F0;
  uint8_t flags3F1;
  uint8_t pad44A[0x27];
  uint8_t staffHoldFrames;
  uint8_t pad472[0x3BE];
  float verticalVel;
  uint8_t pad834[0x6C];
  GameObject* focusObject;
  uint8_t pad8A8[0x08];
  GameObject* heldObj;
  uint8_t pad8B8[0x1E];
  int16_t characterId;
  uint8_t pad8D8[0x1C];
  float waterDepth;
  uint8_t pad8F8[0x94];
  uint8_t curAnimId;
} PlayerState;

_Static_assert(sizeof(Vec3f) == 0x0C, "Vec3f");
_Static_assert(sizeof(Vec3s) == 0x06, "Vec3s");
_Static_assert(offsetof(GameObject, rotY) == 0x02, "GameObject.anim.rotY");
_Static_assert(offsetof(GameObject, rotZ) == 0x04, "GameObject.anim.rotZ");
_Static_assert(offsetof(GameObject, localPosX) == 0x0C, "GameObject.anim.localPosX");
_Static_assert(offsetof(GameObject, localPosY) == 0x10, "GameObject.anim.localPosY");
_Static_assert(offsetof(GameObject, localPosZ) == 0x14, "GameObject.anim.localPosZ");
_Static_assert(offsetof(GameObject, velocityX) == 0x24, "GameObject.anim.velocityX");
_Static_assert(offsetof(GameObject, velocityY) == 0x28, "GameObject.anim.velocityY");
_Static_assert(offsetof(GameObject, velocityZ) == 0x2C, "GameObject.anim.velocityZ");
_Static_assert(offsetof(GameObject, parent) == 0x30, "GameObject.anim.parent");
_Static_assert(offsetof(GameObject, objectFlags) == 0xF8, "GameObject.objectFlags");
_Static_assert(offsetof(GameObject, extra) == 0x100, "GameObject.extra");
_Static_assert(offsetof(PlayerState, resultFloorY) == 0x1D0, "PlayerState.baddie.curvesCollision.resultFloorY");
_Static_assert(offsetof(PlayerState, controlMode) == 0x2B0, "PlayerState.baddie.controlMode");
_Static_assert(offsetof(PlayerState, animSpeedA) == 0x2C0, "PlayerState.baddie.animSpeedA");
_Static_assert(offsetof(PlayerState, animSpeedB) == 0x2C4, "PlayerState.baddie.animSpeedB");
_Static_assert(offsetof(PlayerState, animSpeedC) == 0x2D4, "PlayerState.baddie.animSpeedC");
_Static_assert(offsetof(PlayerState, playerStatus) == 0x3B0, "PlayerState.playerStatus");
_Static_assert(offsetof(PlayerState, flags360) == 0x3B8, "PlayerState.flags360");
_Static_assert(offsetof(PlayerState, flags3F0) == 0x448, "PlayerState.flags3F0");
_Static_assert(offsetof(PlayerState, flags3F1) == 0x449, "PlayerState.flags3F1");
_Static_assert(offsetof(PlayerState, staffHoldFrames) == 0x471, "PlayerState.staffHoldFrames");
_Static_assert(offsetof(PlayerState, verticalVel) == 0x830, "PlayerState.verticalVel");
_Static_assert(offsetof(PlayerState, focusObject) == 0x8A0, "PlayerState.focusObject");
_Static_assert(offsetof(PlayerState, heldObj) == 0x8B0, "PlayerState.heldObj");
_Static_assert(offsetof(PlayerState, characterId) == 0x8D6, "PlayerState.characterId");
_Static_assert(offsetof(PlayerState, waterDepth) == 0x8F4, "PlayerState.waterDepth");
_Static_assert(offsetof(PlayerState, curAnimId) == 0x98C, "PlayerState.curAnimId");

/* ByteFlags masks of PlayerState.flags3F0 and flags3F1 (b04 is bit 0x04). */
#define FLAGS3F0_B04 0x04
#define FLAGS3F0_B08 0x08
#define FLAGS3F0_SWIMMING 0x20
#define FLAGS3F1_B01 0x01

typedef struct FlyModeGame {
  int (*getGameState)(void);
  int (*getCurUiDll)(void);
  int (*getSaveGameLoadStatus)(void);
  GameObject* (*Obj_GetPlayerObject)(void);
  GameObject* (*getArwing)(void);
  int (*getCurSeqNo)(void);
  int (*isInBounds)(float x, float z);
  int32_t (*getCurMapLayer)(void);
  void (*playerTeleport)(GameObject* player, const Vec3f* position, const Vec3s* rotation, int unused);
  void (*objSetPos)(GameObject* player, float x, float y, float z);
  void (*Obj_SetParent)(GameObject* obj, GameObject* newParent, int updateLocalTransform);
  int16_t* gArrivedWarpIndex;
  int16_t* gPendingWarpIndex;
  int* gGameLoopPendingMapId;
  float* timeDelta;
} FlyModeGame;

extern FlyModeGame game;

void modLog(FhLogLevel level, const char* format, ...);

int flyHooksInstall(FhMod* mod, const FhModHost* host);
void flyHooksRemove(FhMod* mod, const FhModHost* host);

int flyModeGameplayActive(void);
void flyModeUpdateSession(void);
void flyModePoll(int toggleKeyDown, int upKeyDown, int downKeyDown, int returnKeyDown, int active);
void flyModeUpdate(GameObject* player);
void flyModeReset(void);

#endif
