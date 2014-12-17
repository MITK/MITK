MITK_CREATE_MODULE(
  DEPENDS MitkPlanarFigure MitkOverlays
  PACKAGE_DEPENDS VTK|vtkGUISupportQt+vtkGUISupportQtOpenGL Qt4|QtGui+QtOpenGL Qt5|Widgets+OpenGL
  SUBPROJECTS MITK-CoreUI
  EXPORT_DEFINE QMITK_EXPORT
)

add_subdirectory(Testing)
