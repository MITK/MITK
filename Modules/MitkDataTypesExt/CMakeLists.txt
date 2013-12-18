MITK_CREATE_MODULE(MitkDataTypesExt
                   DEPENDS Mitk
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

