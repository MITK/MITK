MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk LegacyIO PlanarFigure
               PACKAGE_DEPENDS QT QVTK
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
               QT_MODULE
)

add_subdirectory(Testing)
