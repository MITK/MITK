MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk
               PACKAGE_DEPENDS QT
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
               QT_MODULE
)

add_subdirectory(Testing)
