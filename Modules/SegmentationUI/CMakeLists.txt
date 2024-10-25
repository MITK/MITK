MITK_CREATE_MODULE (
  INCLUDE_DIRS Qmitk SegmentationUtilities
  DEPENDS MitkSegmentation MitkQtWidgetsExt MitkFormsUI
  PACKAGE_DEPENDS PRIVATE CTK|CTKWidgets Qt6|Concurrent nlohmann_json
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
