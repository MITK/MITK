MITK_CREATE_MODULE(
  INCLUDE_DIRS Qmitk
  DEPENDS MitkOpenIGTLink MitkQtWidgetsExt MitkPersistence
  #GENERATED_CPP ${TOOL_GUI_CPPS} ${TOOL_CPPS}
)

## create IGTUI config
#configure_file(mitkIGTUIConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTUIConfig.h @ONLY)
