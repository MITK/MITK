set(_us_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")

function(usFunctionGenerateModuleInit src_var)
  set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/us_init.cpp")
  configure_file("${_us_cmake_dir}/usModuleInit.cpp" ${module_init_src_file} @ONLY)

  set(_src ${module_init_src_file} ${${src_var}})
  set(${src_var} ${_src} PARENT_SCOPE)
endfunction()
