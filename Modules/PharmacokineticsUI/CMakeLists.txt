MITK_CREATE_MODULE(PharmacokineticsUI
  INCLUDE_DIRS Common Qmitk
  DEPENDS MitkPharmacokinetics MitkQtWidgets MitkQtWidgetsExt
  PACKAGE_DEPENDS Qt5|Core CTK|CTKWidgets Boost
)
