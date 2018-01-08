MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkCLCore MitkCommandLine
  PACKAGE_DEPENDS PUBLIC Eigen
)

add_subdirectory(test)
