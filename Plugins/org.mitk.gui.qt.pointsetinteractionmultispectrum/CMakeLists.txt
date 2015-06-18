project(org_mitk_gui_qt_pointsetinteractionmultispectrum)

if(MITK_USE_Qt5)
  find_package(Qt5OpenGL REQUIRED)
  find_package(Qt5Sql REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Xml REQUIRED)
endif()

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE POINTSETINTERACTIONMULTISPECTRUM_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
