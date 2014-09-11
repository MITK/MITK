MITK_CREATE_MODULE(OpenCVVideoSupportUI
  DEPENDS MitkQtWidgetsExt MitkOpenCVVideoSupport
  EXPORT_DEFINE MITK_OPENCVVIDEOSUPPORTUI_EXPORT
)

# no tests yet
#if(BUILD_TESTING)
  #add_subdirectory(Testing)
#endif(BUILD_TESTING)
