MITK_CREATE_MODULE(
  DEPENDS MitkPlanarFigure MitkOverlays
  PACKAGE_DEPENDS PUBLIC VTK|vtkGUISupportQt+vtkGUISupportQtOpenGL Qt4|QtGui+QtOpenGL Qt5|Widgets+OpenGL
  SUBPROJECTS MITK-CoreUI
)

add_subdirectory(Testing)
