option(BUILD_RectifyImageMiniApps "" OFF)

if(BUILD_RectifyImageMiniApps)
  mitkFunctionCreateCommandLineApp(
    NAME RectifyImageMiniApp
    DEPENDS MitkCore MitkCommandLine
  )
endif()
