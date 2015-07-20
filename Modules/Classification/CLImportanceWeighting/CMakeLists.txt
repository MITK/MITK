MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkClassificationCore MitkDataCollection
  #PACKAGE_DEPENDS PUBLIC Eigen
  WARNINGS_AS_ERRORS
)

#add_subdirectory(test)
