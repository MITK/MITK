MITK_CREATE_MODULE(
  DEPENDS MitkAlgorithmsExt MitkQtWidgets
  PACKAGE_DEPENDS
    PUBLIC Qwt CTK|CTKWidgets
    PRIVATE Qt5|Concurrent+Svg+Xml
)
