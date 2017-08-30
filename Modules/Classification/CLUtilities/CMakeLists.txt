MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkCLCore
  PACKAGE_DEPENDS PUBLIC Eigen
)

add_subdirectory(test)
