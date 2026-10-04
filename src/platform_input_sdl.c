#define _GNU_SOURCE

#include "platform_input.h"
#include "fly_mode.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>

/* SDL_Scancode values from Foxhollow's SDL3 (SDL_scancode.h). */
#define SDL3_SCANCODE_HOME 74
#define SDL3_SCANCODE_PAGEUP 75
#define SDL3_SCANCODE_END 77
#define SDL3_SCANCODE_PAGEDOWN 78

typedef const bool* (*SdlGetKeyboardStateFn)(int* numkeys);
typedef void* (*SdlGetKeyboardFocusFn)(void);

static SdlGetKeyboardStateFn sGetKeyboardState;
static SdlGetKeyboardFocusFn sGetKeyboardFocus;

static void* resolve_sdl_symbol(FhMod* mod, const FhModHost* host, const char* name) {
  void* address = dlsym(RTLD_DEFAULT, name);

  if (address == NULL) {
    address = host->symbolAddress(mod, name);
  }
  if (address == NULL) {
    modLog(FH_LOG_ERROR, "could not resolve %s", name);
  }
  return address;
}

int platformInputInitialize(FhMod* mod, const FhModHost* host) {
  const bool* keys;
  int count = 0;

  sGetKeyboardState = (SdlGetKeyboardStateFn)resolve_sdl_symbol(mod, host, "SDL_GetKeyboardState");
  sGetKeyboardFocus = (SdlGetKeyboardFocusFn)resolve_sdl_symbol(mod, host, "SDL_GetKeyboardFocus");
  if (sGetKeyboardState == NULL || sGetKeyboardFocus == NULL) {
    platformInputShutdown();
    return 0;
  }
  /* Page Down is the highest scancode Fly Mode reads. */
  keys = sGetKeyboardState(&count);
  if (keys == NULL || count <= SDL3_SCANCODE_PAGEDOWN) {
    modLog(FH_LOG_ERROR, "SDL keyboard state is unavailable");
    platformInputShutdown();
    return 0;
  }
  return 1;
}

void platformInputShutdown(void) {
  sGetKeyboardState = NULL;
  sGetKeyboardFocus = NULL;
}

int platformInputActive(void) {
  return sGetKeyboardFocus != NULL && sGetKeyboardFocus() != NULL;
}

/* Keys are read by physical position. SDL clears this state when the window
   loses focus and restores only modifier keys when it regains it, so a key
   held across a focus change reads as up until it is pressed again. */
int platformKeyDown(PlatformKey key) {
  const bool* keys;
  int count = 0;
  int scancode;

  switch (key) {
    case PLATFORM_KEY_HOME:
      scancode = SDL3_SCANCODE_HOME;
      break;
    case PLATFORM_KEY_END:
      scancode = SDL3_SCANCODE_END;
      break;
    case PLATFORM_KEY_PAGE_UP:
      scancode = SDL3_SCANCODE_PAGEUP;
      break;
    case PLATFORM_KEY_PAGE_DOWN:
      scancode = SDL3_SCANCODE_PAGEDOWN;
      break;
    default:
      return 0;
  }
  if (sGetKeyboardState == NULL) return 0;
  keys = sGetKeyboardState(&count);
  return keys != NULL && scancode < count && keys[scancode];
}
