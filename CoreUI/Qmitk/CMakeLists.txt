MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk
               PACKAGE_DEPENDS QT QVTK
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
               QT_MODULE
)

