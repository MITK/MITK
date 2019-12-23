
#! \brief Creates a MITK CTK plugin.
#!
#! This function should be called from the plugins CMakeLists.txt file.
#! The target name is available after the macro call as ${PLUGIN_TARGET}
#! to add additional libraries in your CMakeLists.txt. Include paths and link
#! libraries are set depending on the value of the Required-Plugins header
#! in your manifest_headers.cmake file.
#!
#! This function internally calls ctkMacroBuildPlugin() and adds support
#! for Qt Help files and installers.
#!
#! Options:
#! \param TEST_PLUGIN Mark this plug-in as a testing plug-in.
#! \param NO_INSTALL  Don't install this plug-in.
#!
#! Parameters:
#!
#! \param EXPORT_DIRECTIVE (required) The export directive to use in the generated
#!        <plugin_target>_Exports.h file.
#!
#! Multi-value parameters (all optional):
#!
#! \param EXPORTED_INCLUDE_SUFFIXES A list of sub-directories which should
#!        be added to the current source directory. The resulting directories
#!        will be available in the set of include directories of depending plug-ins.
#! \param MODULE_DEPENDS (optional) A list of Modules this plug-in depends on.
#! \param PACKAGE_DEPENDS (optional) A list of external packages this plug-in depends on.
#! \param DOXYGEN_TAGFILES (optional) Which external tag files should be available for the plugin documentation
#! \param MOC_OPTIONS (optional) Additional options to pass to the Qt MOC compiler
#! \param WARNINGS_NO_ERRORS (optional) Do not handle compiler warnings as errors
function(mitk_create_plugin)

  # options
  set(arg_options
    TEST_PLUGIN # Mark this plug-in as a testing plug-in
    NO_INSTALL  # Don't install this plug-in
    NO_QHP_TRANSFORM
    WARNINGS_NO_ERRORS
  )

  # single value arguments
  set(arg_single
    EXPORT_DIRECTIVE # (required) TODO: could be generated via CMake as it is done for MITK modules already
  )

  # multiple value arguments
  set(arg_multiple
    EXPORTED_INCLUDE_SUFFIXES # (optional) additional public include directories
    MODULE_DEPENDS            # (optional)
    PACKAGE_DEPENDS
    DOXYGEN_TAGFILES
    MOC_OPTIONS
    SUBPROJECTS
  )

  cmake_parse_arguments(_PLUGIN "${arg_options}" "${arg_single}" "${arg_multiple}" ${ARGN})

  if(_PLUGIN_TEST_PLUGIN)
    set(_PLUGIN_NO_INSTALL 1)
    set(is_test_plugin "TEST_PLUGIN")
  else()
    set(is_test_plugin)
  endif()

  set(_PLUGIN_MOC_OPTIONS "-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED ${_PLUGIN_MOC_OPTIONS}")

  set(PLUGIN_TARGET ${PROJECT_NAME})

  mitk_check_module_dependencies(MODULES ${_PLUGIN_MODULE_DEPENDS}
                                 PACKAGES ${_PLUGIN_PACKAGE_DEPENDS}
                                 MISSING_DEPENDENCIES_VAR _missing_deps
                                 MODULE_DEPENDENCIES_VAR _module_deps
                                 PACKAGE_DEPENDENCIES_VAR _package_deps)

  if(_missing_deps)
    if(NOT MITK_BUILD_ALL_PLUGINS)
      message(SEND_ERROR "${PROJECT_NAME} is missing requirements and won't be built. Missing: ${_missing_deps}")
    else()
      message(STATUS "${PROJECT_NAME} is missing requirements and won't be built. Missing: ${_missing_deps}")
    endif()
    return()
  endif()

  # -------------- All dependencies are resolved ------------------

  message(STATUS "Creating CTK plugin ${PROJECT_NAME}")

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

  # Compute the plugin dependencies
  ctkFunctionGetTargetLibraries(_PLUGIN_target_libraries "")

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

  mitkFunctionOrganizeSources(
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
    TARGET_LIBRARIES ${_PLUGIN_target_libraries}
    CACHED_RESOURCEFILES ${_PLUGIN_CACHED_RESOURCE_FILES}
    TRANSLATIONS ${_PLUGIN_TRANSLATION_FILES}
    OUTPUT_DIR ${_output_dir}
    NO_INSTALL # we install the plug-in ourselves
    ${is_test_plugin}
  )

  mitk_use_modules(TARGET ${PLUGIN_TARGET}
    MODULES ${_PLUGIN_MODULE_DEPENDS}
    PACKAGES ${_PLUGIN_PACKAGE_DEPENDS}
  )

  set_property(TARGET ${PLUGIN_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${PLUGIN_TARGET})
  set_property(TARGET ${PLUGIN_TARGET} PROPERTY US_MODULE_NAME ${PLUGIN_TARGET})

  if(NOT CMAKE_CURRENT_SOURCE_DIR MATCHES "^${CMAKE_SOURCE_DIR}.*")
    foreach(MITK_EXTENSION_DIR ${MITK_EXTENSION_DIRS})
      if(CMAKE_CURRENT_SOURCE_DIR MATCHES "^${MITK_EXTENSION_DIR}.*")
        get_filename_component(MITK_EXTENSION_ROOT_FOLDER ${MITK_EXTENSION_DIR} NAME)
        set_property(TARGET ${PLUGIN_TARGET} PROPERTY FOLDER "${MITK_EXTENSION_ROOT_FOLDER}/Plugins")
        break()
      endif()
    endforeach()
  else()
    set_property(TARGET ${PLUGIN_TARGET} PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Plugins")
  endif()

  set(plugin_c_flags)
  set(plugin_cxx_flags)

  if(NOT _PLUGIN_WARNINGS_NO_ERRORS)
    if(MSVC_VERSION)
      mitkFunctionCheckCAndCXXCompilerFlags("/WX" plugin_c_flags plugin_cxx_flags)
    else()
      mitkFunctionCheckCAndCXXCompilerFlags(-Werror plugin_c_flags plugin_cxx_flags)
      mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=c++0x-static-nonintegral-init" plugin_c_flags plugin_cxx_flags)
      mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=static-member-init" plugin_c_flags plugin_cxx_flags)
      mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unknown-warning" plugin_c_flags plugin_cxx_flags)
      mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=gnu" plugin_c_flags plugin_cxx_flags)
      mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=cast-function-type" plugin_c_flags plugin_cxx_flags)
      mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=inconsistent-missing-override" plugin_c_flags plugin_cxx_flags)
    endif()
  endif()

  if(plugin_c_flags)
    string(REPLACE " " ";" plugin_c_flags "${plugin_c_flags}")
    target_compile_options(${PLUGIN_TARGET} PRIVATE ${plugin_c_flags})
  endif()

  if(plugin_cxx_flags)
    string(REPLACE " " ";" plugin_cxx_flags "${plugin_cxx_flags}")
    target_compile_options(${PLUGIN_TARGET} PRIVATE ${plugin_cxx_flags})
  endif()

  if(NOT MY_SUBPROJECTS)
    if(MITK_DEFAULT_SUBPROJECTS)
      set(MY_SUBPROJECTS ${MITK_DEFAULT_SUBPROJECTS})
    elseif(TARGET MITK-Plugins)
      set(MY_SUBPROJECTS MITK-Plugins)
    endif()
  endif()

  if(MY_SUBPROJECTS)
    set_property(TARGET ${PLUGIN_TARGET} PROPERTY LABELS ${MY_SUBPROJECTS})
    foreach(subproject ${MY_SUBPROJECTS})
      add_dependencies(${subproject} ${PLUGIN_TARGET})
    endforeach()
  endif()

  if(_PLUGIN_TEST_PLUGIN)
    find_package(CppUnit REQUIRED)
    target_include_directories(${PLUGIN_TARGET} PRIVATE ${CppUnit_INCLUDE_DIRS})
    target_link_libraries(${PLUGIN_TARGET} PRIVATE ${CppUnit_LIBRARIES})
  endif()

  if(mbilog_FOUND)
    target_link_libraries(${PLUGIN_TARGET} PRIVATE mbilog)
  endif()

  set(_PLUGIN_META_FILES "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml")
    list(APPEND _PLUGIN_META_FILES "${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml")
  endif()

  set(PLUGIN_TARGET ${PLUGIN_TARGET} PARENT_SCOPE)

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

      mitkFunctionInstallCTKPlugin(TARGETS ${PLUGIN_TARGET}
                                   DESTINATION ${install_subdir})

    endforeach()

    set(_autoload_targets )
    foreach(_dependency ${_module_deps})
      get_target_property(_dep_autoloads ${_dependency} MITK_AUTOLOAD_TARGETS)
      if (_dep_autoloads)
        list(APPEND _autoload_targets ${_dep_autoloads})
      endif()
    endforeach()

    # The MITK_AUTOLOAD_TARGETS property is used in the mitkFunctionInstallAutoLoadModules
    # macro which expects a list of plug-in targets.
    if (_autoload_targets)
      list(REMOVE_DUPLICATES _autoload_targets)
      set_target_properties(${PLUGIN_TARGET} PROPERTIES MITK_AUTOLOAD_TARGETS "${_autoload_targets}")
    endif()
  endif()

endfunction()


function(_FUNCTION_CREATE_CTK_QT_COMPRESSED_HELP qch_file use_xsl)

  set(_manifest_path "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  if(NOT EXISTS ${_manifest_path})
    message(FATAL_ERROR "${_manifest_path} not found")
  endif()

  include(${_manifest_path})
  string(REPLACE "_" "." Plugin-SymbolicName "${PLUGIN_TARGET}")

  configure_file(${MITK_SOURCE_DIR}/Documentation/doxygen_plugin_manual.conf.in
                 ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                 )

  set(_qhp_xsl_file "${MITK_SOURCE_DIR}/Documentation/qhp_toc.xsl")
  set(_generated_qhp_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/html/index.qhp")
  set(_transformed_qhp_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/html/${PLUGIN_TARGET}.qhp")
  set(${qch_file} "${CMAKE_CURRENT_BINARY_DIR}/resources/${PLUGIN_TARGET}.qch")

  set(_xsl_command )
  if(use_xsl)
    set(_xsl_command COMMAND ${QT_XMLPATTERNS_EXECUTABLE} ${_qhp_xsl_file} ${_generated_qhp_file} -output ${_transformed_qhp_file})
  endif()

  file(GLOB _file_dependencies "${PLUGIN_DOXYGEN_INPUT_DIR}/*")

  add_custom_command(OUTPUT ${${qch_file}}
                     # Generate a Qt help project (index.qhp) with doxygen
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                     # Use a XSL transformation to get rid of the top-level entry
                     ${_xsl_command}
                     # Generate the final Qt compressed help file (.qch)
                     COMMAND ${QT_HELPGENERATOR_EXECUTABLE} ${_transformed_qhp_file} -o ${${qch_file}}
                     DEPENDS ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf ${_file_dependencies}
                     )

  #set_source_files_properties(${qch_file} PROPERTIES GENERATED 1)

  set(${qch_file} ${${qch_file}} PARENT_SCOPE)

endfunction()

function(MACRO_CREATE_MITK_CTK_PLUGIN)
  message(SEND_ERROR "The function MACRO_CREATE_MITK_CTK_PLUGIN was renamed to mitk_create_plugin in MITK 2015.05.")
endfunction()
