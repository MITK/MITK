set(CPP_FILES
  mitkBaseApplication.cpp
  mitkProvisioningInfo.cpp
  QmitkSafeApplication.cpp
  QmitkSingleApplication.cpp
)

if(MITK_USE_sentry)
  list(APPEND CPP_FILES
    QmitkCrashDumpDialog.cpp
    QmitkUiFreezeWatchdog.cpp
  )
endif()
