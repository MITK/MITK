# MACRO_CREATE_PLUGIN()
#
# Creates the current plugin.
# This macro should be called from the plugins CMakeLists.txt file. The plugin
# library is build in a subdirectory of PLUGIN_OUTPUT_DIR, which is derived from
# the variable PLUGIN_OUTPUT_BASE_DIR coming from COLLECT_PLUGINS().
# The target name is available as PLUGIN_TARGET
# to add additional libraries in your CMakeLists.txt. Include paths and link
# libraries are set depending on the value of the Required-Bundles header
# in your plugins MANIFEST.MF file.
#
# Available variables:
#
# - PLUGIN_OUTPUT_DIR the binary output directory of your plugin
# - PLUGIN_TARGET the target name of your plugins dll
#
#
MACRO(MACRO_CREATE_PLUGIN)

  IF(MSVC80)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
  ENDIF(MSVC80)

  #_MACRO_CREATE_PLUGIN_NAME(PLUGIN_NAME
  #                          INPUT ${CMAKE_CURRENT_SOURCE_DIR}
  #                          BASEDIR ${PLUGINS_SOURCE_BASE_DIR})

  MACRO_PARSE_MANIFEST(${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF)


  MESSAGE(STATUS "Creating plugin ${BUNDLE-SYMBOLICNAME}")

  STRING(REPLACE . _ PLUGIN_TARGET ${BUNDLE-SYMBOLICNAME})

  IF(NOT PLUGIN_CPP_FILES)
    INCLUDE(files.cmake)

    SET(PLUGIN_CPP_FILES ${CPP_FILES})
    SET(PLUGIN_MOC_H_FILES ${MOC_H_FILES})
    SET(PLUGIN_UI_FILES ${UI_FILES})
    SET(PLUGIN_RESOURCE_FILES ${RESOURCE_FILES})
    SET(PLUGIN_RES_FILES ${RES_FILES})
    SET(PLUGIN_H_FILES ${H_FILES})

  ELSE(NOT PLUGIN_CPP_FILES)
    MESSAGE(STATUS "Using PLUGIN_CPP_FILES for plugin ${BUNDLE-SYMBOLICNAME}")
  ENDIF(NOT PLUGIN_CPP_FILES)

  SET(PLUGIN_OUTPUT_DIR ${PLUGINS_OUTPUT_BASE_DIR}/${BUNDLE-SYMBOLICNAME})

  FILE(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})

  # Copy the META-INF directory into the binary output dir
  SOURCE_GROUP("Plugin META Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF)
  SET (PLUGIN_META_FILES ${PLUGIN_META_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF ${PLUGIN_OUTPUT_DIR}/META-INF/MANIFEST.MF COPYONLY)

  # Copy the plugin.xml into the binary output dir
  SET(PLUGINXML ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml)
  IF(EXISTS ${PLUGINXML})
    SOURCE_GROUP("Plugin META Files" FILES ${PLUGINXML})
    SET (PLUGIN_META_FILES ${PLUGIN_META_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml)
    CONFIGURE_FILE(${PLUGINXML} ${PLUGIN_OUTPUT_DIR}/plugin.xml COPYONLY)
  ENDIF(EXISTS ${PLUGINXML})

  # Copy the resource files into the binary output dir
  FOREACH(_resource ${PLUGIN_RESOURCE_FILES})
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${_resource}" "${PLUGIN_OUTPUT_DIR}/${_resource}" COPYONLY)
  ENDFOREACH(_resource ${PLUGIN_RESOURCE_FILES})

  SET(PLUGIN_GENERATED_FILES "")
  IF(PLUGIN_UI_FILES)
    QT4_WRAP_UI(PLUGIN_GENERATED_FILES ${PLUGIN_UI_FILES})
  ENDIF(PLUGIN_UI_FILES)
  IF(PLUGIN_MOC_H_FILES)
    # this is a workaround for Visual Studio. The relative include paths in the generated
    # moc files can get very long and can't be resolved by the MSVC compiler. See bug #1961
    # for details.
    FOREACH(moc_h_file ${PLUGIN_MOC_H_FILES})
      QT4_WRAP_CPP(PLUGIN_GENERATED_FILES ${moc_h_file} OPTIONS -f${moc_h_file})
    ENDFOREACH(moc_h_file)
  ENDIF(PLUGIN_MOC_H_FILES)
  IF(PLUGIN_RES_FILES)
    QT4_ADD_RESOURCES(PLUGIN_GENERATED_FILES ${PLUGIN_RES_FILES})
  ENDIF(PLUGIN_RES_FILES)


  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})
  _MACRO_SETUP_PLUGIN_DEPENDENCIES(_linklibs PLUGIN_NAME ${BUNDLE-SYMBOLICNAME})

  MACRO_ORGANIZE_SOURCES(INPUT ${PLUGIN_CPP_FILES}
                         ${PLUGIN_UI_FILES}
                         ${PLUGIN_RES_FILES}
                         ${PLUGIN_H_FILES}
                         GENERATED ${PLUGIN_GENERATED_FILES})

  SET(_all_target_files
      ${PLUGIN_CPP_FILES}
      ${PLUGIN_UI_FILES}
      ${PLUGIN_RES_FILES}
      ${PLUGIN_H_FILES}
      ${PLUGIN_GENERATED_FILES}
      ${GLOBBED_DOX_FILES}
      ${PLUGIN_META_FILES}
      ${CORRESPONDING__H_FILES}
      ${CORRESPONDING__TXX_FILES}
      )

  #INCLUDE_DIRECTORIES(PLUGIN_OUTPUT_DIR)
  ADD_LIBRARY(${PLUGIN_TARGET} ${_all_target_files})
  LIST(APPEND PLUGIN_TARGETS ${PLUGIN_TARGET})
  SET_TARGET_PROPERTIES(${PLUGIN_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/bin"
    DEBUG_POSTFIX ${OPENCHERRY_DEBUG_POSTFIX})

  # we need to explicitly state the debug versions of the libraries
  # we are linking to in the TARGET_LINK_LIBRARIES command.
  # Although we set CMAKE_DEBUG_POSTFIX to d, CMake automatically
  # appends it in a TARGET_LINK_LIBRARIES(target lib1) command only
  # if lib1 has been build within the same project.
  # External projects using this macro would therefore always link
  # to lib1, instead of lib1d in debug configurations
  SET(_debug_linklibs "")
  FOREACH(_linklib ${PLUGIN_LINK_LIBRARIES})
    SET(_debug_linklibs ${_debug_linklibs} optimized "${_linklib}" debug "${_linklib}${CMAKE_DEBUG_POSTFIX}")
  ENDFOREACH(_linklib)
  #MESSAGE(STATUS "${PLUGIN_TARGET} deps: ${_linklibs}")
  TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} ${_debug_linklibs})
  TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} optimized "PocoFoundation" debug "PocoFoundation${CMAKE_DEBUG_POSTFIX}")

  #SET_TARGET_PROPERTIES(${PLUGIN_TARGET} PROPERTIES PREFIX lib IMPORT_PREFIX lib)
  SET_TARGET_PROPERTIES(${PLUGIN_TARGET} PROPERTIES PREFIX lib)
  
  FILE(RELATIVE_PATH _toplevel_dir "${PLUGINS_SOURCE_BASE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  STRING(REGEX REPLACE ".*/(.+)/?$" "\\1" _toplevel_output_dir ${PLUGINS_OUTPUT_BASE_DIR})
  MACRO_INSTALL_PLUGIN("${PLUGINS_OUTPUT_BASE_DIR}/${_toplevel_dir}" 
                       TARGETS ${PLUGIN_TARGET}
					   DESTINATION "bin/${_toplevel_output_dir}")

ENDMACRO(MACRO_CREATE_PLUGIN)


# Macro to set specific Qt options, calls CREATE_PLUGIN at the end
MACRO(MACRO_CREATE_QT_PLUGIN)

  IF(QT4_FOUND)
    INCLUDE(${QT_USE_FILE})

    MACRO_CREATE_PLUGIN()

    TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} ${QT_LIBRARIES})
  ENDIF(QT4_FOUND)

ENDMACRO(MACRO_CREATE_QT_PLUGIN)
