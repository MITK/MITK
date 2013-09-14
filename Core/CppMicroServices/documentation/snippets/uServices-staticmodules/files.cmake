set(snippet_src_files
  main.cpp
)

set(base_dir uServices-staticmodules)
add_library(MyStaticModule STATIC ${base_dir}/MyStaticModule.cpp)
set_property(TARGET MyStaticModule APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE)

set(snippet_link_libraries
  MyStaticModule
)
