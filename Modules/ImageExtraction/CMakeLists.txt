MITK_CREATE_MODULE( ImageExtraction
               DEPENDS Mitk ipSegmentation PlanarFigure
               DEPENDS_INTERNAL pic2vtk IIL4MITK
               PROVIDES ImageExtraction
)

IF(BUILD_TESTING)

  ADD_SUBDIRECTORY(Testing)

ENDIF(BUILD_TESTING)
