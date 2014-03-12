SET(QT_USE_QTTEST 1) # for the test driver

MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk
               PACKAGE_DEPENDS QT QVTK
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
               QT_MODULE
)

add_subdirectory(Testing)
