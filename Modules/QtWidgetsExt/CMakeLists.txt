MITK_CREATE_MODULE(
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers
  DEPENDS MitkImageStatistics MitkQtWidgets
  PACKAGE_DEPENDS
    PUBLIC Qwt CTK|CTKWidgets Qt4|QtWebKit Qt5|WebKitWidgets
    PRIVATE Qt5|Concurrent+Svg+Xml
  WARNINGS_AS_ERRORS
)
