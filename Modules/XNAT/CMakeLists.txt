MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkQtWidgets
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKXNATCore
    PRIVATE Qt4|QtGui Qt5|UiTools+XmlPatterns+Widgets
)
