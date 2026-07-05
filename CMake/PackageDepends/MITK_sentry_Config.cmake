set(ALL_LIBRARIES sentry::sentry)

# Present in the static build; required for non-fatal snapshots
# (crashpad::CrashpadClient::DumpWithoutCrash has no sentry C API).
if(TARGET sentry_crashpad::client)
  list(APPEND ALL_LIBRARIES sentry_crashpad::client)
endif()
