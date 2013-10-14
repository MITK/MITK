MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk
               PACKAGE_DEPENDS Qt4 QVTK
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
               QT_MODULE
)

add_subdirectory(Testing)
