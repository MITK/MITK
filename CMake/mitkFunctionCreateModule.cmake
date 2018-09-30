##################################################################
#
# mitk_create_module
#
#! Creates a module for the automatic module dependency system within MITK.
#!
#! Example:
#!
#! \code
#! mitk_create_module(
#!     DEPENDS PUBLIC MitkCore
#!     PACKAGE_DEPENDS
#!       PRIVATE Qt5|Xml+Networking
#!       PUBLIC  ITK|Watershed
#! \endcode
#!
#! The <moduleName> parameter specifies the name of the module which is used
#! to create a logical target name. The parameter is optional in case the
#! MITK_MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME variable evaluates to TRUE. The
#! module name will then be derived from the directory name in which this
#! function is called.
#!
#! If set, the following variables will be used to validate the module name:
#!
#!   MITK_MODULE_NAME_REGEX_MATCH The module name must match this regular expression.
#!   MITK_MODULE_NAME_REGEX_NOT_MATCH The module name must not match this regular expression.
#!
#! If the MITK_MODULE_NAME_PREFIX variable is set, the module name will be prefixed
#! with its contents.
#!
#! A modules source files are specified in a separate CMake file usually
#! called files.cmake, located in the module root directory. The
#! mitk_create_module() macro evaluates the following CMake variables
#! from the files.cmake file:
#!
#! - CPP_FILES A list of .cpp files
#! - H_FILES A list of .h files without a corresponding .cpp file
#! - TXX_FILES A list of .txx files
#! - RESOURCE_FILES A list of files (resources) which are embedded into the module
#! - MOC_H_FILES A list of Qt header files which should be processed by the MOC
#! - UI_FILES A list of .ui Qt UI files
#! - QRC_FILES A list of .qrc Qt resource files
#! - DOX_FILES A list of .dox Doxygen files
#!
#! List of variables available after the function is called:
#! - MODULE_NAME
#! - MODULE_TARGET
#! - MODULE_IS_ENABLED
#! - MODULE_SUBPROJECTS
#!
#! \sa mitk_create_executable
#!
#! Parameters (all optional):
#!
#! \param <moduleName> The module name (also used as target name)
#! \param FILES_CMAKE File name of a CMake file setting source list variables
#!        (defaults to files.cmake)
#! \param VERSION Module version number, e.g. "1.2.0"
#! \param AUTOLOAD_WITH A module target name identifying the module which will
#!        trigger the automatic loading of this module
#! \param DEPRECATED_SINCE Marks this modules as deprecated since <arg>
#! \param DESCRIPTION A description for this module
#!
#! Multi-value Parameters (all optional):
#!

#! \param SUBPROJECTS List of CDash labels
#! \param INCLUDE_DIRS Include directories for this module:
#!        \verbatim
#! [[PUBLIC|PRIVATE|INTERFACE] <dir1>...]...
#! \endverbatim
#!        The default scope for include directories is PUBLIC.
#! \param DEPENDS List of module dependencies:
#!        \verbatim
#! [[PUBLIC|PRIVATE|INTERFACE] <module1>...]...
#! \endverbatim
#!        The default scope for module dependencies is PUBLIC.
#! \param PACKAGE_DEPENDS List of public packages dependencies (e.g. Qt, VTK, etc.).
#!        Package dependencies have the following syntax:
#!        \verbatim
#! [PUBLIC|PRIVATE|INTERFACE] PACKAGE[|COMPONENT1[+COMPONENT2]...]
#! \endverbatim
#!        The default scope for package dependencies is PRIVATE.
#! \param ADDITIONAL_LIBS List of additional private libraries linked to this module.
#!        The folder containing the library will be added to the global list of library search paths.
#! \param CPP_FILES List of source files for this module. If the list is non-empty,
#!        the module does not need to provide a files.cmake file or FILES_CMAKE argument.
#! \param H_FILES List of public header files for this module. It is recommended to use
#!        a files.cmake file instead.
#!
#! Options (optional)
#!
#! \param FORCE_STATIC Force building this module as a static library
#! \param GCC_DEFAULT_VISIBILITY Do not use gcc visibility flags - all
#!        symbols will be exported
#! \param NO_INIT Do not create CppMicroServices initialization code
#! \param NO_FEATURE_INFO Do not create a feature info by calling add_feature_info()
#! \param WARNINGS_NO_ERRORS Do not treat compiler warnings as errors
#
##################################################################
function(mitk_create_module)

  set(_macro_params
      VERSION                # module version number, e.g. "1.2.0"
      EXPORT_DEFINE          # export macro name for public symbols of this module (DEPRECATED)
      AUTOLOAD_WITH          # a module target name identifying the module which will trigger the
                             # automatic loading of this module
      FILES_CMAKE            # file name of a CMake file setting source list variables
                             # (defaults to files.cmake)
      DEPRECATED_SINCE       # marks this modules as deprecated
      DESCRIPTION            # a description for this module
     )

  set(_macro_multiparams
      SUBPROJECTS            # list of CDash labels
      INCLUDE_DIRS           # include directories: [PUBLIC|PRIVATE|INTERFACE] <list>
      INTERNAL_INCLUDE_DIRS  # include dirs internal to this module (DEPRECATED)
      DEPENDS                # list of modules this module depends on: [PUBLIC|PRIVATE|INTERFACE] <list>
      DEPENDS_INTERNAL       # list of modules this module internally depends on (DEPRECATED)
      PACKAGE_DEPENDS        # list of "packages this module depends on (e.g. Qt, VTK, etc.): [PUBLIC|PRIVATE|INTERFACE] <package-list>
      TARGET_DEPENDS         # list of CMake targets this module should depend on
      ADDITIONAL_LIBS        # list of addidtional private libraries linked to this module.
      CPP_FILES              # list of cpp files
      H_FILES                # list of header files: [PUBLIC|PRIVATE] <list>
     )

  set(_macro_options
      FORCE_STATIC           # force building this module as a static library
      HEADERS_ONLY           # this module is a headers-only library
      GCC_DEFAULT_VISIBILITY # do not use gcc visibility flags - all symbols will be exported
      NO_DEFAULT_INCLUDE_DIRS # do not add default include directories like "include" or "."
      NO_INIT                # do not create CppMicroServices initialization code
      NO_FEATURE_INFO        # do not create a feature info by calling add_feature_info()
      WARNINGS_NO_ERRORS     # do not treat compiler warnings as errors
      EXECUTABLE             # create an executable; do not use directly, use mitk_create_executable() instead
      C_MODULE               # compile all source files as C sources
      CXX_MODULE             # compile all source files as C++ sources
     )

  cmake_parse_arguments(MODULE "${_macro_options}" "${_macro_params}" "${_macro_multiparams}" ${ARGN})

  set(MODULE_NAME ${MODULE_UNPARSED_ARGUMENTS})

  # -----------------------------------------------------------------
  # Sanity checks

  if(NOT MODULE_NAME)
    if(MITK_MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME)
      get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    else()
      message(SEND_ERROR "The module name must not be empty")
    endif()
  endif()

  set(_deprecated_args INTERNAL_INCLUDE_DIRS DEPENDS_INTERNAL EXPORT_DEFINE TARGET_DEPENDS HEADERS_ONLY)
  foreach(_deprecated_arg ${_deprecated_args})
    if(MODULE_${_deprecated_arg})
      message(WARNING "The ${_deprecated_arg} argument is deprecated")
    endif()
  endforeach()

  set(_module_type module)
  set(_Module_type Module)
  if(MODULE_EXECUTABLE)
    set(_module_type executable)
    set(_Module_type Executable)
  endif()

  if(MITK_MODULE_NAME_REGEX_MATCH)
    if(NOT ${MODULE_NAME} MATCHES ${MITK_MODULE_NAME_REGEX_MATCH})
      message(SEND_ERROR "The ${_module_type} name \"${MODULE_NAME}\" does not match the regular expression \"${MITK_MODULE_NAME_REGEX_MATCH}\".")
    endif()
  endif()
  if(MITK_MODULE_NAME_REGEX_NOT_MATCH)
    if(${MODULE_NAME} MATCHES ${MITK_MODULE_NAME_REGEX_NOT_MATCH})
      message(SEND_ERROR "The ${_module_type} name \"${MODULE_NAME}\" must not match the regular expression \"${MITK_MODULE_NAME_REGEX_NOT_MATCH}\".")
    endif()
  endif()

  if(MITK_MODULE_NAME_PREFIX AND NOT MODULE_NAME MATCHES "^${MITK_MODULE_NAME_PREFIX}.*$")
    set(MODULE_NAME "${MITK_MODULE_NAME_PREFIX}${MODULE_NAME}")
  endif()

  if(NOT MODULE_FILES_CMAKE)
    set(MODULE_FILES_CMAKE files.cmake)
  endif()
  if(NOT IS_ABSOLUTE ${MODULE_FILES_CMAKE})
    set(MODULE_FILES_CMAKE ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_FILES_CMAKE})
  endif()

  if(NOT MODULE_SUBPROJECTS)
    if(MITK_DEFAULT_SUBPROJECTS)
      set(MODULE_SUBPROJECTS ${MITK_DEFAULT_SUBPROJECTS})
    elseif(TARGET MITK-Modules)
      set(MODULE_SUBPROJECTS MITK-Modules)
    endif()
  endif()

  # check if the subprojects exist as targets
  if(MODULE_SUBPROJECTS)
    foreach(subproject ${MODULE_SUBPROJECTS})
      if(NOT TARGET ${subproject})
        message(SEND_ERROR "The subproject ${subproject} does not have a corresponding target")
      endif()
    endforeach()
  endif()

  # -----------------------------------------------------------------
  # Check if module should be build

  set(MODULE_TARGET ${MODULE_NAME})

  # assume worst case
  set(MODULE_IS_ENABLED 0)
  # first we check if we have an explicit module build list
  if(MITK_MODULES_TO_BUILD)
    list(FIND MITK_MODULES_TO_BUILD ${MODULE_NAME} _MOD_INDEX)
    if(_MOD_INDEX EQUAL -1)
      set(MODULE_IS_EXCLUDED 1)
    endif()
  endif()

  if(NOT MODULE_IS_EXCLUDED)
    # first of all we check for the dependencies
    _mitk_parse_package_args(${MODULE_PACKAGE_DEPENDS})
    mitk_check_module_dependencies(MODULES ${MODULE_DEPENDS}
                                   PACKAGES ${PACKAGE_NAMES}
                                   MISSING_DEPENDENCIES_VAR _MISSING_DEP
                                   PACKAGE_DEPENDENCIES_VAR PACKAGE_NAMES)

    if(_MISSING_DEP)
      if(MODULE_NO_FEATURE_INFO)
        message("${_Module_type} ${MODULE_NAME} won't be built, missing dependency: ${_MISSING_DEP}")
      endif()
      set(MODULE_IS_ENABLED 0)
    else()
      set(MODULE_IS_ENABLED 1)
      # now check for every package if it is enabled. This overlaps a bit with
      # MITK_CHECK_MODULE ...
      foreach(_package ${PACKAGE_NAMES})
        if((DEFINED MITK_USE_${_package}) AND NOT (MITK_USE_${_package}))
          message("${_Module_type} ${MODULE_NAME} won't be built. Turn on MITK_USE_${_package} if you want to use it.")
          set(MODULE_IS_ENABLED 0)
          break()
        endif()
      endforeach()
    endif()
  endif()

  # -----------------------------------------------------------------
  # Start creating the module

  if(MODULE_IS_ENABLED)

    # clear variables defined in files.cmake
    set(RESOURCE_FILES )
    set(CPP_FILES )
    set(H_FILES )
    set(TXX_FILES )
    set(DOX_FILES )
    set(UI_FILES )
    set(MOC_H_FILES )
    set(QRC_FILES )

    # clear other variables
    set(Q${KITNAME}_GENERATED_CPP )
    set(Q${KITNAME}_GENERATED_MOC_CPP )
    set(Q${KITNAME}_GENERATED_QRC_CPP )
    set(Q${KITNAME}_GENERATED_UI_CPP )

    # check and set-up auto-loading
    if(MODULE_AUTOLOAD_WITH)
      if(NOT TARGET "${MODULE_AUTOLOAD_WITH}")
        message(SEND_ERROR "The module target \"${MODULE_AUTOLOAD_WITH}\" specified as the auto-loading module for \"${MODULE_NAME}\" does not exist")
      endif()
    endif()
    set(_module_autoload_meta_target "${CMAKE_PROJECT_NAME}-autoload")
    # create a meta-target if it does not already exist
    if(NOT TARGET ${_module_autoload_meta_target})
      add_custom_target(${_module_autoload_meta_target})
    endif()

    if(NOT MODULE_EXPORT_DEFINE)
      set(MODULE_EXPORT_DEFINE ${MODULE_NAME}_EXPORT)
    endif()

    if(MITK_GENERATE_MODULE_DOT)
      message("MODULEDOTNAME ${MODULE_NAME}")
      foreach(dep ${MODULE_DEPENDS})
        message("MODULEDOT \"${MODULE_NAME}\" -> \"${dep}\" ; ")
      endforeach(dep)
    endif(MITK_GENERATE_MODULE_DOT)

    if (EXISTS ${MODULE_FILES_CMAKE})
      include(${MODULE_FILES_CMAKE})
    endif()

    if(MODULE_CPP_FILES)
      list(APPEND CPP_FILES ${MODULE_CPP_FILES})
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src")
      # Preprend the "src" directory to the cpp file list
      set(_cpp_files ${CPP_FILES})
      set(CPP_FILES )
      foreach(_cpp_file ${_cpp_files})
        list(APPEND CPP_FILES "src/${_cpp_file}")
      endforeach()
    endif()

    if(CPP_FILES OR RESOURCE_FILES OR UI_FILES OR MOC_H_FILES OR QRC_FILES)
      set(MODULE_HEADERS_ONLY 0)
      if(MODULE_C_MODULE)
        set_source_files_properties(${CPP_FILES} PROPERTIES LANGUAGE C)
      elseif(MODULE_CXX_MODULE)
        set_source_files_properties(${CPP_FILES} PROPERTIES LANGUAGE CXX)
      endif()
    else()
      set(MODULE_HEADERS_ONLY 1)
      if(MODULE_AUTOLOAD_WITH)
        message(SEND_ERROR "A headers only module cannot be auto-loaded")
      endif()
    endif()

    set(module_c_flags )
    set(module_c_flags_debug )
    set(module_c_flags_release )
    set(module_cxx_flags )
    set(module_cxx_flags_debug )
    set(module_cxx_flags_release )

    if(MODULE_GCC_DEFAULT_VISIBILITY OR NOT CMAKE_COMPILER_IS_GNUCXX)
      # We only support hidden visibility for gcc for now. Clang still has troubles with
      # correctly marking template declarations and explicit template instantiations as exported.
      # See http://comments.gmane.org/gmane.comp.compilers.clang.scm/50028
      # and http://llvm.org/bugs/show_bug.cgi?id=10113
      set(CMAKE_CXX_VISIBILITY_PRESET default)
      set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)
    else()
      set(CMAKE_CXX_VISIBILITY_PRESET hidden)
      set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    endif()

    if(NOT MODULE_WARNINGS_NO_ERRORS)
      if(MSVC_VERSION)
        mitkFunctionCheckCAndCXXCompilerFlags("/WX" module_c_flags module_cxx_flags)
	# this would turn on unused parameter warnings, but unfortunately MSVC cannot
	# distinguish yet between internal and external headers so this would be triggered
	# a lot by external code. There is support for it on the way so this line could be
	# reactivated after https://gitlab.kitware.com/cmake/cmake/issues/17904 has been fixed.
        # mitkFunctionCheckCAndCXXCompilerFlags("/w34100" module_c_flags module_cxx_flags)
      else()
        mitkFunctionCheckCAndCXXCompilerFlags(-Werror module_c_flags module_cxx_flags)

        # The flag "c++0x-static-nonintegral-init" has been renamed in newer Clang
        # versions to "static-member-init", see
        # http://clang-developers.42468.n3.nabble.com/Wc-0x-static-nonintegral-init-gone-td3999651.html
        #
        # Also, older Clang and seemingly all gcc versions do not warn if unknown
        # "-no-*" flags are used, so CMake will happily append any -Wno-* flag to the
        # command line. This may get confusing if unrelated compiler errors happen and
        # the error output then additionally contains errors about unknown flags (which
        # is not the case if there were no compile errors).
        #
        # So instead of using -Wno-* we use -Wno-error=*, which will be properly rejected by
        # the compiler and if applicable, prints the specific warning as a real warning and
        # not as an error (although -Werror was given).

        mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=c++0x-static-nonintegral-init" module_c_flags module_cxx_flags)
        mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=static-member-init" module_c_flags module_cxx_flags)
        mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unknown-warning" module_c_flags module_cxx_flags)
        mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=gnu" module_c_flags module_cxx_flags)
        mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=class-memaccess" module_c_flags module_cxx_flags)
        mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=inconsistent-missing-override" module_c_flags module_cxx_flags)
      endif()
    endif()

    if(MODULE_FORCE_STATIC)
      set(_STATIC STATIC)
    else()
      set(_STATIC )
    endif(MODULE_FORCE_STATIC)

    if(NOT MODULE_HEADERS_ONLY)
      if(NOT MODULE_NO_INIT OR RESOURCE_FILES)
        find_package(CppMicroServices QUIET NO_MODULE REQUIRED)
      endif()
      if(NOT MODULE_NO_INIT)
        usFunctionGenerateModuleInit(CPP_FILES)
      endif()

      set(binary_res_files )
      set(source_res_files )
      if(RESOURCE_FILES)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resource")
          set(res_dir resource)
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
          set(res_dir Resources)
        else()
          message(SEND_ERROR "Resources specified but ${CMAKE_CURRENT_SOURCE_DIR}/resource directory not found.")
        endif()
        foreach(res_file ${RESOURCE_FILES})
          if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${res_dir}/${res_file})
            list(APPEND binary_res_files "${res_file}")
          else()
            list(APPEND source_res_files "${res_file}")
          endif()
        endforeach()

        # Add a source level dependencies on resource files
        usFunctionGetResourceSource(TARGET ${MODULE_TARGET} OUT CPP_FILES)
      endif()
    endif()

    if(MITK_USE_Qt5)
      if(UI_FILES)
        qt5_wrap_ui(Q${KITNAME}_GENERATED_UI_CPP ${UI_FILES})
      endif()
      if(MOC_H_FILES)
        qt5_wrap_cpp(Q${KITNAME}_GENERATED_MOC_CPP ${MOC_H_FILES} OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
      endif()
      if(QRC_FILES)
        qt5_add_resources(Q${KITNAME}_GENERATED_QRC_CPP ${QRC_FILES})
      endif()
    endif()

    set(Q${KITNAME}_GENERATED_CPP ${Q${KITNAME}_GENERATED_CPP} ${Q${KITNAME}_GENERATED_UI_CPP} ${Q${KITNAME}_GENERATED_MOC_CPP} ${Q${KITNAME}_GENERATED_QRC_CPP})

    mitkFunctionOrganizeSources(
      SOURCE ${CPP_FILES}
      HEADER ${H_FILES}
      TXX ${TXX_FILES}
      DOC ${DOX_FILES}
      UI ${UI_FILES}
      QRC ${QRC_FILES}
      MOC ${Q${KITNAME}_GENERATED_MOC_CPP}
      GEN_QRC ${Q${KITNAME}_GENERATED_QRC_CPP}
      GEN_UI ${Q${KITNAME}_GENERATED_UI_CPP}
      )

    set(coverage_sources
        ${CPP_FILES} ${H_FILES} ${GLOBBED__H_FILES} ${CORRESPONDING__H_FILES} ${TXX_FILES}
        ${TOOL_CPPS} ${TOOL_GUI_CPPS})

    if(MODULE_SUBPROJECTS)
      set_property(SOURCE ${coverage_sources} APPEND PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
    endif()

    # ---------------------------------------------------------------
    # Create the actual module target

    if(MODULE_HEADERS_ONLY)
      add_library(${MODULE_TARGET} INTERFACE)
    else()
      if(MODULE_EXECUTABLE)
        add_executable(${MODULE_TARGET}
                       ${MODULE_CPP_FILES} ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP}
                       ${DOX_FILES} ${UI_FILES} ${QRC_FILES})
        set(_us_module_name main)
      else()
        add_library(${MODULE_TARGET} ${_STATIC}
                    ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP}
                    ${DOX_FILES} ${UI_FILES} ${QRC_FILES})
        set(_us_module_name ${MODULE_TARGET})
      endif()

      # Apply properties to the module target.
      target_compile_definitions(${MODULE_TARGET} PRIVATE US_MODULE_NAME=${_us_module_name})
      if(MODULE_C_MODULE)
        if(module_c_flags)
          string(REPLACE " " ";" module_c_flags "${module_c_flags}")
          target_compile_options(${MODULE_TARGET} PRIVATE ${module_c_flags})
        endif()
        if(module_c_flags_debug)
          string(REPLACE " " ";" module_c_flags_debug "${module_c_flags_debug}")
          target_compile_options(${MODULE_TARGET} PRIVATE $<$<CONFIG:Debug>:${module_c_flags_debug}>)
        endif()
        if(module_c_flags_release)
          string(REPLACE " " ";" module_c_flags_release "${module_c_flags_release}")
          target_compile_options(${MODULE_TARGET} PRIVATE $<$<CONFIG:Release>:${module_c_flags_release}>)
        endif()
      else()
        if(module_cxx_flags)
          string(REPLACE " " ";" module_cxx_flags "${module_cxx_flags}")
          target_compile_options(${MODULE_TARGET} PRIVATE ${module_cxx_flags})
        endif()
        if(module_cxx_flags_debug)
          string(REPLACE " " ";" module_cxx_flags_debug "${module_cxx_flags_debug}")
          target_compile_options(${MODULE_TARGET} PRIVATE $<$<CONFIG:Debug>:${module_cxx_flags_debug}>)
        endif()
        if(module_cxx_flags_release)
          string(REPLACE " " ";" module_cxx_flags_release "${module_cxx_flags_release}")
          target_compile_options(${MODULE_TARGET} PRIVATE $<$<CONFIG:Release>:${module_cxx_flags_release}>)
        endif()
      endif()

      set_property(TARGET ${MODULE_TARGET} PROPERTY US_MODULE_NAME ${_us_module_name})

      # Add additional library search directories to a global property which
      # can be evaluated by other CMake macros, e.g. our install scripts.
      if(MODULE_ADDITIONAL_LIBS)
        target_link_libraries(${MODULE_TARGET} PRIVATE ${MODULE_ADDITIONAL_LIBS})
        get_property(_mitk_additional_library_search_paths GLOBAL PROPERTY MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS)
        foreach(_lib_filepath ${MODULE_ADDITIONAL_LIBS})
          get_filename_component(_search_path "${_lib_filepath}" PATH)
          if(_search_path)
            list(APPEND _mitk_additional_library_search_paths "${_search_path}")
          endif()
        endforeach()
        if(_mitk_additional_library_search_paths)
          list(REMOVE_DUPLICATES _mitk_additional_library_search_paths)
          set_property(GLOBAL PROPERTY MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS ${_mitk_additional_library_search_paths})
        endif()
      endif()

      # add the target name to a global property which is used in the top-level
      # CMakeLists.txt file to export the target
      set_property(GLOBAL APPEND PROPERTY MITK_MODULE_TARGETS ${MODULE_TARGET})
      if(MODULE_AUTOLOAD_WITH)
        # for auto-loaded modules, adapt the output directory
        add_dependencies(${_module_autoload_meta_target} ${MODULE_TARGET})
        if(WIN32)
          set(_module_output_prop RUNTIME_OUTPUT_DIRECTORY)
        else()
          set(_module_output_prop LIBRARY_OUTPUT_DIRECTORY)
        endif()
        set(_module_output_dir ${CMAKE_${_module_output_prop}}/${MODULE_AUTOLOAD_WITH})
        get_target_property(_module_is_imported ${MODULE_AUTOLOAD_WITH} IMPORTED)
        if(NOT _module_is_imported)
          # if the auto-loading module is not imported, get its location
          # and put the auto-load module relative to it.
          get_target_property(_module_output_dir ${MODULE_AUTOLOAD_WITH} ${_module_output_prop})
          set_target_properties(${MODULE_TARGET} PROPERTIES
                                ${_module_output_prop} ${_module_output_dir}/${MODULE_AUTOLOAD_WITH})
        else()
          set_target_properties(${MODULE_TARGET} PROPERTIES
                                ${_module_output_prop} ${CMAKE_${_module_output_prop}}/${MODULE_AUTOLOAD_WITH})
        endif()
        set_target_properties(${MODULE_TARGET} PROPERTIES
                              MITK_AUTOLOAD_DIRECTORY ${MODULE_AUTOLOAD_WITH})

        # add the auto-load module name as a property
        set_property(TARGET ${MODULE_AUTOLOAD_WITH} APPEND PROPERTY MITK_AUTOLOAD_TARGETS ${MODULE_TARGET})
      endif()

      if(binary_res_files)
        usFunctionAddResources(TARGET ${MODULE_TARGET}
                               WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${res_dir}
                               FILES ${binary_res_files})
      endif()
      if(source_res_files)
        usFunctionAddResources(TARGET ${MODULE_TARGET}
                               WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${res_dir}
                               FILES ${source_res_files})
      endif()
      if(binary_res_files OR source_res_files)
        usFunctionEmbedResources(TARGET ${MODULE_TARGET})
      endif()

      if(MODULE_DEPRECATED_SINCE)
        set_property(TARGET ${MODULE_TARGET} PROPERTY MITK_MODULE_DEPRECATED_SINCE ${MODULE_DEPRECATED_SINCE})
      endif()

      # create export macros
      if (NOT MODULE_EXECUTABLE)
        set(_export_macro_name )
        if(MITK_LEGACY_EXPORT_MACRO_NAME)
          set(_export_macro_names
            EXPORT_MACRO_NAME ${MODULE_EXPORT_DEFINE}
            NO_EXPORT_MACRO_NAME ${MODULE_NAME}_NO_EXPORT
            DEPRECATED_MACRO_NAME ${MODULE_NAME}_DEPRECATED
            NO_DEPRECATED_MACRO_NAME ${MODULE_NAME}_NO_DEPRECATED
          )
        endif()
        generate_export_header(${MODULE_NAME}
          ${_export_macro_names}
          EXPORT_FILE_NAME ${MODULE_NAME}Exports.h
        )
      endif()

      target_include_directories(${MODULE_TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    endif()

    # ---------------------------------------------------------------
    # Properties for both header-only and compiled modules

    if(MODULE_HEADERS_ONLY)
      set(_module_property_type INTERFACE)
    else()
      set(_module_property_type PUBLIC)
    endif()

    if(MODULE_TARGET_DEPENDS)
      add_dependencies(${MODULE_TARGET} ${MODULE_TARGET_DEPENDS})
    endif()

    if(MODULE_SUBPROJECTS AND NOT MODULE_HEADERS_ONLY)
      set_property(TARGET ${MODULE_TARGET} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
      foreach(subproject ${MODULE_SUBPROJECTS})
        add_dependencies(${subproject} ${MODULE_TARGET})
      endforeach()
    endif()

    set(DEPENDS "${MODULE_DEPENDS}")
    if(NOT MODULE_NO_INIT AND NOT MODULE_HEADERS_ONLY)
      # Add a CppMicroServices dependency implicitly, since it is
      # needed for the generated "module initialization" code.
      set(DEPENDS "CppMicroServices;${DEPENDS}")
    endif()
    if(DEPENDS OR MODULE_PACKAGE_DEPENDS)
      mitk_use_modules(TARGET ${MODULE_TARGET}
                       MODULES ${DEPENDS}
                       PACKAGES ${MODULE_PACKAGE_DEPENDS}
                      )
    endif()

    if(NOT MODULE_C_MODULE)
      target_compile_features(${MODULE_TARGET} ${_module_property_type} ${MITK_CXX_FEATURES})
    endif()

    # add include directories
    if(MODULE_INTERNAL_INCLUDE_DIRS)
      target_include_directories(${MODULE_TARGET} PRIVATE ${MODULE_INTERNAL_INCLUDE_DIRS})
    endif()
    if(NOT MODULE_NO_DEFAULT_INCLUDE_DIRS)
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_include_directories(${MODULE_TARGET} ${_module_property_type} include)
      else()
        target_include_directories(${MODULE_TARGET} ${_module_property_type} .)
      endif()
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src)
        target_include_directories(${MODULE_TARGET} PRIVATE src)
      endif()
    endif()
    target_include_directories(${MODULE_TARGET} ${_module_property_type} ${MODULE_INCLUDE_DIRS})

  endif()

  # -----------------------------------------------------------------
  # Record missing dependency information

  if(_MISSING_DEP)
    if(MODULE_DESCRIPTION)
      set(MODULE_DESCRIPTION "${MODULE_DESCRIPTION} (missing dependencies: ${_MISSING_DEP})")
    else()
      set(MODULE_DESCRIPTION "(missing dependencies: ${_MISSING_DEP})")
    endif()
  endif()
  if(NOT MODULE_NO_FEATURE_INFO)
    add_feature_info(${MODULE_NAME} MODULE_IS_ENABLED "${MODULE_DESCRIPTION}")
  endif()

  set(MODULE_NAME ${MODULE_NAME} PARENT_SCOPE)
  set(MODULE_TARGET ${MODULE_TARGET} PARENT_SCOPE)
  set(MODULE_IS_ENABLED ${MODULE_IS_ENABLED} PARENT_SCOPE)
  set(MODULE_SUBPROJECTS ${MODULE_SUBPROJECTS} PARENT_SCOPE)

endfunction()
