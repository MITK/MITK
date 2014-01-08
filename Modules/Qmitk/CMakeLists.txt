MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk PlanarFigure
               PACKAGE_DEPENDS QVTK
               QT4_MODULES QtCore QtGui
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
)

add_subdirectory(Testing)
