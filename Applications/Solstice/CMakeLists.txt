project(Solstice)

set(_app_options)
if(MITK_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

set(MITK_EXTAPP_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MitkWorkbench.provisioning")

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
  org.blueberry.test
  org.blueberry.uitest
  org.mitk.gui.qt.coreapplication
  org.mitk.gui.qt.diffusionimagingapp
)

FunctionCreateBlueBerryApplication(
  NAME MitkSolstice
  DESCRIPTION "MITK Workbench"
  EXCLUDE_PLUGINS ${_exclude_plugins}
  ${_app_options}
)

add_executable(${OSGI_APP} MACOSX_BUNDLE "src/application/berryMain.cpp")

target_link_libraries(${OSGI_APP} PRIVATE ${PROJECT_NAME} mbilog)
if(_ctk_test_plugins)
  add_dependencies(${OSGI_APP} ${_ctk_test_plugins})
  add_dependencies(BlueBerry ${OSGI_APP})
  set_property(TARGET ${OSGI_APP} APPEND PROPERTY LABELS BlueBerry)
endif()

configure_file(src/application/solstice.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}.ini)


add_executable(${OSGI_UI_APP} MACOSX_BUNDLE "src/application/berryMainUI.cpp")

target_link_libraries(${OSGI_UI_APP} PRIVATE ${PROJECT_NAME} mbilog)
if(MITK_USE_Qt5)
  target_link_libraries(${OSGI_UI_APP} PRIVATE Qt5::Widgets)
endif()

if(_ctk_test_plugins)
  add_dependencies(${OSGI_UI_APP} ${_ctk_test_plugins})
  add_dependencies(BlueBerry ${OSGI_UI_APP})
  set_property(TARGET ${OSGI_UI_APP} APPEND PROPERTY LABELS BlueBerry)
endif()

configure_file(src/application/solstice.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_UI_APP}.ini)

# Add meta dependencies (e.g. on auto-load modules from depending modules)
if(TARGET ${CMAKE_PROJECT_NAME}-autoload)
  add_dependencies(MitkWorkbench ${CMAKE_PROJECT_NAME}-autoload)
endif()

#Setting application icon for macOS systems
set_target_properties(MitkWorkbench PROPERTIES MACOSX_BUNDLE_ICON_FILE "icon.icns")

if(APPLE)
  install(FILES "icons/icon.icns" DESTINATION "MitkWorkbench.app/Contents/Resources")
endif(APPLE)
