SET(MITK_CPACK_PACKAGE_EXECUTABLES "" CACHE INTERNAL "Collecting windows shortcuts to executables" FORCE)
IF(MITK_USE_QT AND QT4_FOUND)
  
  IF(MITK_USE_BLUEBERRY)
    IF(MITK_BUILD_org.mitk.gui.qt.application)
      ADD_SUBDIRECTORY(CoreApp)
    ENDIF()
    IF(MITK_BUILD_org.mitk.gui.qt.extapplication)
      ADD_SUBDIRECTORY(ExtApp)
    ENDIF()
  ENDIF()

  IF(MITK_BUILD_TUTORIAL)
    ADD_SUBDIRECTORY(Tutorial)
  ENDIF()

ENDIF()
