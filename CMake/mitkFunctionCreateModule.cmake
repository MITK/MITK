function(_link_directories_for_packages)
  set(ALL_LIBRARY_DIRS )
  foreach(package ${ARGN})
    if(NOT ${package} MATCHES "^(Qt[45].*|ITK|VTK)$")
      foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
        if(EXISTS "${dir}/MITK_${package}_Config.cmake")
          include("${dir}/MITK_${package}_Config.cmake")
          break()
        endif()
      endforeach()
    endif()
  endforeach()
  if(ALL_LIBRARY_DIRS)
    list(REMOVE_DUPLICATES ALL_LIBRARY_DIRS)
    link_directories(${ALL_LIBRARY_DIRS})
  endif()
endfunction()

##################################################################
#
# mitk_create_module
#
#! Creates a module for the automatic module dependency system within MITK.
#! Configurations are generated in the moduleConf directory.
#!
#! USAGE:
#!
#! \code
#! MITK_CREATE_MODULE([<moduleName>]
#!     [INCLUDE_DIRS <include directories>]
#!     [INTERNAL_INCLUDE_DIRS <internally used include directories>]
#!     [DEPENDS <modules we need>]
#!     [PACKAGE_DEPENDS <packages we need, like ITK, VTK, QT>]
#!     [TARGET_DEPENDS <list of additional dependencies>
#!     [EXPORT_DEFINE <declspec macro name for dll exports>]
#!     [QT_MODULE]
#!     [HEADERS_ONLY]
#!     [WARNINGS_AS_ERRORS]
#! \endcode
#!
#! The <moduleName> parameter specifies the name of the module which is used
#! create a logical target name. The parameter is options in case the
#! MITK_MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME variable evaluates to TRUE. The
#! module name will then be derived from the directory name in which this
#! macro is called.
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
#! - ALL_META_DEPENDENCIES
#!
#! \param QT_MODULE deprecated. Just use Qt4 or Qt5 in the PACKAGE_DEPENDS argument.
#! \param HEADERS_ONLY specify this if the modules just contains header files.
##################################################################
function(mitk_create_module)

  set(_macro_params
      SUBPROJECTS            # list of CDash labels
      VERSION                # module version number, e.g. "1.2.0"
      INCLUDE_DIRS           # exported include dirs (used in mitkMacroCreateModuleConf.cmake)
      INTERNAL_INCLUDE_DIRS  # include dirs internal to this module
      DEPENDS                # list of modules this module depends on
      DEPENDS_INTERNAL       # list of modules this module internally depends on
      PACKAGE_DEPENDS        # list of "packages this module depends on (e.g. Qt, VTK, etc.)
      TARGET_DEPENDS         # list of CMake targets this module should depend on
      EXPORT_DEFINE          # export macro name for public symbols of this module
      AUTOLOAD_WITH          # a module target name identifying the module which will trigger the
                             # automatic loading of this module
      ADDITIONAL_LIBS        # list of addidtional libraries linked to this module
      FILES_CMAKE            # file name of a CMake file setting source list variables
                             # (defaults to files.cmake)
      GENERATED_CPP          # not used (?)
      DEPRECATED_SINCE       # marks this modules as deprecated
      DESCRIPTION            # a description for this module
     )

  set(_macro_options
      QT_MODULE              # the module makes use of Qt4 features and needs moc and ui generated files
      FORCE_STATIC           # force building this module as a static library
      HEADERS_ONLY           # this module is a headers-only library
      GCC_DEFAULT_VISIBILITY # do not use gcc visibility flags - all symbols will be exported
      NO_INIT                # do not create CppMicroServices initialization code
      NO_FEATURE_INFO        # do not create a feature info by calling add_feature_info()
      WARNINGS_AS_ERRORS     # treat all compiler warnings as errors
      EXECUTABLE             # create an executable; do not use directly, use mitk_create_executable() instead
     )

  MACRO_PARSE_ARGUMENTS(MODULE "${_macro_params}" "${_macro_options}" ${ARGN})

  set(MODULE_NAME ${MODULE_DEFAULT_ARGS})

  if(NOT MODULE_NAME)
    if(MITK_MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME)
      get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    else()
      message(SEND_ERROR "The module name must not be empty")
    endif()
  endif()

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

  if (MODULE_QT_MODULE)
    message(WARNING "QT_MODULE keyword is deprecated (in ${_module_type} ${MODULE_NAME}). Please use PACKAGE_DEPENDS Qt4|QtCore and/or PACKAGE_DEPENDS Qt5|Core instead")
    if (NOT "${MODULE_PACKAGE_DEPENDS}" MATCHES "^.*Qt4.*$")
      list(APPEND MODULE_PACKAGE_DEPENDS Qt4|QtGui)
    endif()
  endif()

  if(MODULE_HEADERS_ONLY)
    set(MODULE_TARGET )
    if(MODULE_AUTOLOAD_WITH)
      message(SEND_ERROR "A headers only module cannot be auto-loaded")
    endif()
  else()
    set(MODULE_TARGET ${MODULE_NAME})
  endif()

  if(MODULE_DEPRECATED_SINCE)
    set(MODULE_IS_DEPRECATED 1)
  else()
    set(MODULE_IS_DEPRECATED 0)
  endif()

  if(NOT MODULE_SUBPROJECTS)
    if(MITK_DEFAULT_SUBPROJECTS)
      set(MODULE_SUBPROJECTS ${MITK_DEFAULT_SUBPROJECTS})
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
          set(_module_autoload_meta_target "${MODULE_AUTOLOAD_WITH}-autoload")
          # create a meta-target if it does not already exist
          if(NOT TARGET ${_module_autoload_meta_target})
            add_custom_target(${_module_autoload_meta_target})
          endif()
        endif()

        # Convert relative include dirs to absolute dirs
        set(_include_dirs . ${MODULE_INCLUDE_DIRS})
        set(MODULE_INCLUDE_DIRS)
        foreach(dir ${_include_dirs})
          get_filename_component(_abs_dir ${dir} ABSOLUTE)
          list(APPEND MODULE_INCLUDE_DIRS ${_abs_dir})
        endforeach()
        list(APPEND MODULE_INCLUDE_DIRS ${MITK_BINARY_DIR} ${MODULES_CONF_DIRS})

        # Convert relative internal include dirs to absolute dirs
        set(_include_dirs ${MODULE_INTERNAL_INCLUDE_DIRS})
        set(MODULE_INTERNAL_INCLUDE_DIRS)
        foreach(dir ${_include_dirs})
          get_filename_component(_abs_dir ${dir} ABSOLUTE)
          list(APPEND MODULE_INTERNAL_INCLUDE_DIRS ${_abs_dir})
        endforeach()

        # Qt generates headers in the binary tree
        list(APPEND MODULE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

        # Add the module specific include dirs
        include_directories(${MODULE_INCLUDE_DIRS} ${MODULE_INTERNAL_INCLUDE_DIRS})

        if(NOT MODULE_EXECUTABLE)
          _MITK_CREATE_MODULE_CONF()
        endif()
        if(NOT MODULE_EXPORT_DEFINE)
          set(MODULE_EXPORT_DEFINE ${MODULE_NAME}_EXPORT)
        endif(NOT MODULE_EXPORT_DEFINE)

        if(MITK_GENERATE_MODULE_DOT)
          message("MODULEDOTNAME ${MODULE_NAME}")
          foreach(dep ${MODULE_DEPENDS})
            message("MODULEDOT \"${MODULE_NAME}\" -> \"${dep}\" ; ")
          endforeach(dep)
        endif(MITK_GENERATE_MODULE_DOT)

        # ok, now create the module itself
        include(${MODULE_FILES_CMAKE})

        set(module_c_flags )
        set(module_c_flags_debug )
        set(module_c_flags_release )
        set(module_cxx_flags )
        set(module_cxx_flags_debug )
        set(module_cxx_flags_release )

        if(MODULE_GCC_DEFAULT_VISIBILITY)
          set(use_visibility_flags 0)
        else()
          # We only support hidden visibility for gcc for now. Clang 3.0 still has troubles with
          # correctly marking template declarations and explicit template instantiations as exported.
          # See http://comments.gmane.org/gmane.comp.compilers.clang.scm/50028
          # and http://llvm.org/bugs/show_bug.cgi?id=10113
          if(CMAKE_COMPILER_IS_GNUCXX)
            set(use_visibility_flags 1)
          else()
          #  set(use_visibility_flags 0)
          endif()
        endif()

        if(CMAKE_COMPILER_IS_GNUCXX)
          # MinGW does not export all symbols automatically, so no need to set flags.
          #
          # With gcc < 4.5, RTTI symbols from classes declared in third-party libraries
          # which are not "gcc visibility aware" are marked with hidden visibility in
          # DSOs which include the class declaration and which are compiled with
          # hidden visibility. This leads to dynamic_cast and exception handling problems.
          # While this problem could be worked around by sandwiching the include
          # directives for the third-party headers between "#pragma visibility push/pop"
          # statements, it is generally safer to just use default visibility with
          # gcc < 4.5.
          if(${GCC_VERSION} VERSION_LESS "4.5" OR MINGW)
            set(use_visibility_flags 0)
          endif()
        endif()

        if(use_visibility_flags)
          mitkFunctionCheckCAndCXXCompilerFlags("-fvisibility=hidden" module_c_flags module_cxx_flags)
          mitkFunctionCheckCAndCXXCompilerFlags("-fvisibility-inlines-hidden" module_c_flags module_cxx_flags)
        endif()

        configure_file(${MITK_SOURCE_DIR}/CMake/moduleExports.h.in ${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/${MODULE_NAME}Exports.h @ONLY)

        if(MODULE_WARNINGS_AS_ERRORS)
          if(MSVC_VERSION)
            mitkFunctionCheckCAndCXXCompilerFlags("/WX" module_c_flags module_cxx_flags)
          else()
            mitkFunctionCheckCAndCXXCompilerFlags("-Werror" module_c_flags module_cxx_flags)

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

            # VNL headers throw a lot of these, not fixable for us at least in ITK 3
            mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unused-parameter" module_c_flags module_cxx_flags)

            # Some DICOM header file in ITK
            mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=cast-align" module_c_flags module_cxx_flags)

          endif()
        endif(MODULE_WARNINGS_AS_ERRORS)

        if(MODULE_FORCE_STATIC)
          set(_STATIC STATIC)
        else()
          set(_STATIC )
        endif(MODULE_FORCE_STATIC)

        # create a meta-target for auto-loaded modules
        add_custom_target(${MODULE_NAME}-autoload)

        if(NOT MODULE_HEADERS_ONLY)
          find_package(CppMicroServices QUIET NO_MODULE REQUIRED)
          if(NOT MODULE_NO_INIT)
            usFunctionGenerateModuleInit(CPP_FILES)
          endif()
          if(RESOURCE_FILES)
            # Add a source level dependency on resource files
            list(APPEND CPP_FILES ${MODULE_TARGET}_resources.cpp)
          endif()
        endif()

        # Qt 4 case
        if(MITK_USE_Qt4)
          if(UI_FILES)
            qt4_wrap_ui(Q${KITNAME}_GENERATED_UI_CPP ${UI_FILES})
          endif()
          if(MOC_H_FILES)
            qt4_wrap_cpp(Q${KITNAME}_GENERATED_MOC_CPP ${MOC_H_FILES} OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
          endif()
          if(QRC_FILES)
            qt4_add_resources(Q${KITNAME}_GENERATED_QRC_CPP ${QRC_FILES})
          endif()
        endif()

        # all the same for Qt 5
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

        ORGANIZE_SOURCES(SOURCE ${CPP_FILES}
                         HEADER ${H_FILES}
                         TXX ${TXX_FILES}
                         DOC ${DOX_FILES}
                         UI ${UI_FILES}
                         QRC ${QRC_FILES}
                         MOC ${Q${KITNAME}_GENERATED_MOC_CPP}
                         GEN_QRC ${Q${KITNAME}_GENERATED_QRC_CPP}
                         GEN_UI ${Q${KITNAME}_GENERATED_UI_CPP})

        set(coverage_sources
            ${CPP_FILES} ${H_FILES} ${GLOBBED__H_FILES} ${CORRESPONDING__H_FILES} ${TXX_FILES}
            ${TOOL_CPPS} ${TOOL_GUI_CPPS})

        if(MODULE_SUBPROJECTS)
          set_property(SOURCE ${coverage_sources} APPEND PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
        endif()

        if(NOT MODULE_HEADERS_ONLY)

          # We have to include the MITK_<package>_Config.cmake files here because
          # some external packages do not provide exported targets with an
          # absolute path to link to. So we need to add link directories *before*
          # add_library() or add_executable() is called. So far, this is needed only
          # for GDCM and ACVD.

          _link_directories_for_packages(${PACKAGE_NAMES})

          # Apply properties to the module target.
          # We cannot use set_target_properties like below since there is no way to
          # differentiate C/C++ and Releas/Debug flags using target properties.
          # See http://www.cmake.org/Bug/view.php?id=6493
          #set_target_properties(${MODULE_TARGET} PROPERTIES
          #                      COMPILE_FLAGS "${module_compile_flags}")
          #
          # Strangely, we need to set the variables below in the parent scope
          # (outside of the function) to be picked up by the target.
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${module_c_flags}" PARENT_SCOPE)
          set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${module_c_flags_debug}" PARENT_SCOPE)
          set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${module_c_flags_release}" PARENT_SCOPE)
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${module_cxx_flags}" PARENT_SCOPE)
          set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${module_cxx_flags_debug}" PARENT_SCOPE)
          set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${module_cxx_flags_release}" PARENT_SCOPE)

          if(MODULE_EXECUTABLE)
            add_executable(${MODULE_TARGET}
                           ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP}
                           ${DOX_FILES} ${UI_FILES} ${QRC_FILES})
            set(_us_module_name main)
          else()
            add_library(${MODULE_TARGET} ${_STATIC}
                        ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP}
                        ${DOX_FILES} ${UI_FILES} ${QRC_FILES})
            set(_us_module_name ${MODULE_TARGET})
          endif()
          set_property(TARGET ${MODULE_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${_us_module_name})
          set_property(TARGET ${MODULE_TARGET} PROPERTY US_MODULE_NAME ${_us_module_name})

          if(MODULE_TARGET_DEPENDS)
            add_dependencies(${MODULE_TARGET} ${MODULE_TARGET_DEPENDS})
          endif()

          if(MODULE_SUBPROJECTS)
            set_property(TARGET ${MODULE_TARGET} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
            foreach(subproject ${MODULE_SUBPROJECTS})
              add_dependencies(${subproject} ${MODULE_TARGET})
            endforeach()
          endif()

          set(DEPENDS "${MODULE_DEPENDS}")
          if(NOT MODULE_NO_INIT)
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

          if(MINGW)
            target_link_libraries(${MODULE_TARGET} ssp) # add stack smash protection lib
          endif()

          # Add additional library search directories to a global property which
          # can be evaluated by other CMake macros, e.g. our install scripts.
          if(MODULE_ADDITIONAL_LIBS)
            target_link_libraries(${MODULE_TARGET} ${MODULE_ADDITIONAL_LIBS})
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

          if(RESOURCE_FILES)
            set(res_dir Resources)
            set(binary_res_files )
            set(source_res_files )
            foreach(res_file ${RESOURCE_FILES})
              if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${res_dir}/${res_file})
                list(APPEND binary_res_files "${res_file}")
              else()
                list(APPEND source_res_files "${res_file}")
              endif()
            endforeach()

            set(res_macro_args )
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
          endif()

        endif()

      endif()
    endif()
  endif()

  if(NOT MODULE_IS_ENABLED AND NOT MODULE_EXECUTABLE)
    _MITK_CREATE_MODULE_CONF()
  endif()

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
  set(ALL_META_DEPENDENCIES ${ALL_META_DEPENDENCIES} PARENT_SCOPE)

endfunction()
