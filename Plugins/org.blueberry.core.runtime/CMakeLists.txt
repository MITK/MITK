project(org_blueberry_core_runtime)

set(QT_USE_QTXML 1)

mitk_create_plugin(EXPORT_DIRECTIVE org_blueberry_core_runtime_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
                            src/application
                            src/dynamichelpers
                            src/registry
  MODULE_DEPENDS PUBLIC mbilog
)

target_link_libraries(${PLUGIN_TARGET} PUBLIC Poco::Foundation Poco::Util Poco::XML)
if(MITK_USE_Qt5)
  target_link_libraries(${PLUGIN_TARGET} PRIVATE Qt5::Gui Qt5::Xml)
endif()

# Set compiler flags
target_compile_definitions(${PLUGIN_TARGET} PUBLIC "$<$<PLATFORM_ID:Windows>:POCO_NO_UNWINDOWS;WIN32_LEAN_AND_MEAN>")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/berryConfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/berryConfig.h" @ONLY)
