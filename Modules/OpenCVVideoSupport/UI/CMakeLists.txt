MITK_CREATE_MODULE(mitkOpenCVVideoSupportUI
  DEPENDS MitkExt QmitkExt mitkOpenCVVideoSupport
  PACKAGE_DEPENDS QT
  QT_MODULE
  EXPORT_DEFINE MITK_OPENCVVIDEOSUPPORTUI_EXPORT
)

# no tests yet
#if(BUILD_TESTING)
	#add_subdirectory(Testing)
#endif(BUILD_TESTING)