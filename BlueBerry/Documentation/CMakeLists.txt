# 
# Variables:
#  BLUEBERRY_DOCS_OUTPUT_DIR: documentation output directory (optional)

add_subdirectory(snippets)

find_package(Doxygen)

if(DOXYGEN_FOUND)

option(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
mark_as_advanced(USE_DOT)

set(HAVE_DOT "NO")
if(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  set(HAVE_DOT "YES")
endif()

set(BLUEBERRY_DOCS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "BlueBerry output directory for all generated documentation.")
set(BLUEBERRY_DOXYGEN_OUTPUT_DIR ${BLUEBERRY_DOCS_OUTPUT_DIR}/reference/api CACHE INTERNAL "BlueBerry doxygen output directory")
set(BLUEBERRY_XPDOC_OUTPUT_DIR ${BLUEBERRY_DOCS_OUTPUT_DIR}/reference/extension-points)

set(BLUEBERRY_DOXYGEN_TAGFILE_NAME ${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/BlueBerry.tag CACHE INTERNAL "BlueBerry Doxygen tag file")

# This is relative to the working directory of the doxygen command
set(BLUEBERRY_DOXYGEN_STYLESHEET bb_doxygen.css)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${BLUEBERRY_DOXYGEN_STYLESHEET}
               ${CMAKE_CURRENT_BINARY_DIR}/${BLUEBERRY_DOXYGEN_STYLESHEET} @COPYONLY)

file(RELATIVE_PATH BLUEBERRY_DOXYGEN_XP_RELATIVEDIR ${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/html ${BLUEBERRY_XPDOC_OUTPUT_DIR}/)

set(_doxygen_qt4bundles )
set(_doxygen_bundles )
set(_doxygen_binary_qt4bundles )
set(_doxygen_binary_bundles )
file(GLOB _bundle_dirs RELATIVE "${BLUEBERRY_PLUGINS_SOURCE_DIR}" "${BLUEBERRY_PLUGINS_SOURCE_DIR}/*")
foreach(_bundle_dir ${_bundle_dirs})
  string(REGEX MATCH "^\\..*" _matched_hidden "${_bundle_dir}")
  if(NOT _matched_hidden)
    if(IS_DIRECTORY "${BLUEBERRY_PLUGINS_SOURCE_DIR}/${_bundle_dir}")
      string(REGEX MATCH ".*\\.qt.*" _matched "${_bundle_dir}")
      if(_matched)
        if(QT4_FOUND)
          set(_doxygen_qt4bundles "${_doxygen_qt4bundles} \"${BLUEBERRY_PLUGINS_SOURCE_DIR}/${_bundle_dir}\"")
          set(_doxygen_binary_qt4bundles  "${_doxygen_binary_qt4bundles} \"${BLUEBERRY_PLUGINS_BINARY_DIR}/${_bundle_dir}\"")
        endif()
      else()
        set(_doxygen_bundles "${_doxygen_bundles} \"${BLUEBERRY_PLUGINS_SOURCE_DIR}/${_bundle_dir}\"")
        set(_doxygen_binary_bundles  "${_doxygen_binary_bundles} \"${BLUEBERRY_PLUGINS_BINARY_DIR}/${_bundle_dir}\"")
      endif()
    endif()
  endif()
endforeach()

# Compile a doxygen input filter for processing CMake scripts
include(FunctionCMakeDoxygenFilterCompile)
FunctionCMakeDoxygenFilterCompile(NAMESPACE "CMake")

configure_file(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)
               
configure_file(reference/extension-points/index.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/reference/extension-points/index.dox)

find_package(Doxygen)

add_custom_target(BlueBerryDoc 
                  ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  DEPENDS ${_BlueBerryDoc_depends}
                  )
                  
# convert the extension points schema files into html
if(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)
  file(GLOB_RECURSE _plugin_xmls  ${BLUEBERRY_PLUGINS_SOURCE_DIR}/plugin.xml)
  MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
                       OUTPUT_DIR "${BLUEBERRY_XPDOC_OUTPUT_DIR}"
                       TARGET_NAME BlueBerryXPDoc
                       )

  add_dependencies(BlueBerryDoc BlueBerryXPDoc)
endif(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)

configure_file(schema.css ${BLUEBERRY_DOCS_OUTPUT_DIR}/schema.css)

endif(DOXYGEN_FOUND)
