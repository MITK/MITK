MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkCLCore MitkCommandLine
  PACKAGE_DEPENDS PUBLIC OpenMP|OpenMP_CXX Eigen
)

add_subdirectory(test)
