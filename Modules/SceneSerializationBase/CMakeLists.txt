MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC tinyxml Boost
  WARNINGS_AS_ERRORS
)
add_subdirectory(test)

