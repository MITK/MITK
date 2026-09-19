option(BUILD_ClassificationCmdApps "Build classification command-line tools" OFF)

if(BUILD_ClassificationCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(NAME CLScreenshot DEPENDS MitkCLUtilities MitkQtWidgets)
  mitkFunctionCreateCommandLineApp(NAME CLGlobalImageFeatures DEPENDS MitkCLUtilities MitkQtWidgets)
  mitkFunctionCreateCommandLineApp(NAME CLMRNormalization DEPENDS MitkCLMRUtilities)
  mitkFunctionCreateCommandLineApp(NAME CLN4)
endif()
