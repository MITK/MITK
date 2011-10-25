# MACRO_CREATE_CTK_PLUGIN()
#
# Creates a CTK plugin.
# This macro should be called from the plugins CMakeLists.txt file.
# The target name is available after the macro call as ${PROJECT_NAME}
# to add additional libraries in your CMakeLists.txt. Include paths and link
# libraries are set depending on the value of the Required-Plugins header
# in your manifest_headers.cmake file.
#
MACRO(MACRO_CREATE_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_PLUGIN "EXPORT_DIRECTIVE;EXPORTED_INCLUDE_SUFFIXES" "TEST_PLUGIN" ${ARGN})

  MESSAGE(STATUS "Creating CTK plugin ${PROJECT_NAME}")

  SET(PLUGIN_TARGET ${PROJECT_NAME})

  INCLUDE(files.cmake)

  SET(_PLUGIN_CPP_FILES ${CPP_FILES})
  SET(_PLUGIN_MOC_H_FILES ${MOC_H_FILES})
  SET(_PLUGIN_UI_FILES ${UI_FILES})
  SET(_PLUGIN_CACHED_RESOURCE_FILES ${CACHED_RESOURCE_FILES})
  SET(_PLUGIN_TRANSLATION_FILES ${TRANSLATION_FILES})
  SET(_PLUGIN_QRC_FILES ${QRC_FILES})
  SET(_PLUGIN_H_FILES ${H_FILES})
  SET(_PLUGIN_TXX_FILES ${TXX_FILES})
  SET(_PLUGIN_DOX_FILES ${DOX_FILES})
  SET(_PLUGIN_CMAKE_FILES ${CMAKE_FILES} files.cmake)
  SET(_PLUGIN_FILE_DEPENDENCIES ${FILE_DEPENDENCIES})

  IF(CTK_PLUGINS_OUTPUT_DIR)
    SET(_output_dir "${CTK_PLUGINS_OUTPUT_DIR}")
  ELSE()
    SET(_output_dir "")
  ENDIF()

  IF(_PLUGIN_TEST_PLUGIN)
    SET(is_test_plugin "TEST_PLUGIN")
  ELSE()
    SET(is_test_plugin)
  ENDIF()

  #------------------------------------------------------------#
  #------------------ Qt Help support -------------------------#

  SET(PLUGIN_GENERATED_QCH_FILES )
  IF (BLUEBERRY_USE_QT_HELP AND
      EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/documentation/UserManual")
    SET(PLUGIN_DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/documentation/UserManual")
    SET(PLUGIN_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/documentation/UserManual")
    _FUNCTION_CREATE_CTK_QT_COMPRESSED_HELP(PLUGIN_GENERATED_QCH_FILES )
    LIST(APPEND _PLUGIN_CACHED_RESOURCE_FILES ${PLUGIN_GENERATED_QCH_FILES})
  ENDIF()

  # Compute the plugin dependencies
  ctkFunctionGetTargetLibraries(_PLUGIN_target_libraries)

  ctkMacroBuildPlugin(
    NAME ${PLUGIN_TARGET}
    EXPORT_DIRECTIVE ${_PLUGIN_EXPORT_DIRECTIVE}
    SRCS ${_PLUGIN_CPP_FILES}
    MOC_SRCS ${_PLUGIN_MOC_H_FILES}
    UI_FORMS ${_PLUGIN_UI_FILES}
    EXPORTED_INCLUDE_SUFFIXES ${_PLUGIN_EXPORTED_INCLUDE_SUFFIXES}
    RESOURCES ${_PLUGIN_QRC_FILES}
    TARGET_LIBRARIES ${_PLUGIN_target_libraries}
    CACHED_RESOURCEFILES ${_PLUGIN_CACHED_RESOURCE_FILES}
    TRANSLATIONS ${_PLUGIN_TRANSLATION_FILES}
    OUTPUT_DIR ${_output_dir}
    ${is_test_plugin}
  )

  IF(mbilog_FOUND)
    TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} mbilog)
  ENDIF()
  
  INCLUDE_DIRECTORIES(${Poco_INCLUDE_DIRS})

  TARGET_LINK_LIBRARIES(${PLUGIN_TARGET}
    optimized PocoFoundation debug PocoFoundationd
    optimized PocoUtil debug PocoUtild
    optimized PocoXML debug PocoXMLd
  )

  SET(_PLUGIN_META_FILES "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml")
    LIST(APPEND _PLUGIN_META_FILES "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml")
  ENDIF()

  MACRO_ORGANIZE_SOURCES(
    SOURCE ${_PLUGIN_CPP_FILES}
    HEADER ${_PLUGIN_H_FILES}
    TXX ${_PLUGIN_TXX_FILES}
    DOC ${_PLUGIN_DOX_FILES}
    UI ${_PLUGIN_UI_FILES}
    QRC ${_PLUGIN_QRC_FILES} ${_PLUGIN_CACHED_RESOURCE_FILES}
    META ${_PLUGIN_META_FILES}
    MOC ${MY_MOC_CPP}
    GEN_UI ${MY_UI_CPP}
    GEN_QRC ${MY_QRC_SRCS}
  )

  
  #------------------------------------------------------------#
  #------------------ Installer support -----------------------#
  IF(NOT _PLUGIN_TEST_PLUGIN)
    SET(install_directories "")
    IF(NOT MACOSX_BUNDLE_NAMES)
      SET(install_directories bin/plugins)
    ELSE(NOT MACOSX_BUNDLE_NAMES)
      FOREACH(bundle_name ${MACOSX_BUNDLE_NAMES})
        LIST(APPEND install_directories ${bundle_name}.app/Contents/MacOS/plugins)
      ENDFOREACH(bundle_name)
    ENDIF(NOT MACOSX_BUNDLE_NAMES)

    FOREACH(install_subdir ${install_directories})

      MACRO_INSTALL_CTK_PLUGIN(TARGETS ${PLUGIN_TARGET}
                               DESTINATION ${install_subdir})

    ENDFOREACH()
  ENDIF()

ENDMACRO()

