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

mitk_create_plugin(EXPORT_DIRECTIVE BERRY_UI_QT
                        EXPORTED_INCLUDE_SUFFIXES ${PLUGIN_exported_include_suffixes})

if(MITK_USE_Qt5)
  target_link_libraries(${PLUGIN_TARGET} PUBLIC Qt5::Widgets Qt5::PrintSupport Qt5::Svg)
endif()
