project(org_mitk_gui_qt_extapplication)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE MITK_QT_EXTAPP
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS Qt4|QtWebKit Qt5|WebKit
)

if (DESIRED_QT_VERSION MATCHES "5")
  if (Qt5WebKit_DIR)
    add_definitions(-DQT_WEBKIT)
  endif()
else()
  if(QT_QTWEBKIT_FOUND)
    add_definitions(-DQT_WEBKIT)
  endif(QT_QTWEBKIT_FOUND)
endif()
