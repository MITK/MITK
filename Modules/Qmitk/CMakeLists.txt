MITK_CREATE_MODULE( Qmitk
               DEPENDS Mitk
               PACKAGE_DEPENDS QVTK
               QT4_MODULES QtCore QtGui
               SUBPROJECTS MITK-CoreUI
               EXPORT_DEFINE QMITK_EXPORT
)

add_subdirectory(Testing)
