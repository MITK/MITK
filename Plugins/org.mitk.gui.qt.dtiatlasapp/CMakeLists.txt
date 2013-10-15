# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_dtiatlasapp)

set(PLUGIN_QT4_MODULES QtWebKit)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DTIATLAS_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES Qmitk SceneSerialization
)

if(QT_QTWEBKIT_FOUND)
  add_definitions(-DQT_WEBKIT)
endif(QT_QTWEBKIT_FOUND)
