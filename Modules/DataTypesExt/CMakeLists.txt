set(_additional_libs)
if(USE_ITKZLIB)
  list(APPEND _additional_libs itkzlib)
else()
  list(APPEND _additional_libs z)
endif(USE_ITKZLIB)

MITK_CREATE_MODULE(DEPENDS MitkCore
                   ADDITIONAL_LIBS ${_additional_libs}
                   WARNINGS_AS_ERRORS
                  )

add_subdirectory(test)

