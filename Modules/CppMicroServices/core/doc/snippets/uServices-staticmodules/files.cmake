set(snippet_src_files
  main.cpp
)

set(base_dir uServices-staticmodules)
add_library(MyStaticModule STATIC ${base_dir}/MyStaticModule.cpp)
target_link_libraries(MyStaticModule ${PROJECT_TARGET})
set_property(TARGET MyStaticModule APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE US_MODULE_NAME=MyStaticModule)
set_property(TARGET MyStaticModule PROPERTY US_MODULE_NAME MyStaticModule)
set_property(TARGET MyStaticModule PROPERTY FOLDER "${MITK_ROOT_FOLDER}/CppMicroServices/Snippets")

set(snippet_link_libraries
  MyStaticModule
)
