MITK_CREATE_MODULE( ImageStatistics
               DEPENDS Mitk ImageExtraction PlanarFigure
               PROVIDES ImageStatistics
)

IF(BUILD_TESTING)

  ADD_SUBDIRECTORY(Testing)

ENDIF(BUILD_TESTING)
