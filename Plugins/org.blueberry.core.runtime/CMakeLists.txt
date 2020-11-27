project(org_blueberry_core_runtime)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_core_runtime_EXPORT
  EXPORTED_INCLUDE_SUFFIXES
    src
    src/application
    src/dynamichelpers
    src/registry
  MODULE_DEPENDS PUBLIC mbilog
  PACKAGE_DEPENDS
    PUBLIC Poco|Foundation+Util+XML
    PRIVATE Qt5|Gui+Xml
)

target_compile_definitions(${PLUGIN_TARGET} PUBLIC "$<$<PLATFORM_ID:Windows>:POCO_NO_UNWINDOWS;WIN32_LEAN_AND_MEAN>")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/berryConfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/berryConfig.h" @ONLY)
