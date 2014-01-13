MITK_CREATE_MODULE( ImageStatistics
  DEPENDS Mitk ImageExtraction PlanarFigure
  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
