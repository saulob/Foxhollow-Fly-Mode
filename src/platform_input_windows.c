#include "platform_input.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int platformInputInitialize(FhMod* mod, const FhModHost* host) {
  (void)mod;
  (void)host;
  return 1;
}

void platformInputShutdown(void) {
}

int platformInputActive(void) {
  HWND window = GetForegroundWindow();
  DWORD processId = 0;

  if (window == NULL) return 0;
  GetWindowThreadProcessId(window, &processId);
  return processId == GetCurrentProcessId();
}

/* GetAsyncKeyState reports the physical key, focused or not. */
int platformKeyDown(PlatformKey key) {
  int virtualKey;

  switch (key) {
    case PLATFORM_KEY_HOME:
      virtualKey = VK_HOME;
      break;
    case PLATFORM_KEY_END:
      virtualKey = VK_END;
      break;
    case PLATFORM_KEY_PAGE_UP:
      virtualKey = VK_PRIOR;
      break;
    case PLATFORM_KEY_PAGE_DOWN:
      virtualKey = VK_NEXT;
      break;
    default:
      return 0;
  }
  return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}
