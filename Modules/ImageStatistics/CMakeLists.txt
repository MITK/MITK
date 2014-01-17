MITK_CREATE_MODULE( ImageStatistics
  DEPENDS ImageExtraction PlanarFigure
  PACKAGE_DEPENDS ITK|ITKVTK
  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
