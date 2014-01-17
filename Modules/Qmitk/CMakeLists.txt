MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk PlanarFigure
               PACKAGE_DEPENDS VTK|vtkGUISupportQt Qt4|QtGui
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
)

add_subdirectory(Testing)
