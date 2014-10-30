
#! \brief Creates a CTK plugin.
#!
#! This macro should be called from the plugins CMakeLists.txt file.
#! The target name is available after the macro call as ${PLUGIN_TARGET}
#! to add additional libraries in your CMakeLists.txt. Include paths and link
#! libraries are set depending on the value of the Required-Plugins header
#! in your manifest_headers.cmake file.
#!
#! This macro internally calls ctkMacroBuildPlugin() and adds support
#! for Qt Help files and installers.
#!
#! \param EXPORT_DIRECTIVE (required) The export directive to use in the generated
#!        <plugin_target>_Exports.h file.
#! \param EXPORTED_INCLUDE_SUFFIXES (optional) a list of sub-directories which should
#!        be added to the current source directory. The resulting directories
#!        will be available in the set of include directories of depending plug-ins.
#! \param DOXYGEN_TAGFILES (optional) Which external tag files should be available for the plugin documentation
#! \param MOC_OPTIONS (optional) Additional options to pass to the Qt MOC compiler
#! \param TEST_PLUGIN (option) Mark this plug-in as a testing plug-in.
#! \param NO_INSTALL (option) Don't install this plug-in.
macro(MACRO_CREATE_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_PLUGIN "EXPORT_DIRECTIVE;EXPORTED_INCLUDE_SUFFIXES;DOXYGEN_TAGFILES;MOC_OPTIONS" "TEST_PLUGIN;NO_INSTALL;NO_QHP_TRANSFORM" ${ARGN})

  message(STATUS "Creating CTK plugin ${PROJECT_NAME}")

  set(PLUGIN_TARGET ${PROJECT_NAME})

  include(files.cmake)

  set(_PLUGIN_CPP_FILES ${CPP_FILES})
  set(_PLUGIN_MOC_H_FILES ${MOC_H_FILES})
  set(_PLUGIN_UI_FILES ${UI_FILES})
  set(_PLUGIN_CACHED_RESOURCE_FILES ${CACHED_RESOURCE_FILES})
  set(_PLUGIN_TRANSLATION_FILES ${TRANSLATION_FILES})
  set(_PLUGIN_QRC_FILES ${QRC_FILES})
  set(_PLUGIN_H_FILES ${H_FILES})
  set(_PLUGIN_TXX_FILES ${TXX_FILES})
  set(_PLUGIN_DOX_FILES ${DOX_FILES})
  set(_PLUGIN_CMAKE_FILES ${CMAKE_FILES} files.cmake)
  set(_PLUGIN_FILE_DEPENDENCIES ${FILE_DEPENDENCIES})

  if(CTK_PLUGINS_OUTPUT_DIR)
    set(_output_dir "${CTK_PLUGINS_OUTPUT_DIR}")
  else()
    set(_output_dir "")
  endif()

  if(_PLUGIN_TEST_PLUGIN)
    set(is_test_plugin "TEST_PLUGIN")
  else()
    set(is_test_plugin)
  endif()

  # Compute the plugin dependencies
  ctkFunctionGetTargetLibraries(_PLUGIN_target_libraries)


  #------------------------------------------------------------#
  #------------------ Qt Help support -------------------------#

  set(PLUGIN_GENERATED_QCH_FILES )
  if(BLUEBERRY_USE_QT_HELP AND
      EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/documentation/UserManual")
    set(PLUGIN_DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/documentation/UserManual")
    set(PLUGIN_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/documentation/UserManual")

    # Create a list of Doxygen tag files from the plug-in dependencies
    set(PLUGIN_DOXYGEN_TAGFILES)
    foreach(_dep_target ${_PLUGIN_target_libraries})
      string(REPLACE _ . _dep ${_dep_target})

      get_target_property(_is_imported ${_dep_target} IMPORTED)
      if(_is_imported)
        get_target_property(_import_loc_debug ${_dep_target} IMPORTED_LOCATION_DEBUG)
        get_target_property(_import_loc_release ${_dep_target} IMPORTED_LOCATION_RELEASE)
        # There is not necessarily a debug and release build
        if(_import_loc_release)
          set(_import_loc ${_import_loc_release})
        else()
          set(_import_loc ${_import_loc_debug})
        endif()
        get_filename_component(_target_filename "${_import_loc}" NAME)
        # on windows there might be a Debug or Release subdirectory
        string(REGEX REPLACE "/bin/plugins/(Debug/|Release/)?${_target_filename}" "/Plugins/${_dep}/documentation/UserManual" plugin_tag_dir "${_import_loc}" )
      else()
        set(plugin_tag_dir "${CMAKE_BINARY_DIR}/Plugins/${_dep}/documentation/UserManual")
      endif()

      set(_tag_file "${plugin_tag_dir}/${_dep_target}.tag")
      if(EXISTS ${_tag_file})
        set(PLUGIN_DOXYGEN_TAGFILES "${PLUGIN_DOXYGEN_TAGFILES} \"${_tag_file}=qthelp://${_dep}/bundle/\"")
      endif()
    endforeach()
    if(_PLUGIN_DOXYGEN_TAGFILES)
      set(PLUGIN_DOXYGEN_TAGFILES "${PLUGIN_DOXYGEN_TAGFILES} ${_PLUGIN_DOXYGEN_TAGFILES}")
    endif()
    #message("PLUGIN_DOXYGEN_TAGFILES: ${PLUGIN_DOXYGEN_TAGFILES}")

    if(_PLUGIN_NO_QHP_TRANSFORM)
      set(_use_qhp_xsl 0)
    else()
      set(_use_qhp_xsl 1)
    endif()
    _FUNCTION_CREATE_CTK_QT_COMPRESSED_HELP(PLUGIN_GENERATED_QCH_FILES ${_use_qhp_xsl})
    list(APPEND _PLUGIN_CACHED_RESOURCE_FILES ${PLUGIN_GENERATED_QCH_FILES})
  endif()

  #------------------------------------------------------------#
  #------------------ Create Plug-in --------------------------#

  set(_additional_target_libraries )
  if(_PLUGIN_TEST_PLUGIN)
    find_package(CppUnit REQUIRED)
    include_directories(${CppUnit_INCLUDE_DIRS})
    list(APPEND _additional_target_libraries ${CppUnit_LIBRARIES})
  endif()

  if(mbilog_FOUND)
    include_directories(${mbilog_INCLUDE_DIRS})
  endif()

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

  ctkMacroBuildPlugin(
    NAME ${PLUGIN_TARGET}
    EXPORT_DIRECTIVE ${_PLUGIN_EXPORT_DIRECTIVE}
    SRCS ${_PLUGIN_CPP_FILES} ${_PLUGIN_H_FILES} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES}
    MOC_SRCS ${_PLUGIN_MOC_H_FILES}
    MOC_OPTIONS ${_PLUGIN_MOC_OPTIONS}
    UI_FORMS ${_PLUGIN_UI_FILES}
    EXPORTED_INCLUDE_SUFFIXES ${_PLUGIN_EXPORTED_INCLUDE_SUFFIXES}
    RESOURCES ${_PLUGIN_QRC_FILES}
    TARGET_LIBRARIES ${_PLUGIN_target_libraries} ${_additional_target_libraries}
    CACHED_RESOURCEFILES ${_PLUGIN_CACHED_RESOURCE_FILES}
    TRANSLATIONS ${_PLUGIN_TRANSLATION_FILES}
    OUTPUT_DIR ${_output_dir}
    NO_INSTALL # we install the plug-in ourselves
    ${is_test_plugin}
  )

  if(mbilog_FOUND)
    target_link_libraries(${PLUGIN_TARGET} mbilog)
  endif()

  include_directories(${Poco_INCLUDE_DIRS})
  include_directories(${BlueBerry_BINARY_DIR})

  # Only add the following Poco libraries due to possible name clashes
  # in PocoPDF with libpng when also linking QtGui.
  foreach(lib Foundation Util XML)
    target_link_libraries(${PLUGIN_TARGET} ${Poco_${lib}_LIBRARY})
  endforeach()

  # Set compiler flags
  get_target_property(_plugin_compile_flags ${PLUGIN_TARGET} COMPILE_FLAGS)
  if(NOT _plugin_compile_flags)
    set(_plugin_compile_flags "")
  endif()
  if(WIN32)
    set(_plugin_compile_flags "${_plugin_compile_flags} -DPOCO_NO_UNWINDOWS -DWIN32_LEAN_AND_MEAN")
  endif()
  set_target_properties(${PLUGIN_TARGET} PROPERTIES COMPILE_FLAGS "${_plugin_compile_flags}")

  set(_PLUGIN_META_FILES "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml")
    list(APPEND _PLUGIN_META_FILES "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml")
  endif()



  #------------------------------------------------------------#
  #------------------ Installer support -----------------------#
  if(NOT _PLUGIN_NO_INSTALL)
    set(install_directories "")
    if(NOT MACOSX_BUNDLE_NAMES)
      set(install_directories bin/plugins)
    else(NOT MACOSX_BUNDLE_NAMES)
      foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
        list(APPEND install_directories ${bundle_name}.app/Contents/MacOS/plugins)
      endforeach(bundle_name)
    endif(NOT MACOSX_BUNDLE_NAMES)

    foreach(install_subdir ${install_directories})

      MACRO_INSTALL_CTK_PLUGIN(TARGETS ${PLUGIN_TARGET}
                               DESTINATION ${install_subdir})

    endforeach()
  endif()

endmacro()

