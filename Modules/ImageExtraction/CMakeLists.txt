MITK_CREATE_MODULE( ImageExtraction
               DEPENDS Mitk mitkIpPic PlanarFigure
               DEPENDS_INTERNAL pic2vtk
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
