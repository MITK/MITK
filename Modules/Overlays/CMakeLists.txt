MITK_CREATE_MODULE( Overlays
                    DEPENDS Qmitk
                    PACKAGE_DEPENDS QT QVTK
                    QT_MODULE
)

  if(BUILD_TESTING)
    add_subdirectory(Testing)
  endif(BUILD_TESTING)
