MITK_CREATE_MODULE(
  SUBPROJECTS MITK-IGT
  INCLUDE_DIRS Qmitk
  DEPENDS MitkIGT MitkQtWidgetsExt MitkPersistence
  GENERATED_CPP ${TOOL_GUI_CPPS} ${TOOL_CPPS}
)

## create IGTUI config
configure_file(mitkIGTUIConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTUIConfig.h @ONLY)


