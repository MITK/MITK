MITK_CREATE_MODULE(PharmacokineticsUI
  AUTOMOC
  INCLUDE_DIRS PRIVATE src/Qmitk
  DEPENDS MitkPharmacokinetics MitkQtWidgets MitkQtWidgetsExt
  PACKAGE_DEPENDS CTK|CTKWidgets
)
