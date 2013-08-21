##################################################################
#
# MITK_CREATE_MODULE
#
#! Creates a module for the automatic module dependency system within MITK.
#! Configurations are generated in the moduleConf directory.
#!
#! USAGE:
#!
#! \code
#! MITK_CREATE_MODULE( <moduleName>
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
#! \param MODULE_NAME_IN The name for the new module
#! \param HEADERS_ONLY specify this if the modules just contains header files.
##################################################################
macro(MITK_CREATE_MODULE MODULE_NAME_IN)

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
      GENERATED_CPP          # not used (?)
     )

  set(_macro_options
      QT_MODULE              # the module makes use of Qt features and needs moc and ui generated files
      FORCE_STATIC           # force building this module as a static library
      HEADERS_ONLY           # this module is a headers-only library
      GCC_DEFAULT_VISIBILITY # do not use gcc visibility flags - all symbols will be exported
      NO_INIT                # do not create CppMicroServices initialization code
      WARNINGS_AS_ERRORS     # treat all compiler warnings as errors
     )

  MACRO_PARSE_ARGUMENTS(MODULE "${_macro_params}" "${_macro_options}" ${ARGN})

  set(MODULE_NAME ${MODULE_NAME_IN})

  if(MODULE_HEADERS_ONLY)
    set(MODULE_PROVIDES )
    if(MODULE_AUTOLOAD_WITH)
      message(SEND_ERROR "A headers only module cannot be auto-loaded")
    endif()
  else()
    set(MODULE_PROVIDES ${MODULE_NAME})
    if(NOT MODULE_NO_INIT AND NOT MODULE_NAME STREQUAL "Mitk")
      # Add a dependency to the "Mitk" module
      #list(APPEND MODULE_DEPENDS Mitk)
    endif()
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

  # check and set-up auto-loading
  if(MODULE_AUTOLOAD_WITH)
    if(NOT TARGET "${MODULE_AUTOLOAD_WITH}")
      message(SEND_ERROR "The module target \"${MODULE_AUTOLOAD_WITH}\" specified as the auto-loading module for \"${MODULE_NAME}\" does not exist")
    endif()
    # create a meta-target if it does not already exist
    set(_module_autoload_meta_target "${MODULE_AUTOLOAD_WITH}-universe")
    if(NOT TARGET ${_module_autoload_meta_target})
      add_custom_target(${_module_autoload_meta_target})
    endif()
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
  if(NOT MODULE_IS_EXCLUDED AND NOT (MODULE_QT_MODULE AND NOT MITK_USE_QT))
    # first of all we check for the dependencies
    MITK_CHECK_MODULE(_MISSING_DEP ${MODULE_DEPENDS})
    if(_MISSING_DEP)
      message("Module ${MODULE_NAME} won't be built, missing dependency: ${_MISSING_DEP}")
      set(MODULE_IS_ENABLED 0)
    else(_MISSING_DEP)
      set(MODULE_IS_ENABLED 1)
      # now check for every package if it is enabled. This overlaps a bit with
      # MITK_CHECK_MODULE ...
      foreach(_package ${MODULE_PACKAGE_DEPENDS})
        if((DEFINED MITK_USE_${_package}) AND NOT (MITK_USE_${_package}))
          message("Module ${MODULE_NAME} won't be built. Turn on MITK_USE_${_package} if you want to use it.")
          set(MODULE_IS_ENABLED 0)
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
        set(Q${KITNAME}_GENERATED_MOC_CPP )
        set(Q${KITNAME}_GENERATED_QRC_CPP )
        set(Q${KITNAME}_GENERATED_UI_CPP )

        _MITK_CREATE_MODULE_CONF()
        if(NOT MODULE_EXPORT_DEFINE)
          set(MODULE_EXPORT_DEFINE ${MODULE_NAME}_EXPORT)
        endif(NOT MODULE_EXPORT_DEFINE)

        if(MITK_GENERATE_MODULE_DOT)
          message("MODULEDOTNAME ${MODULE_NAME}")
          foreach(dep ${MODULE_DEPENDS})
            message("MODULEDOT \"${MODULE_NAME}\" -> \"${dep}\" ; ")
          endforeach(dep)
        endif(MITK_GENERATE_MODULE_DOT)

        set(DEPENDS "${MODULE_DEPENDS}")
        if(NOT MODULE_NO_INIT)
          # Add a CppMicroServices dependency implicitly, since it is
          # needed for the generated "module initialization" code.
          set(DEPENDS "CppMicroServices;${DEPENDS}")
        endif()
        set(DEPENDS_BEFORE "not initialized")
        set(PACKAGE_DEPENDS "${MODULE_PACKAGE_DEPENDS}")
        MITK_USE_MODULE(${DEPENDS})

        # ok, now create the module itself
        include_directories(. ${ALL_INCLUDE_DIRECTORIES})
        include(files.cmake)

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

        if(NOT MODULE_NO_INIT AND NOT MODULE_HEADERS_ONLY)
          set(MODULE_LIBNAME ${MODULE_PROVIDES})

          usFunctionGenerateModuleInit(CPP_FILES
                                       NAME ${MODULE_NAME}
                                       LIBRARY_NAME ${MODULE_LIBNAME}
                                       DEPENDS ${MODULE_DEPENDS} ${MODULE_DEPENDS_INTERNAL} ${MODULE_PACKAGE_DEPENDS}
                                       #VERSION ${MODULE_VERSION}
                                      )

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
              list(APPEND res_macro_args ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${res_dir}
                                         FILES ${binary_res_files})
            endif()
            if(source_res_files)
              list(APPEND res_macro_args ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${res_dir}
                                         FILES ${source_res_files})
            endif()

            usFunctionEmbedResources(CPP_FILES
                                     LIBRARY_NAME ${MODULE_LIBNAME}
                                     ${res_macro_args})
          endif()

        endif()

        if(MODULE_QT_MODULE)
          # TODO: here we would need handling of both Qt 4 and Qt 5
          if(UI_FILES)
            QT4_WRAP_UI(Q${KITNAME}_GENERATED_UI_CPP ${UI_FILES})
          endif(UI_FILES)

          if(MOC_H_FILES)
            QT4_WRAP_CPP(Q${KITNAME}_GENERATED_MOC_CPP ${MOC_H_FILES} OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
          endif(MOC_H_FILES)

          if(QRC_FILES)
            QT4_ADD_RESOURCES(Q${KITNAME}_GENERATED_QRC_CPP ${QRC_FILES})
          endif(QRC_FILES)

          set(Q${KITNAME}_GENERATED_CPP ${Q${KITNAME}_GENERATED_CPP} ${Q${KITNAME}_GENERATED_UI_CPP} ${Q${KITNAME}_GENERATED_MOC_CPP} ${Q${KITNAME}_GENERATED_QRC_CPP})
        endif()

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
          if(ALL_LIBRARY_DIRS)
            # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
            link_directories(${ALL_LIBRARY_DIRS})
          endif(ALL_LIBRARY_DIRS)

          add_library(${MODULE_PROVIDES} ${_STATIC}
                      ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP}
                      ${DOX_FILES} ${UI_FILES} ${QRC_FILES})

          if(MODULE_TARGET_DEPENDS)
            add_dependencies(${MODULE_PROVIDES} ${MODULE_TARGET_DEPENDS})
          endif()

          if(MODULE_SUBPROJECTS)
            set_property(TARGET ${MODULE_PROVIDES} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
            foreach(subproject ${MODULE_SUBPROJECTS})
              add_dependencies(${subproject} ${MODULE_PROVIDES})
            endforeach()
          endif()

          if(ALL_LIBRARIES)
            target_link_libraries(${MODULE_PROVIDES} ${ALL_LIBRARIES})
          endif(ALL_LIBRARIES)

          if(MODULE_QT_MODULE AND QT_LIBRARIES)
            target_link_libraries(${MODULE_PROVIDES} ${QT_LIBRARIES})
          endif()

          if(MINGW)
            target_link_libraries(${MODULE_PROVIDES} ssp) # add stack smash protection lib
          endif()

          # Apply properties to the module target.
          # We cannot use set_target_properties like below since there is no way to
          # differentiate C/C++ and Releas/Debug flags using target properties.
          # See http://www.cmake.org/Bug/view.php?id=6493
          #set_target_properties(${MODULE_PROVIDES} PROPERTIES
          #                      COMPILE_FLAGS "${module_compile_flags}")
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${module_c_flags}")
          set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${module_c_flags_debug}")
          set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${module_c_flags_release}")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${module_cxx_flags}")
          set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${module_cxx_flags_debug}")
          set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${module_cxx_flags_release}")

          # Add additional library search directories to a global property which
          # can be evaluated by other CMake macros, e.g. our install scripts.
          if(MODULE_ADDITIONAL_LIBS)
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
          set_property(GLOBAL APPEND PROPERTY MITK_MODULE_TARGETS ${MODULE_PROVIDES})
          if(MODULE_AUTOLOAD_WITH)
            # for auto-loaded modules, adapt the output directory
            add_dependencies(${_module_autoload_meta_target} ${MODULE_PROVIDES})
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
              set_target_properties(${MODULE_PROVIDES} PROPERTIES
                                    ${_module_output_prop} ${_module_output_dir}/${MODULE_AUTOLOAD_WITH})
            else()
              set_target_properties(${MODULE_PROVIDES} PROPERTIES
                                    ${_module_output_prop} ${CMAKE_${_module_output_prop}}/${MODULE_AUTOLOAD_WITH})
            endif()
            set_target_properties(${MODULE_PROVIDES} PROPERTIES
                                  MITK_AUTOLOAD_DIRECTORY ${MODULE_AUTOLOAD_WITH})

            # add the auto-load module name as a property
            set_property(TARGET ${MODULE_AUTOLOAD_WITH} APPEND PROPERTY MITK_AUTOLOAD_TARGETS ${MODULE_PROVIDES})
          else()
            # Add meta dependencies (e.g. on auto-load modules from depending modules)
            if(ALL_META_DEPENDENCIES)
              add_dependencies(${MODULE_PROVIDES} ${ALL_META_DEPENDENCIES})
            endif()
          endif()
        endif()

      endif(MODULE_IS_ENABLED)
    endif(_MISSING_DEP)
  endif(NOT MODULE_IS_EXCLUDED AND NOT (MODULE_QT_MODULE AND NOT MITK_USE_QT))

  if(NOT MODULE_IS_ENABLED)
    _MITK_CREATE_MODULE_CONF()
  endif(NOT MODULE_IS_ENABLED)

endmacro(MITK_CREATE_MODULE)
