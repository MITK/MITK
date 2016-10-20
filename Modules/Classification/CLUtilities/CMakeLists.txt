MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkCLCore MitkCommandLine
  PACKAGE_DEPENDS PUBLIC Eigen OpenCV
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
