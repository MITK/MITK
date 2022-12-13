project(org_blueberry_ui_qt)

set(PLUGIN_exported_include_suffixes
  src
  src/actions
  src/application
  src/commands
  src/guitk
  src/handlers
  src/intro
  src/model
  src/presentations
  src/services
  src/testing
  src/tweaklets
  src/util
)

if(MITK_USE_Qt5)
  set(PLUGIN_package_depends
    PUBLIC Qt5|Widgets+PrintSupport+Svg
  )
endif()

mitk_create_plugin(
  EXPORT_DIRECTIVE BERRY_UI_QT
  EXPORTED_INCLUDE_SUFFIXES ${PLUGIN_exported_include_suffixes}
  MODULE_DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS ${PLUGIN_package_depends}
)
