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
#! \endcode
#!
#! \param MODULE_NAME_IN The name for the new module
#! \param HEADERS_ONLY specify this if the modules just contains header files.
##################################################################
MACRO(MITK_CREATE_MODULE MODULE_NAME_IN)
  MACRO_PARSE_ARGUMENTS(MODULE
                        "SUBPROJECTS;VERSION;INCLUDE_DIRS;INTERNAL_INCLUDE_DIRS;DEPENDS;DEPENDS_INTERNAL;PACKAGE_DEPENDS;TARGET_DEPENDS;EXPORT_DEFINE;ADDITIONAL_LIBS;GENERATED_CPP"
                        "QT_MODULE;FORCE_STATIC;HEADERS_ONLY;GCC_DEFAULT_VISIBILITY;NO_INIT"
                        ${ARGN})
                        
  SET(MODULE_NAME ${MODULE_NAME_IN})

  IF(MODULE_HEADERS_ONLY)
    SET(MODULE_PROVIDES )
  ELSE()
    SET(MODULE_PROVIDES ${MODULE_NAME})
    IF(NOT MODULE_NO_INIT AND NOT MODULE_NAME STREQUAL "Mitk")
      # Add a dependency to the "Mitk" module
      LIST(APPEND MODULE_DEPENDS Mitk)
    ENDIF()
  ENDIF()

  IF(NOT MODULE_SUBPROJECTS)
    IF(MITK_DEFAULT_SUBPROJECTS)
      SET(MODULE_SUBPROJECTS ${MITK_DEFAULT_SUBPROJECTS})
    ENDIF()
  ENDIF()
  
  # check if the subprojects exist as targets
  IF(MODULE_SUBPROJECTS)
    FOREACH(subproject ${MODULE_SUBPROJECTS})
      IF(NOT TARGET ${subproject})
        MESSAGE(SEND_ERROR "The subproject ${subproject} does not have a corresponding target")
      ENDIF()
    ENDFOREACH()
  ENDIF()  

    
  
  # assume worst case
  SET(MODULE_IS_ENABLED 0)
  # first we check if we have an explicit module build list
  IF(MITK_MODULES_TO_BUILD)
    LIST(FIND MITK_MODULES_TO_BUILD ${MODULE_NAME} _MOD_INDEX)
    IF(_MOD_INDEX EQUAL -1)
      SET(MODULE_IS_EXCLUDED 1)
    ENDIF()
  ENDIF()
  IF(NOT MODULE_IS_EXCLUDED)
    # first of all we check for the dependencies
    MITK_CHECK_MODULE(_MISSING_DEP ${MODULE_DEPENDS})
    IF(_MISSING_DEP)
      MESSAGE("Module ${MODULE_NAME} won't be built, missing dependency: ${_MISSING_DEP}") 
      SET(MODULE_IS_ENABLED 0)
    ELSE(_MISSING_DEP)
      SET(MODULE_IS_ENABLED 1)
      # now check for every package if it is enabled. This overlaps a bit with
      # MITK_CHECK_MODULE ... 
      FOREACH(_package ${MODULE_PACKAGE_DEPENDS})
        IF((DEFINED MITK_USE_${_package}) AND NOT (MITK_USE_${_package}))
          MESSAGE("Module ${MODULE_NAME} won't be built. Turn on MITK_USE_${_package} if you want to use it.")
          SET(MODULE_IS_ENABLED 0)
        ENDIF()  
      ENDFOREACH()
      IF(MODULE_IS_ENABLED)

        IF(NOT MODULE_QT_MODULE OR MITK_USE_QT)
          SET(MODULE_IS_ENABLED 1)
          _MITK_CREATE_MODULE_CONF()
          IF(NOT MODULE_EXPORT_DEFINE)
            SET(MODULE_EXPORT_DEFINE ${MODULE_NAME}_EXPORT)
          ENDIF(NOT MODULE_EXPORT_DEFINE)
          CONFIGURE_FILE(${MITK_SOURCE_DIR}/CMake/moduleExports.h.in ${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/${MODULE_NAME}Exports.h @ONLY)

          IF(MITK_GENERATE_MODULE_DOT)
            MESSAGE("MODULEDOTNAME ${MODULE_NAME}")
            FOREACH(dep ${MODULE_DEPENDS})
              MESSAGE("MODULEDOT \"${MODULE_NAME}\" -> \"${dep}\" ; ")
            ENDFOREACH(dep)
          ENDIF(MITK_GENERATE_MODULE_DOT)

          IF(NOT MODULE_NO_INIT)
            # Create variables of the ModuleInfo object, created in CMake/mitkModuleInit.cpp
            SET(MODULE_LIBNAME ${MODULE_PROVIDES})
            SET(MODULE_DEPENDS_STR "")
            FOREACH(_dep ${MODULE_DEPENDS} ${MODULE_DEPENDS_INTERNAL})
              SET(MODULE_DEPENDS_STR "${MODULE_DEPENDS_STR} ${_dep}")
            ENDFOREACH()
            SET(MODULE_PACKAGE_DEPENDS_STR "")
            FOREACH(_dep ${MODULE_PACKAGE_DEPENDS})
              SET(MODULE_PACKAGE_DEPENDS_STR "${MODULE_PACKAGE_DEPENDS_STR} ${_dep}")
            ENDFOREACH()
            IF(MODULE_QT_MODULE)
              SET(MODULE_QT_BOOL "true")
            ELSE()
              SET(MODULE_QT_BOOL "false")
            ENDIF()
            SET(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}_init.cpp")
            CONFIGURE_FILE(${MITK_SOURCE_DIR}/CMake/mitkModuleInit.cpp ${module_init_src_file} @ONLY)
          ENDIF()

          SET(DEPENDS "${MODULE_DEPENDS}") 
          SET(DEPENDS_BEFORE "not initialized")
          SET(PACKAGE_DEPENDS "${MODULE_PACKAGE_DEPENDS}")
          MITK_USE_MODULE("${MODULE_DEPENDS}") 

          # ok, now create the module itself 
          INCLUDE_DIRECTORIES(. ${ALL_INCLUDE_DIRECTORIES})
          INCLUDE(files.cmake)

          SET(module_compile_flags )
          # MinGW does not export all symbols automatically, so no need to set flags
          IF(CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW AND NOT MODULE_GCC_DEFAULT_VISIBILITY)
            IF(${GCC_VERSION} VERSION_GREATER "4.5.0")
              # With gcc < 4.5, type_info objects need special care. Especially exceptions
              # and inline definitions of classes from 3rd party libraries would need
              # to be wrapped with pragma GCC visibility statements if the library is not
              # prepared to handle hidden visibility. This would need too many changes in
              # MITK and would be too fragile.
              SET(module_compile_flags "${module_compile_flags} -fvisibility=hidden -fvisibility-inlines-hidden")
            ENDIF()
          ENDIF()

          IF(NOT MODULE_NO_INIT)
            LIST(APPEND CPP_FILES ${module_init_src_file})
          ENDIF()

          IF(NOT MODULE_QT_MODULE)
            ORGANIZE_SOURCES(SOURCE ${CPP_FILES}
                             HEADER ${H_FILES}
                             TXX ${TXX_FILES}
                             DOC ${DOX_FILES}
                            )

            IF(MODULE_FORCE_STATIC)
              SET(_STATIC STATIC)
            ELSE()
              SET(_STATIC )
            ENDIF(MODULE_FORCE_STATIC)

            SET(coverage_sources ${CPP_FILES} ${H_FILES} ${GLOBBED__H_FILES} ${CORRESPONDING__H_FILES} ${TXX_FILES} ${TOOL_CPPS})
            IF(MODULE_SUBPROJECTS)
              SET_PROPERTY(SOURCE ${coverage_sources} APPEND PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
            ENDIF()

            IF(NOT MODULE_HEADERS_ONLY)
              IF(ALL_LIBRARY_DIRS)
              # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
                LINK_DIRECTORIES(${ALL_LIBRARY_DIRS})
              ENDIF(ALL_LIBRARY_DIRS)
              ADD_LIBRARY(${MODULE_PROVIDES} ${_STATIC} ${coverage_sources} ${CPP_FILES_GENERATED} ${DOX_FILES} ${UI_FILES})
              IF(MODULE_TARGET_DEPENDS)
                ADD_DEPENDENCIES(${MODULE_PROVIDES} ${MODULE_TARGET_DEPENDS})
              ENDIF()
              IF(MODULE_SUBPROJECTS)
                SET_PROPERTY(TARGET ${MODULE_PROVIDES} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
                FOREACH(subproject ${MODULE_SUBPROJECTS})
                  ADD_DEPENDENCIES(${subproject} ${MODULE_PROVIDES})
                ENDFOREACH()
              ENDIF()
              IF(ALL_LIBRARIES)
                TARGET_LINK_LIBRARIES(${MODULE_PROVIDES} ${ALL_LIBRARIES})
              ENDIF(ALL_LIBRARIES)

              IF(MINGW)
                TARGET_LINK_LIBRARIES(${MODULE_PROVIDES} ssp) # add stack smash protection lib
              ENDIF()
            ENDIF()

          ELSE(NOT MODULE_QT_MODULE)

            INCLUDE(files.cmake)
            IF(NOT MODULE_NO_INIT)
              LIST(APPEND CPP_FILES ${module_init_src_file})
            ENDIF()
      
            IF(UI_FILES)
              QT4_WRAP_UI(Q${KITNAME}_GENERATED_UI_CPP ${UI_FILES})
            ENDIF(UI_FILES)

            IF(MOC_H_FILES)
              QT4_WRAP_CPP(Q${KITNAME}_GENERATED_MOC_CPP ${MOC_H_FILES})
            ENDIF(MOC_H_FILES)

            IF(QRC_FILES)
              QT4_ADD_RESOURCES(Q${KITNAME}_GENERATED_QRC_CPP ${QRC_FILES})
            ENDIF(QRC_FILES)

            SET(Q${KITNAME}_GENERATED_CPP ${Q${KITNAME}_GENERATED_CPP} ${Q${KITNAME}_GENERATED_UI_CPP} ${Q${KITNAME}_GENERATED_MOC_CPP} ${Q${KITNAME}_GENERATED_QRC_CPP})

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

            SET(coverage_sources ${CPP_FILES} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES} ${TXX_FILES} ${TOOL_GUI_CPPS})
            SET_PROPERTY(SOURCE ${coverage_sources} APPEND PROPERTY LABELS ${MODULE_SUBPROJECTS})

            IF(NOT MODULE_HEADERS_ONLY)
              IF(ALL_LIBRARY_DIRS)
                # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
                LINK_DIRECTORIES(${ALL_LIBRARY_DIRS})
              ENDIF(ALL_LIBRARY_DIRS)
              ADD_LIBRARY(${MODULE_PROVIDES} ${coverage_sources} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP} ${DOX_FILES} ${UI_FILES} ${QRC_FILES})
              TARGET_LINK_LIBRARIES(${MODULE_PROVIDES} ${QT_LIBRARIES} ${ALL_LIBRARIES} QVTK)
              IF(MODULE_TARGET_DEPENDS)
                ADD_DEPENDENCIES(${MODULE_PROVIDES} ${MODULE_TARGET_DEPENDS})
              ENDIF()
              IF(MINGW)
                TARGET_LINK_LIBRARIES(${MODULE_PROVIDES} ssp) # add stack smash protection lib
              ENDIF()
              IF(MODULE_SUBPROJECTS)
                SET_PROPERTY(TARGET ${MODULE_PROVIDES} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
                FOREACH(subproject ${MODULE_SUBPROJECTS})
                  ADD_DEPENDENCIES(${subproject} ${MODULE_PROVIDES})
                ENDFOREACH()
              ENDIF()
            ENDIF()

          ENDIF(NOT MODULE_QT_MODULE)

          IF(NOT MODULE_HEADERS_ONLY)
            # Apply properties to the module target.
            SET_TARGET_PROPERTIES(${MODULE_PROVIDES} PROPERTIES
              COMPILE_FLAGS "${module_compile_flags}"
            )
          ENDIF()

          # install only if shared lib (for now)
          IF(NOT _STATIC OR MINGW)
            IF(NOT MODULE_HEADERS_ONLY)
              # # deprecated: MITK_INSTALL_TARGETS(${MODULE_PROVIDES})
            ENDIF()
          ENDIF(NOT _STATIC OR MINGW)

        ENDIF(NOT MODULE_QT_MODULE OR MITK_USE_QT)
      ENDIF(MODULE_IS_ENABLED)
    ENDIF(_MISSING_DEP)
  ENDIF(NOT MODULE_IS_EXCLUDED)
    
  IF(NOT MODULE_IS_ENABLED)
    _MITK_CREATE_MODULE_CONF()
  ENDIF(NOT MODULE_IS_ENABLED)

ENDMACRO(MITK_CREATE_MODULE)
