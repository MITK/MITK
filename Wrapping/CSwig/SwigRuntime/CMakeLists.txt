include_directories(${CMAKE_CURRENT_SOURCE_DIR})

macro(CREATE_SWIG_RUNTIME lang_option lang_postfix lang_link_libs)
  set(source_file "${CMAKE_CURRENT_BINARY_DIR}/swigrun${lang_postfix}.cxx")
  set(lib_name "SwigRuntime${lang_postfix}")
  
  add_library(${lib_name} SHARED "${source_file}") 
  target_link_libraries(${lib_name} ${lang_link_libs})

  # work around linkage problem on some solaris systems
  if(CMAKE_SYSTEM MATCHES "SunOS-." AND CMAKE_COMPILER_IS_GNUCXX AND CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(${lib_name} stdc++)
  endif(CMAKE_SYSTEM MATCHES "SunOS-." AND CMAKE_COMPILER_IS_GNUCXX AND CMAKE_COMPILER_IS_GNUCC)

  install_targets("${WRAP_ITK_INSTALL_PREFIX}/lib" ${lib_name})
  set_target_properties(${lib_name} PROPERTIES LINK_FLAGS "${CSWIG_EXTRA_LINKFLAGS}")

  add_custom_command(
   SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swigrun.h
   COMMAND ${GCCXML}
   ARGS -fxml-start=_cable_ -DCABLE_CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/swigrun.h
      -fxml=${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml 
   TARGET ${lib_name}
   OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml
   DEPENDS ${GCCXML})

  add_custom_command(
    SOURCE ${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml
    COMMAND ${CSWIG}
    ARGS -o "${source_file}"
         ${lang_option}
         -c++ ${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml
    TARGET ${lib_name}
    OUTPUTS "${source_file}"
    DEPENDS ${CSWIG})
endmacro(CREATE_SWIG_RUNTIME)

if(MITK_USE_Python)
  set(PYLIBS ${PYTHON_LIBRARY})
  if(PYTHON_DEBUG_LIBRARY)
     set(PYLIBS debug ${PYTHON_DEBUG_LIBRARY} optimized ${PYTHON_LIBRARY})
  endif()
  #message(warning "PYLIBS: ${PYLIBS}")
  CREATE_SWIG_RUNTIME(-python Python "${PYLIBS}")
endif(MITK_USE_Python)

