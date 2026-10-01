set(CPP_FILES
  mitkBaseApplication.cpp
  mitkProvisioningInfo.cpp
  QmitkSafeApplication.cpp
  QmitkSingleApplication.cpp
)

if(MITK_USE_sentry)
  list(APPEND CPP_FILES
    mitkCrashDumpSessionOptions.cpp
    mitkICrashReportService.cpp
    QmitkCrashDumpDialog.cpp
    QmitkCrashDumpListWidget.cpp
    QmitkCrashDumpManagerDialog.cpp
    QmitkCrashDumpUiUtils.cpp
    QmitkUiFreezeWatchdog.cpp
  )
endif()
