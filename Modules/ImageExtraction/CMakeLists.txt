MITK_CREATE_MODULE( ImageExtraction
               DEPENDS MitkAlgorithmsExt
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
