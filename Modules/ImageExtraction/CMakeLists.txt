MITK_CREATE_MODULE( ImageExtraction
               DEPENDS Mitk mitkIpPic PlanarFigure
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
