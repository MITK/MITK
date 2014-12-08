project(org_blueberry_ui_qt)

MACRO_CREATE_CTK_PLUGIN(EXPORT_DIRECTIVE BERRY_UI_QT
                        EXPORTED_INCLUDE_SUFFIXES src src/application)

if(MITK_USE_Qt5)
  qt5_use_modules(${PLUGIN_TARGET} Widgets)
endif()
