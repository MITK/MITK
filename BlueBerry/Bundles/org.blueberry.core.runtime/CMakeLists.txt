project(org_blueberry_core_runtime)

set(QT_USE_QTXML 1)

MACRO_CREATE_CTK_PLUGIN(EXPORT_DIRECTIVE org_blueberry_core_runtime_EXPORT
                        EXPORTED_INCLUDE_SUFFIXES src src/application src/registry)

target_link_libraries(${PLUGIN_TARGET} PUBLIC Poco::Foundation Poco::Util Poco::XML)
if(MITK_USE_Qt5)
  target_link_libraries(${PLUGIN_TARGET} PRIVATE Qt5::Gui Qt5::Xml)
endif()

# Set compiler flags
target_compile_definitions(${PLUGIN_TARGET} PUBLIC "$<$<PLATFORM_ID:Windows>:POCO_NO_UNWINDOWS;WIN32_LEAN_AND_MEAN>")

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/berryConfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/berryConfig.h" @ONLY)
