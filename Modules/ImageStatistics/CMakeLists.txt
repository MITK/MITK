MITK_CREATE_MODULE( ImageStatistics
               DEPENDS Mitk ImageExtraction PlanarFigure
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
