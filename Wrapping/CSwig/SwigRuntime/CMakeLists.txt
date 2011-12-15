INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

MACRO(CREATE_SWIG_RUNTIME lang_option lang_postfix lang_link_libs)
  SET(source_file "${CMAKE_CURRENT_BINARY_DIR}/swigrun${lang_postfix}.cxx")
  SET(lib_name "SwigRuntime${lang_postfix}")
  
  ADD_LIBRARY(${lib_name} SHARED "${source_file}") 
  TARGET_LINK_LIBRARIES(${lib_name} ${lang_link_libs})

  # work around linkage problem on some solaris systems
  IF(CMAKE_SYSTEM MATCHES "SunOS-." AND CMAKE_COMPILER_IS_GNUCXX AND CMAKE_COMPILER_IS_GNUCC)
    TARGET_LINK_LIBRARIES(${lib_name} stdc++)
  ENDIF(CMAKE_SYSTEM MATCHES "SunOS-." AND CMAKE_COMPILER_IS_GNUCXX AND CMAKE_COMPILER_IS_GNUCC)

  INSTALL_TARGETS("${WRAP_ITK_INSTALL_PREFIX}/lib" ${lib_name})
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES LINK_FLAGS "${CSWIG_EXTRA_LINKFLAGS}")

  ADD_CUSTOM_COMMAND(
   SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swigrun.h
   COMMAND ${GCCXML}
   ARGS -fxml-start=_cable_ -DCABLE_CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/swigrun.h
      -fxml=${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml 
   TARGET ${lib_name}
   OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml
   DEPENDS ${GCCXML})

  ADD_CUSTOM_COMMAND(
    SOURCE ${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml
    COMMAND ${CSWIG}
    ARGS -o "${source_file}"
         ${lang_option}
         -c++ ${CMAKE_CURRENT_BINARY_DIR}/swigrun.xml
    TARGET ${lib_name}
    OUTPUTS "${source_file}"
    DEPENDS ${CSWIG})
ENDMACRO(CREATE_SWIG_RUNTIME)

IF(MITK_USE_Python)
  SET(PYLIBS ${PYTHON_LIBRARY})
  IF(PYTHON_DEBUG_LIBRARY)
     SET(PYLIBS debug ${PYTHON_DEBUG_LIBRARY} optimized ${PYTHON_LIBRARY})
  ENDIF()
  #message(warning "PYLIBS: ${PYLIBS}")
  CREATE_SWIG_RUNTIME(-python Python "${PYLIBS}")
ENDIF(MITK_USE_Python)

