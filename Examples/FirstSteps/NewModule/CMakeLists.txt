MITK_CREATE_MODULE(
  INCLUDE_DIRS
    PUBLIC ${MITK_BINARY_DIR}
    PRIVATE src/DataManagement
  DEPENDS PUBLIC MitkCore
  WARNINGS_AS_ERRORS
  SUBPROJECTS MITK-Examples
)

add_subdirectory(autoload/IO)
add_subdirectory(MiniApps)
add_subdirectory(test)
