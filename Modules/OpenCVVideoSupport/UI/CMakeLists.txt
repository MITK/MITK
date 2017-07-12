MITK_CREATE_MODULE(OpenCVVideoSupportUI
  DEPENDS MitkQtWidgetsExt MitkOpenCVVideoSupport
  WARNINGS_AS_ERRORS
)

# no tests yet
#if(BUILD_TESTING)
  #add_subdirectory(Testing)
#endif(BUILD_TESTING)
