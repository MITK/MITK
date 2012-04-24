set(QT_USE_QTWEBKIT TRUE)
include(${QT_USE_FILE})

if(QT_QTWEBKIT_FOUND)
  add_definitions(-DQT_WEBKIT)
endif(QT_QTWEBKIT_FOUND)

# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_dtiatlasapp)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DTIATLAS_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES Qmitk SceneSerialization
)

target_link_libraries(${PLUGIN_TARGET} ${QT_LIBRARIES})
