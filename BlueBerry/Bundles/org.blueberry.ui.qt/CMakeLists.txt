project(org_blueberry_ui_qt)

MACRO_CREATE_CTK_PLUGIN(EXPORT_DIRECTIVE BERRY_UI_QT
                        EXPORTED_INCLUDE_SUFFIXES src src/application)

target_link_libraries(${PLUGIN_TARGET} PUBLIC Qt5::Widgets)
