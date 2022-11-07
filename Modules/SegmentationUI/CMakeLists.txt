MITK_CREATE_MODULE (
  INCLUDE_DIRS Qmitk SegmentationUtilities
  DEPENDS MitkSegmentation MitkQtWidgetsExt
  PACKAGE_DEPENDS PRIVATE CTK|CTKWidgets nlohmann_json
)
