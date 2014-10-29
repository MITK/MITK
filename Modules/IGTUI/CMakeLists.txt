MITK_CREATE_MODULE(
  SUBPROJECTS MITK-IGT
  INCLUDE_DIRS Qmitk
  DEPENDS MitkIGT MitkQtWidgetsExt MitkPersistence
)

## create IGTUI config
configure_file(mitkIGTUIConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTUIConfig.h @ONLY)


