#include "fly_mode.h"

#include <stdarg.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static const FhModHost* H;
static FhMod* M;

void modLog(FhLogLevel level, const char* format, ...) {
  char message[256];
  int prefix;
  va_list args;

  if (!H || !H->log || !M) return;
  prefix = snprintf(message, sizeof(message), "[Fly Mode] ");
  va_start(args, format);
  vsnprintf(message + prefix, sizeof(message) - (size_t)prefix, format, args);
  va_end(args);
  H->log(M, level, message);
}

static int game_window_focused(void) {
  HWND window = GetForegroundWindow();
  DWORD processId = 0;

  if (window == NULL) return 0;
  GetWindowThreadProcessId(window, &processId);
  return processId == GetCurrentProcessId();
}

static int key_down(int key) {
  return (GetAsyncKeyState(key) & 0x8000) != 0;
}

FH_MOD_EXPORT int fh_mod_initialize(FhMod* mod, const FhModHost* host) {
  if (!host || host->abiVersion != FH_MOD_ABI_VERSION || host->structSize < sizeof(FhModHost)) return FH_MOD_ERROR;
  if (!host->log || !host->symbolAddress || !host->hookInstall || !host->hookRemove) return FH_MOD_ERROR;
  H = host;
  M = mod;
  flyModeReset();
  if (!flyHooksInstall(mod, host)) {
    flyHooksRemove(mod, host);
    modLog(FH_LOG_ERROR, "disabled: required host symbols or hooks are unavailable");
    return FH_MOD_ERROR;
  }
  modLog(FH_LOG_INFO, "v1.0.1 loaded (F5 Toggle, F6 Up, F7 Down, F8 Safe)");
  return FH_MOD_OK;
}

/* F5 toggles and F8 returns once per press; F6 and F7 act while held. Keys
   only act while the game window is focused. */
FH_MOD_EXPORT void fh_mod_update(FhMod* mod) {
  int active;
  (void)mod;

  flyModeUpdateSession();
  active = flyModeGameplayActive() && game_window_focused();
  flyModePoll(key_down(VK_F5), key_down(VK_F6), key_down(VK_F7), key_down(VK_F8), active);
}

FH_MOD_EXPORT void fh_mod_shutdown(FhMod* mod) {
  (void)mod;
  if (H && M) flyHooksRemove(M, H);
  flyModeReset();
  H = 0;
  M = 0;
}
