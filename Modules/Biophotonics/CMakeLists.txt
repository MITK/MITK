MITK_CREATE_MODULE(mitkBiophotonics
  WARNINGS_AS_ERRORS
)
if( MITK_USE_Python )
  ADD_SUBDIRECTORY(python)
endif()
