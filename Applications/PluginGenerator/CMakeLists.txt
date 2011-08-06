find_package(CTK REQUIRED)

include(${QT_USE_FILE})
include(${CTK_USE_FILE})

set(src_files PluginGenerator.cpp)

qt4_add_resources(src_files plugin_template.qrc project_template.qrc)

add_executable(PluginGenerator ${src_files})
target_link_libraries(PluginGenerator ${CTK_LIBRARIES} ${QT_LIBRARIES})

# subproject support
add_dependencies(MITK-CoreUI PluginGenerator)

# Test the plugin generator
include(mitkTestPluginGenerator)
