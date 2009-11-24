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

  MESSAGE(STATUS "Creating plugin ${BUNDLE-SYMBOLICNAME}")

  STRING(REPLACE . _ PLUGIN_TARGET ${BUNDLE-SYMBOLICNAME})

  SET(PLUGIN_OUTPUT_DIR ${${BUNDLE-SYMBOLICNAME}_OUT_DIR})
  FILE(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})

  INCLUDE(files.cmake)

  SET(PLUGIN_CPP_FILES ${CPP_FILES})
  SET(PLUGIN_MOC_H_FILES ${MOC_H_FILES})
  SET(PLUGIN_UI_FILES ${UI_FILES})
  SET(PLUGIN_RESOURCE_FILES ${RESOURCE_FILES})
  SET(PLUGIN_RES_FILES ${RES_FILES})
  SET(PLUGIN_H_FILES ${H_FILES})
  SET(PLUGIN_TXX_FILES ${TXX_FILES})
  SET(PLUGIN_DOX_FILES ${DOX_FILES})
  SET(PLUGIN_CMAKE_FILES ${CMAKE_FILES} files.cmake)
  SET(PLUGIN_FILE_DEPENDENCIES ${FILE_DEPENDENCIES})


  # Copy the META-INF directory into the binary output dir
 
  SET (PLUGIN_META_FILES ${PLUGIN_META_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF ${PLUGIN_OUTPUT_DIR}/META-INF/MANIFEST.MF COPYONLY)

  # Copy the plugin.xml into the binary output dir
  SET(PLUGINXML ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml)
  IF(EXISTS ${PLUGINXML})
    SET (PLUGIN_META_FILES ${PLUGIN_META_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml)
    CONFIGURE_FILE(${PLUGINXML} ${PLUGIN_OUTPUT_DIR}/plugin.xml COPYONLY)
  ENDIF(EXISTS ${PLUGINXML})

  # Copy the resource files into the binary output dir
  FOREACH(_resource ${PLUGIN_RESOURCE_FILES})
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${_resource}" "${PLUGIN_OUTPUT_DIR}/${_resource}" COPYONLY)
  ENDFOREACH(_resource ${PLUGIN_RESOURCE_FILES})

  SET(PLUGIN_GENERATED_UI_FILES )
  IF(PLUGIN_UI_FILES)
    QT4_WRAP_UI(PLUGIN_GENERATED_UI_FILES ${PLUGIN_UI_FILES})
  ENDIF(PLUGIN_UI_FILES)
  
  SET(PLUGIN_GENERATED_MOC_FILES )
  IF(PLUGIN_MOC_H_FILES)
    # this is a workaround for Visual Studio. The relative include paths in the generated
    # moc files can get very long and can't be resolved by the MSVC compiler. See bug #1961
    # for details.
    FOREACH(moc_h_file ${PLUGIN_MOC_H_FILES})
      QT4_WRAP_CPP(PLUGIN_GENERATED_MOC_FILES ${moc_h_file} OPTIONS -f${moc_h_file})
    ENDFOREACH(moc_h_file)
  ENDIF(PLUGIN_MOC_H_FILES)
  
  SET(PLUGIN_GENERATED_QRC_FILES )
  IF(PLUGIN_RES_FILES)
    QT4_ADD_RESOURCES(PLUGIN_GENERATED_QRC_FILES ${PLUGIN_RES_FILES})
  ENDIF(PLUGIN_RES_FILES)
  
  #------------------------------------------------------------#
  #------------------ Qt Help support -------------------------#
  
  SET(PLUGIN_GENERATED_QCH_FILES )
  IF (OPENCHERRY_USE_QT_HELP AND
      EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/documentation/UserManual")
    SET(PLUGIN_DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/documentation/UserManual")
    SET(PLUGIN_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/documentation/UserManual")
    _MACRO_CREATE_QT_COMPRESSED_HELP(PLUGIN_GENERATED_QCH_FILES )
  ENDIF()


  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})
  _MACRO_SETUP_PLUGIN_DEPENDENCIES(_linklibs)

  MACRO_ORGANIZE_SOURCES(SOURCE ${PLUGIN_CPP_FILES}
                         HEADER ${PLUGIN_H_FILES}
						 TXX ${PLUGIN_TXX_FILES}
						 DOC ${PLUGIN_DOX_FILES}
                         UI ${PLUGIN_UI_FILES}
                         QRC ${PLUGIN_RES_FILES}
						 META ${PLUGIN_META_FILES}
                         MOC ${PLUGIN_GENERATED_MOC_FILES}
						 GEN_UI ${PLUGIN_GENERATED_UI_FILES}
						 GEN_QRC ${PLUGIN_GENERATED_QRC_FILES})

  SET(_all_target_files
      ${PLUGIN_CPP_FILES}
	  ${PLUGIN_H_FILES}
	  ${PLUGIN_TXX_FILES}
	  ${PLUGIN_DOX_FILES}
      ${PLUGIN_UI_FILES}
      ${PLUGIN_RES_FILES}
      ${PLUGIN_META_FILES}
	  ${PLUGIN_GENERATED_MOC_FILES}
      ${PLUGIN_GENERATED_UI_FILES}
      ${PLUGIN_GENERATED_QRC_FILES}
      ${PLUGIN_GENERATED_QCH_FILES}
	  ${PLUGIN_CMAKE_FILES}
	  ${CORRESPONDING__H_FILES}
      ${GLOBBED__H_FILES}
      ${PLUGIN_FILE_DEPENDENCIES}
      )

  ADD_LIBRARY(${PLUGIN_TARGET} ${_all_target_files})
  SET(PLUGIN_TARGETS  ${PLUGIN_TARGETS} ${PLUGIN_TARGET} CACHE INTERNAL "Temporary list of plug-in targets")
  SET_TARGET_PROPERTIES(${PLUGIN_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/bin"
    DEBUG_POSTFIX ${OPENCHERRY_DEBUG_POSTFIX})

  IF(mbilog_FOUND)
    TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} mbilog)
  ENDIF()

  # we need to explicitly state the debug versions of the libraries
  # we are linking to in the TARGET_LINK_LIBRARIES command.
  # Although we set the debug postfix to d, CMake automatically
  # appends it in a TARGET_LINK_LIBRARIES(target lib1) command only
  # if lib1 has been build within the same project.
  # External projects using this macro would therefore always link
  # to lib1, instead of lib1d in debug configurations
  SET(_debug_linklibs "")
  FOREACH(_linklib ${PLUGIN_LINK_LIBRARIES})
    SET(_debug_linklibs ${_debug_linklibs} optimized "${_linklib}" debug "${_linklib}${OPENCHERRY_DEBUG_POSTFIX}")
  ENDFOREACH(_linklib)
  
  TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} ${_debug_linklibs})
  TARGET_LINK_LIBRARIES(${PLUGIN_TARGET}
                        optimized "PocoFoundation" debug "PocoFoundation${OPENCHERRY_DEBUG_POSTFIX}"
                        optimized "PocoUtil" debug "PocoUtil${OPENCHERRY_DEBUG_POSTFIX}"
                        optimized "PocoXML" debug "PocoXML${OPENCHERRY_DEBUG_POSTFIX}")

  #SET_TARGET_PROPERTIES(${PLUGIN_TARGET} PROPERTIES PREFIX lib IMPORT_PREFIX lib)
  SET_TARGET_PROPERTIES(${PLUGIN_TARGET} PROPERTIES PREFIX lib)
  
  
  #------------------------------------------------------------#
  #------------------ Installer support -----------------------#
  
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
