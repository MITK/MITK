MITK_CREATE_MODULE(mitkOpenCVVideoSupportUI
  DEPENDS QmitkExt mitkOpenCVVideoSupport
  QT4_MODULES QtGui
  EXPORT_DEFINE MITK_OPENCVVIDEOSUPPORTUI_EXPORT
)

# no tests yet
#if(BUILD_TESTING)
  #add_subdirectory(Testing)
#endif(BUILD_TESTING)
