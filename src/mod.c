#include "fly_mode.h"
#include "platform_input.h"

#include <stdarg.h>
#include <stdio.h>

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

FH_MOD_EXPORT int fh_mod_initialize(FhMod* mod, const FhModHost* host) {
  if (!host || host->abiVersion != FH_MOD_ABI_VERSION || host->structSize < sizeof(FhModHost)) return FH_MOD_ERROR;
  if (!host->log || !host->symbolAddress || !host->hookInstall || !host->hookRemove) return FH_MOD_ERROR;
  H = host;
  M = mod;
  flyModeReset();
  if (!platformInputInitialize(mod, host)) {
    modLog(FH_LOG_ERROR, "disabled: keyboard input is unavailable");
    return FH_MOD_ERROR;
  }
  if (!flyHooksInstall(mod, host)) {
    flyHooksRemove(mod, host);
    platformInputShutdown();
    modLog(FH_LOG_ERROR, "disabled: required host symbols or hooks are unavailable");
    return FH_MOD_ERROR;
  }
  modLog(FH_LOG_INFO, "v1.1.0 loaded (Home Toggle, End Safe, PgUp/PgDn Fly)");
  return FH_MOD_OK;
}

/* Home toggles and End returns once per press; Page Up and Page Down act
   while held. Keys only act while the game window is focused. */
FH_MOD_EXPORT void fh_mod_update(FhMod* mod) {
  int active;
  (void)mod;

  flyModeUpdateSession();
  active = flyModeGameplayActive() && platformInputActive();
  flyModePoll(platformKeyDown(PLATFORM_KEY_HOME), platformKeyDown(PLATFORM_KEY_PAGE_UP),
              platformKeyDown(PLATFORM_KEY_PAGE_DOWN), platformKeyDown(PLATFORM_KEY_END), active);
}

FH_MOD_EXPORT void fh_mod_shutdown(FhMod* mod) {
  (void)mod;
  if (H && M) flyHooksRemove(M, H);
  flyModeReset();
  platformInputShutdown();
  H = 0;
  M = 0;
}
