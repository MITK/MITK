project(org_blueberry_osgi)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE BERRY_OSGI
  EXPORTED_INCLUDE_SUFFIXES src src/application src/event src/service
  )

if(MITK_USE_Qt5)
  qt5_use_modules(${PLUGIN_TARGET} Concurrent Gui Sql)
endif()

add_executable(${OSGI_APP} MACOSX_BUNDLE "src/application/berryMain.cpp")

target_link_libraries(${OSGI_APP} ${PROJECT_NAME} mbilog)
if(_ctk_test_plugins)
  add_dependencies(${OSGI_APP} ${_ctk_test_plugins})
  add_dependencies(BlueBerry ${OSGI_APP})
  set_property(TARGET ${OSGI_APP} APPEND PROPERTY LABELS BlueBerry)
endif()

configure_file(src/application/solstice.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}.ini)


add_executable(${OSGI_UI_APP} MACOSX_BUNDLE "src/application/berryMainUI.cpp")

if(MITK_USE_Qt5)
  qt5_use_modules(${OSGI_UI_APP} Widgets)
endif()

target_link_libraries(${OSGI_UI_APP} ${PROJECT_NAME} mbilog)
if(_ctk_test_plugins)
  add_dependencies(${OSGI_UI_APP} ${_ctk_test_plugins})
  add_dependencies(BlueBerry ${OSGI_UI_APP})
  set_property(TARGET ${OSGI_UI_APP} APPEND PROPERTY LABELS BlueBerry)
endif()

configure_file(src/application/solstice.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_UI_APP}.ini)

set(BLUEBERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/berryConfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/berryConfig.h" @ONLY)

