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
      list(APPEND MODULE_DEPENDS Mitk)
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
  if(NOT MODULE_IS_EXCLUDED)
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

        if(NOT MODULE_QT_MODULE OR MITK_USE_QT)
          set(MODULE_IS_ENABLED 1)
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

          if(NOT MODULE_NO_INIT)
            set(MODULE_LIBNAME ${MODULE_PROVIDES})

            set(module_init_src_file)
            usFunctionGenerateModuleInit(module_init_src_file
                                         NAME ${MODULE_NAME}
                                         LIBRARY_NAME ${MODULE_LIBNAME}
                                         DEPENDS ${MODULE_DEPENDS} ${MODULE_DEPENDS_INTERNAL} ${MODULE_PACKAGE_DEPENDS}
                                         #VERSION ${MODULE_VERSION}
                                        )
          endif()

          set(DEPENDS "${MODULE_DEPENDS}")
          set(DEPENDS_BEFORE "not initialized")
          set(PACKAGE_DEPENDS "${MODULE_PACKAGE_DEPENDS}")
          MITK_USE_MODULE("${MODULE_DEPENDS}")

          # ok, now create the module itself
          include_directories(. ${ALL_INCLUDE_DIRECTORIES})
          include(files.cmake)

          set(module_compile_flags )
          if(WIN32)
            set(module_compile_flags "${module_compile_flags} -DPOCO_NO_UNWINDOWS -DWIN32_LEAN_AND_MEAN")
          endif()

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
            mitkFunctionCheckCompilerFlags("-fvisibility=hidden" module_compile_flags)
            mitkFunctionCheckCompilerFlags("-fvisibility-inlines-hidden" module_compile_flags)
          endif()

          configure_file(${MITK_SOURCE_DIR}/CMake/moduleExports.h.in ${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/${MODULE_NAME}Exports.h @ONLY)

          if(MODULE_WARNINGS_AS_ERRORS)
            if(MSVC_VERSION)
              mitkFunctionCheckCompilerFlags("/WX" module_compile_flags)
            else()
              mitkFunctionCheckCompilerFlags("-Werror" module_compile_flags)

              # The flag "c++0x-static-nonintegral-init" has been renamed in newer Clang
              # versions to "static-member-init", see
              # http://clang-developers.42468.n3.nabble.com/Wc-0x-static-nonintegral-init-gone-td3999651.html
              #
              # Also, older Clang and seemingly all gcc versions do not warn if unknown
              # "-no-*" flags are used, so CMake will happily append any -Wno-* flag to the
              # command line. This may get confusing if unrelated compiler errors happen and
              # the error output then additinally contains errors about unknown flags (which
              # is not the case if there were no compile errors).
              #
              # So instead of using -Wno-* we use -Wno-error=*, which will be properly rejected by
              # the compiler and if applicable, prints the specific warning as a real warning and
              # not as an error (although -Werror was given).

              mitkFunctionCheckCompilerFlags("-Wno-error=c++0x-static-nonintegral-init" module_compile_flags)
              mitkFunctionCheckCompilerFlags("-Wno-error=gnu" module_compile_flags)
            endif()
          endif(MODULE_WARNINGS_AS_ERRORS)

          if(NOT MODULE_NO_INIT)
            list(APPEND CPP_FILES ${module_init_src_file})
          endif()

          if(MODULE_FORCE_STATIC)
            set(_STATIC STATIC)
          else()
            set(_STATIC )
          endif(MODULE_FORCE_STATIC)

          if(NOT MODULE_QT_MODULE)
            ORGANIZE_SOURCES(SOURCE ${CPP_FILES}
                             HEADER ${H_FILES}
                             TXX ${TXX_FILES}
                             DOC ${DOX_FILES}
                            )

            set(coverage_sources ${CPP_FILES} ${H_FILES} ${GLOBBED__H_FILES} ${CORRESPONDING__H_FILES} ${TXX_FILES} ${TOOL_CPPS})
            if(MODULE_SUBPROJECTS)
              set_property(SOURCE ${coverage_sources} APPEND PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
            endif()

            if(NOT MODULE_HEADERS_ONLY)
              if(ALL_LIBRARY_DIRS)
              # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
                link_directories(${ALL_LIBRARY_DIRS})
              endif(ALL_LIBRARY_DIRS)
              add_library(${MODULE_PROVIDES} ${_STATIC} ${coverage_sources} ${CPP_FILES_GENERATED} ${DOX_FILES} ${UI_FILES})
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

              if(MINGW)
                target_link_libraries(${MODULE_PROVIDES} ssp) # add stack smash protection lib
              endif()
            endif()

          else(NOT MODULE_QT_MODULE)

            include(files.cmake)
            if(NOT MODULE_NO_INIT)
              list(APPEND CPP_FILES ${module_init_src_file})
            endif()

            if(UI_FILES)
              QT4_WRAP_UI(Q${KITNAME}_GENERATED_UI_CPP ${UI_FILES})
            endif(UI_FILES)

            if(MOC_H_FILES)
              QT4_WRAP_CPP(Q${KITNAME}_GENERATED_MOC_CPP ${MOC_H_FILES})
            endif(MOC_H_FILES)

            if(QRC_FILES)
              QT4_ADD_RESOURCES(Q${KITNAME}_GENERATED_QRC_CPP ${QRC_FILES})
            endif(QRC_FILES)

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

            # MITK_GENERATE_TOOLS_LIBRARY(Qmitk${LIBPOSTFIX} "NO")

            set(coverage_sources ${CPP_FILES} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES} ${TXX_FILES} ${TOOL_GUI_CPPS})
            set_property(SOURCE ${coverage_sources} APPEND PROPERTY LABELS ${MODULE_SUBPROJECTS})

            if(NOT MODULE_HEADERS_ONLY)
              if(ALL_LIBRARY_DIRS)
                # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
                link_directories(${ALL_LIBRARY_DIRS})
              endif(ALL_LIBRARY_DIRS)
              add_library(${MODULE_PROVIDES} ${_STATIC} ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP} ${DOX_FILES} ${UI_FILES} ${QRC_FILES})
              target_link_libraries(${MODULE_PROVIDES} ${QT_LIBRARIES} ${ALL_LIBRARIES})
              if(MODULE_TARGET_DEPENDS)
                add_dependencies(${MODULE_PROVIDES} ${MODULE_TARGET_DEPENDS})
              endif()
              if(MINGW)
                target_link_libraries(${MODULE_PROVIDES} ssp) # add stack smash protection lib
              endif()
              if(MODULE_SUBPROJECTS)
                set_property(TARGET ${MODULE_PROVIDES} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
                foreach(subproject ${MODULE_SUBPROJECTS})
                  add_dependencies(${subproject} ${MODULE_PROVIDES})
                endforeach()
              endif()
            endif()

          endif(NOT MODULE_QT_MODULE)

          if(NOT MODULE_HEADERS_ONLY)
            # Apply properties to the module target.
            set_target_properties(${MODULE_PROVIDES} PROPERTIES
              COMPILE_FLAGS "${module_compile_flags}"
            )
          endif()

          # install only if shared lib (for now)
          if(NOT _STATIC OR MINGW)
            if(NOT MODULE_HEADERS_ONLY)
              # # deprecated: MITK_INSTALL_TARGETS(${MODULE_PROVIDES})
            endif()
          endif(NOT _STATIC OR MINGW)

        endif(NOT MODULE_QT_MODULE OR MITK_USE_QT)

        if(NOT MODULE_HEADERS_ONLY)
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
  endif(NOT MODULE_IS_EXCLUDED)

  if(NOT MODULE_IS_ENABLED)
    _MITK_CREATE_MODULE_CONF()
  endif(NOT MODULE_IS_ENABLED)

endmacro(MITK_CREATE_MODULE)
