MITK_CREATE_MODULE(
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer
  DEPENDS MitkCore
  WARNINGS_AS_ERRORS
)
add_subdirectory(Testing)

