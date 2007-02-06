SET(CPP_FILES
  mitkBaseRenderer.cpp
  mitkBaseVtkMapper2D.cpp
  mitkBaseVtkMapper3D.cpp
  mitkContourMapper2D.cpp
  mitkContourSetMapper2D.cpp
  mitkContourSetVtkMapper3D.cpp
  mitkContourVtkMapper3D.cpp
  mitkGeometry2DDataMapper2D.cpp
  mitkGeometry2DDataVtkMapper3D.cpp
  mitkGLMapper2D.cpp
  mitkGradientBackground.cpp
  mitkImageBackground2D.cpp
  mitkImageMapper2D.cpp
  mitkLineMapper2D.cpp
  # mitkLineVtkMapper3D.cpp
  mitkMapper2D.cpp
  mitkMapper3D.cpp
  mitkMapper.cpp
  mitkMapperFactory.cpp
  mitkMeshMapper2D.cpp
  mitkMeshVtkMapper3D.cpp
  mitkNativeRenderWindowInteractor.cpp
  mitkOpenGLRenderer.cpp
  mitkPointDataVtkMapper3D.cpp
  mitkPointSetMapper2D.cpp
  mitkPointSetVtkMapper3D.cpp
  mitkPolyDataGLMapper2D.cpp
  mitkRenderWindow.cpp
  mitkSplineVtkMapper3D.cpp
  mitkSurfaceMapper2D.cpp
  mitkSurfaceVtkMapper3D.cpp
  # mitkVectorImageMapper2D.cpp
  # mitkVectorImageVtkGlyphMapper3D.cpp
  mitkVolumeDataVtkMapper3D.cpp
  mitkVtkARRenderWindow.cpp
  mitkVtkRenderWindow.cpp
  mitkVtkStencilRenderWindow.cpp
)
IF(WIN32)
  SET(CPP_FILES ${CPP_FILES}
    mitkVtkWin32ARRenderWindow.cpp
    mitkVtkWin32StencilRenderWindow.cpp
    mitkWin32ARMapper2D.cpp
  )
ENDIF(WIN32)

