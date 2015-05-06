project(org_blueberry_ui_qt_help)

set(QT_USE_QTHELP 1)
set(QT_USE_QTWEBKIT 1)
set(QT_USE_QTNETWORK 1)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_help_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)

if(MITK_USE_Qt5)
  target_link_libraries(${PLUGIN_TARGET} PRIVATE
    Qt5::Help Qt5::OpenGL Qt5::PrintSupport Qt5::WebKitWidgets Qt5::Xml)
else()
  target_link_libraries(${PLUGIN_TARGET} PRIVATE
    ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${QT_QTNETWORK_LIBRARY})
endif()
