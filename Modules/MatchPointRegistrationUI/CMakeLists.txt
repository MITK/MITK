MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Qmitk
  DEPENDS MitkQtWidgets MitkQtWidgetsExt MitkMatchPointRegistration
  PACKAGE_DEPENDS Qt6|Core CTK|CTKWidgets
)
