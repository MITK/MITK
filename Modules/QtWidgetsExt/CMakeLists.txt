MITK_CREATE_MODULE(
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers
  DEPENDS MitkImageStatistics MitkQtWidgets
  PACKAGE_DEPENDS CTK|CTKWidgets Qt4|QtWebKit Qt5|Concurrent+Svg+WebKitWidgets+Xml Qwt
  WARNINGS_AS_ERRORS
)
