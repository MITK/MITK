MITK_CREATE_MODULE( ImageExtraction
               DEPENDS Mitk PlanarFigure
               DEPENDS_INTERNAL pic2vtk
)

IF(BUILD_TESTING)

  ADD_SUBDIRECTORY(Testing)

ENDIF(BUILD_TESTING)
