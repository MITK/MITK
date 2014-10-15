MITK_CREATE_MODULE(DEPENDS MitkCore
                   WARNINGS_AS_ERRORS
)

# build tests?
if(BUILD_TESTING)
  add_subdirectory(Testing)
ENDIF()
