project(org_blueberry_ui_qt_help)

set(QT_USE_QTHELP 1)
set(QT_USE_QTWEBKIT 1)
set(QT_USE_QTNETWORK 1)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_help_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)

if(MITK_USE_Qt5)
  qt5_use_modules(${PLUGIN_TARGET} Help OpenGL PrintSupport WebKitWidgets Xml)
endif()

target_link_libraries(${PROJECT_NAME}
  ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${QT_QTNETWORK_LIBRARY})
