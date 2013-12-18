MITK_CREATE_MODULE(MitkAlgorithmsExt
                   DEPENDS MitkDataTypesExt
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

