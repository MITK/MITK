MITK_CREATE_MODULE(
  INCLUDE_DIRS Qmitk
  DEPENDS MitkCore MitkChart MitkImageStatistics MitkQtWidgets
  PACKAGE_DEPENDS PRIVATE Qt5|WebEngineWidgets
)