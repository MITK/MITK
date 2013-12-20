MITK_CREATE_MODULE(MitkAlgorithmsExt
                   DEPENDS MitkDataTypesExt
                   PACKAGE_DEPENDS ANN
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

