##################################################################
#
# MITK_CREATE_MODULE 
#
# Creates a module for the automatic module dependeny system within MITK.
# Configurations are generated in the moduleConf directory. 
#
# USAGE:
#
# MITK_CREATE_MODULE( <moduleName>
#     [INCLUDE_DIRS <include directories>]
#     [INTERNAL_INCLUDE_DIRS <internally used include directories>]
#     [DEPENDS <modules we need>]
#     [PROVIDES <library name which is built>]
#     [PACKAGE_DEPENDS <packages we need, like ITK, VTK, QT>]
#     [EXPORT_DEFINE <declspec macro name for dll exports>]
#     [QT_MODULE]
#
##################################################################
MACRO(MITK_CREATE_MODULE MODULE_NAME_IN)
  MACRO_PARSE_ARGUMENTS(MODULE
                        "INCLUDE_DIRS;INTERNAL_INCLUDE_DIRS;DEPENDS;DEPENDS_INTERNAL;PROVIDES;PACKAGE_DEPENDS;EXPORT_DEFINE;ADDITIONAL_LIBS;GENERATED_CPP"
                        "QT_MODULE;FORCE_STATIC;HEADERS_ONLY"
                        ${ARGN})
                        
  SET(MODULE_NAME ${MODULE_NAME_IN})
  
  # assume worst case
  SET(MODULE_IS_ENABLED 0)
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
        CONFIGURE_FILE(${MITK_SOURCE_DIR}/CMake/moduleExports.h.in ${MITK_MODULES_CONF_DIR}/${MODULE_NAME}Exports.h @ONLY)
      
        IF(MITK_GENERATE_MODULE_DOT)
          MESSAGE("MODULEDOTNAME ${MODULE_NAME}")
          FOREACH(dep ${MODULE_DEPENDS})
            MESSAGE("MODULEDOT \"${MODULE_NAME}\" -> \"${dep}\" ; ")
          ENDFOREACH(dep)
        ENDIF(MITK_GENERATE_MODULE_DOT)
     
        SET(DEPENDS "${MODULE_DEPENDS}") 
        SET(DEPENDS_BEFORE "not initialized")
        SET(PACKAGE_DEPENDS "${MODULE_PACKAGE_DEPENDS}")
        MITK_USE_MODULE("${MODULE_DEPENDS}") 
      
        # ok, now create the module itself 
        INCLUDE_DIRECTORIES(. ${ALL_INCLUDE_DIRECTORIES})
        INCLUDE(files.cmake)
      
        IF(NOT MODULE_QT_MODULE)
          ORGANIZE_SOURCES(SOURCE ${CPP_FILES}
                           HEADER ${H_FILES}
                           TXX ${TXX_FILES}
                           DOC ${DOX_FILES}
                           )

          IF(MODULE_FORCE_STATIC)
            SET(_STATIC ${MITK_WIN32_FORCE_STATIC})
          ENDIF(MODULE_FORCE_STATIC)

          IF(NOT MODULE_HEADERS_ONLY)
            IF(ALL_LIBRARY_DIRS)
              # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
              LINK_DIRECTORIES(${ALL_LIBRARY_DIRS})
            ENDIF(ALL_LIBRARY_DIRS)  
            ADD_LIBRARY(${MODULE_PROVIDES} ${_STATIC} ${CPP_FILES} ${H_FILES} ${CPP_FILES_GENERATED} ${GLOBBED__H_FILES} ${CORRESPONDING__H_FILES} ${TXX_FILES} ${DOX_FILES} ${UI_FILES} ${TOOL_CPPS})
            IF(ALL_LIBRARIES)
              TARGET_LINK_LIBRARIES(${MODULE_PROVIDES} ${ALL_LIBRARIES})
            ENDIF(ALL_LIBRARIES)  
          ENDIF()
      
        ELSE(NOT MODULE_QT_MODULE)

          INCLUDE(files.cmake)
      
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
      
          IF(NOT MODULE_HEADERS_ONLY)
            IF(ALL_LIBRARY_DIRS)
              # LINK_DIRECTORIES applies only to targets which are added after the call to LINK_DIRECTORIES
              LINK_DIRECTORIES(${ALL_LIBRARY_DIRS})
            ENDIF(ALL_LIBRARY_DIRS)  
            ADD_LIBRARY(${MODULE_PROVIDES} ${CPP_FILES} ${CPP_FILES_GENERATED} ${Q${KITNAME}_GENERATED_CPP} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES} ${TXX_FILES} ${DOX_FILES} ${UI_FILES} ${QRC_FILES} ${TOOL_GUI_CPPS})
            TARGET_LINK_LIBRARIES(${MODULE_PROVIDES} ${QT_LIBRARIES} ${ALL_LIBRARIES} QVTK)
          ENDIF()
      
        ENDIF(NOT MODULE_QT_MODULE) 


        # install only if shared lib (for now)
        IF(NOT _STATIC OR MINGW)
          IF(NOT MODULE_HEADERS_ONLY)
           # # deprecated: MITK_INSTALL_TARGETS(${MODULE_PROVIDES})
          ENDIF()
        ENDIF(NOT _STATIC OR MINGW)
        
      ENDIF(NOT MODULE_QT_MODULE OR MITK_USE_QT)
    ENDIF(MODULE_IS_ENABLED)
  ENDIF(_MISSING_DEP)
  
  IF(NOT MODULE_IS_ENABLED)   
    _MITK_CREATE_MODULE_CONF()
  ENDIF(NOT MODULE_IS_ENABLED)   
ENDMACRO(MITK_CREATE_MODULE)
