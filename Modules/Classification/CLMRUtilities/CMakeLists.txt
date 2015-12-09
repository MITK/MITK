MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkCLCore
  PACKAGE_DEPENDS PUBLIC Eigen Boost
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
