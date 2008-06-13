# sets name to the plugin-name
# the plugin-name is computed by removing basedir from dir and
# replacing slashes with points
MACRO(CREATE_PLUGIN_NAME name basedir dir)

  #MESSAGE(STATUS "name: ${name}, basedir: ${basedir}, dir: ${dir}")

  STRING(REPLACE "${ARGV1}/" "" PLUGIN_NAME_WITH_SLASHES ${dir})
  #MESSAGE(STATUS "PLUGIN_NAME_WITH_SLASHES: ${PLUGIN_NAME_WITH_SLASHES}")
  STRING(REPLACE "/" "." PLUGIN_NAME_WITH_POINTS ${PLUGIN_NAME_WITH_SLASHES})
  #MESSAGE(STATUS "PLUGIN_NAME_WITH_POINTS: ${PLUGIN_NAME_WITH_POINTS}")

  SET(${name} ${PLUGIN_NAME_WITH_POINTS})
  
ENDMACRO(CREATE_PLUGIN_NAME)


# sets pluginpath to the absolut path of the plugin with name pluginname
# 
# the absolut path is found by looking it up in a cache variable which
# contains an alternating list of plugin-names and their absolut paths
#
# pluginname: the unique plugin id (i.e. org.opencherry.osgi)
#
MACRO(FIND_PLUGIN_PATH pluginpath pluginname)

  #IF(${pluginname} MATCHES "org.opencherry.*")
  #  SET(${pluginpath} ${OPENCHERRY_SOURCE_DIR}/${ARGV1}/${PLUGIN_SRC_PREFIX})
  #ELSEIF(${pluginname} MATCHES "org.opencherry.*")
    SET(plugin_found FALSE)
    FOREACH(plugin_includedir ${OPENCHERRY_PLUGIN_INCLUDEDIRS})
      MESSAGE(STATUS "searching includedir: ${plugin_includedir}, found = ${plugin_found}")
      IF(plugin_found)
        SET(${pluginpath} ${plugin_includedir})
        SET(plugin_found false)
      ENDIF(plugin_found)
      
      IF(plugin_includedir STREQUAL ${pluginname})
        SET(plugin_found TRUE)
      ENDIF(plugin_includedir STREQUAL ${pluginname})
      
    ENDFOREACH(plugin_includedir)
  #ENDIF(${pluginname} MATCHES "org.opencherry.*")

ENDMACRO(FIND_PLUGIN_PATH)


# Recursively searches for plugins (i.e. directories containing a
# META-INF/MANIFEST.MF file). The macro adds the found directories
# to cmake and stored the plugin-name and absolute path in an
# internal cache variable. 
#
# VARIABLES:
#
# plugin_output_dir is the common output directory for all 
# plugins found. It is available as PLUGINS_OUTPUT_BASE_DIR
# and used by the CREATE_PLUGIN macro.
#
# add_to_plugin_dirs boolean value determining if the plugin_output_dir
# should be added to the OPENCHERRY_PLUGIN_DIRS variable. Should be usually
# set to ON
#
# build_all if ON the BUILD_pluginname variables are ignored and all
# plugins under this directory are build
#
MACRO(COLLECT_PLUGINS plugin_output_dir add_to_plugin_dirs build_all)
  
SET(PLUGINS_OUTPUT_BASE_DIR ${plugin_output_dir})
SET(ADD_TO_PLUGIN_DIRS ${add_to_plugin_dirs})
SET(BUILD_ALL_PLUGINS ${build_all})

#MESSAGE(SEND_ERROR "PLUGINS_OUTPUT_BASE_DIR: ${PLUGINS_OUTPUT_BASE_DIR}")
#MESSAGE(SEND_ERROR "ADD_TO_PLUGIN_DIRS: ${ADD_TO_PLUGIN_DIRS}")
#MESSAGE(SEND_ERROR "BUILD_ALL_PLUGINS: ${BUILD_ALL_PLUGINS}")

SET(PLUGINS_SOURCE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Collect plugin directories
FILE(GLOB_RECURSE ALL_FILES *)
SET(PLUGIN_DIRS)
FOREACH(FILEENTRY ${ALL_FILES})
  GET_FILENAME_COMPONENT(FILE_PATH ${FILEENTRY} PATH)
  
  IF(EXISTS ${FILE_PATH}/META-INF/MANIFEST.MF)
    #MESSAGE(STATUS "FILE_PATH: ${FILE_PATH}")
    SET(contains FALSE)
    FOREACH(val ${PLUGIN_DIRS})
      IF(val STREQUAL FILE_PATH)
        SET(contains TRUE)
        #BREAK()
      ENDIF(val STREQUAL FILE_PATH)
    ENDFOREACH(val)
  
    IF(NOT contains)
      #MESSAGE(STATUS "Setting ${FILE_PATH}")
      SET(PLUGIN_DIRS ${PLUGIN_DIRS} ${FILE_PATH})
    ENDIF(NOT contains)
  ENDIF(EXISTS ${FILE_PATH}/META-INF/MANIFEST.MF)
ENDFOREACH(FILEENTRY)


FOREACH(PLUGIN_DIR ${PLUGIN_DIRS})
      CREATE_PLUGIN_NAME(PLUGIN_NAME ${PLUGINS_SOURCE_BASE_DIR} ${PLUGIN_DIR})
      # build a list of plugin names and directories to look up
      # include paths later on
      
      SET(PLUGIN_SRC_PREFIX "")
      IF(IS_DIRECTORY ${PLUGIN_DIR}/src)
        SET(PLUGIN_SRC_PREFIX "/src")
      ENDIF(IS_DIRECTORY ${PLUGIN_DIR}/src)
      
      SET(OPENCHERRY_PLUGIN_INCLUDEDIRS
          ${OPENCHERRY_PLUGIN_INCLUDEDIRS}
          ${PLUGIN_NAME} ${PLUGIN_DIR}${PLUGIN_SRC_PREFIX}
          CACHE INTERNAL "Map plugin names to their path")    
ENDFOREACH(PLUGIN_DIR)

FOREACH(PLUGIN_DIR ${PLUGIN_DIRS}) 
      MESSAGE(STATUS "Processing ${PLUGIN_DIR}")
      CREATE_PLUGIN_NAME(PLUGIN_NAME ${PLUGINS_SOURCE_BASE_DIR} ${PLUGIN_DIR})
      #MESSAGE(STATUS "PLUGIN_NAME = ${PLUGIN_NAME}")
      OPTION("BUILD_${PLUGIN_NAME}" "Build ${PLUGIN_NAME} Plugin" ON)
      IF(BUILD_${PLUGIN_NAME} OR BUILD_ALL_PLUGINS)
        ADD_SUBDIRECTORY(${PLUGIN_DIR})
      ENDIF(BUILD_${PLUGIN_NAME} OR BUILD_ALL_PLUGINS)
ENDFOREACH(PLUGIN_DIR)

IF(ADD_TO_PLUGIN_DIRS)
  SET(MATCHED_DIR 0)
  IF(${OPENCHERRY_PLUGIN_DIRS} MATCHES ${PLUGINS_OUTPUT_BASE_DIR})
    SET(MATCHED_DIR 1)
  ENDIF(${OPENCHERRY_PLUGIN_DIRS} MATCHES ${PLUGINS_OUTPUT_BASE_DIR})

  IF(NOT MATCHED_DIR)
    SET(OPENCHERRY_PLUGIN_DIRS "${OPENCHERRY_PLUGIN_DIRS},${PLUGINS_OUTPUT_BASE_DIR}"
        CACHE STRING "A comma-separated list of directories containing plugins" FORCE)
  ENDIF(NOT MATCHED_DIR)
ENDIF(ADD_TO_PLUGIN_DIRS)

ENDMACRO(COLLECT_PLUGINS)


# Creates the current plugin.
# This macro should be called from the plugins CMakeLists.txt file. The plugin
# library is build in a subdirectory of PLUGIN_OUTPUT_DIR, which has been set
# by a call to COLLECT_PLUGINS(). The target name is available as PLUGIN_TARGET
# to add additional libraries in your CMakeLists.txt. Include paths are set
# depending on the variable PLUGIN_DEPENDS_ON which contains a list of plugin-names
# from which header files are needed.
#
MACRO(CREATE_PLUGIN)
  SUPPRESS_VC8_DEPRECATED_WARNINGS()
  CREATE_PLUGIN_NAME(PLUGIN_NAME ${PLUGINS_SOURCE_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  
  STRING(REPLACE . _ PLUGIN_TARGET ${PLUGIN_NAME})
  
  IF(NOT PLUGIN_CPP)
    MESSAGE(STATUS "Using files.cmake for plugin ${PLUGIN_NAME}")
    INCLUDE(files.cmake)
    SET(PLUGIN_CPP ${CPP_FILES})
    SET(PLUGIN_MOC_H ${MOC_H_FILES})
    SET(PLUGIN_FORMS ${UI_FILES})
  ELSE(NOT PLUGIN_CPP)
    MESSAGE(STATUS "Using PLUGIN_CPP for plugin ${PLUGIN_NAME}")
  ENDIF(NOT PLUGIN_CPP)
  
  SET(PLUGIN_OUTPUT_DIR ${PLUGINS_OUTPUT_BASE_DIR}/${PLUGIN_NAME})
  FILE(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})
  
  SET (LIBRARY_OUTPUT_PATH ${PLUGIN_OUTPUT_DIR}/bin)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.MF ${PLUGIN_OUTPUT_DIR}/META-INF/MANIFEST.MF COPYONLY)
  
  SET(PLUGINXML ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml)
  IF(EXISTS ${PLUGINXML})
    CONFIGURE_FILE(${PLUGINXML} ${PLUGIN_OUTPUT_DIR}/plugin.xml COPYONLY)
  ENDIF(EXISTS ${PLUGINXML})

  # set include directories
  FOREACH(plugin_dependency ${PLUGIN_DEPENDS_ON})
    FIND_PLUGIN_PATH(plugin_include_path ${plugin_dependency})
    MESSAGE(STATUS "ADDING INCLUDE DIR FOR DEPENDENCY ${plugin_dependency}: ${plugin_include_path}")
    INCLUDE_DIRECTORIES(${plugin_include_path})
  ENDFOREACH(plugin_dependency)


  SET(PLUGIN_CPP_LIST ${PLUGIN_CPP})
  IF(PLUGIN_FORMS)
    QT4_WRAP_UI(PLUGIN_CPP_LIST ${PLUGIN_FORMS})
  ENDIF(PLUGIN_FORMS)
  IF(PLUGIN_MOC_H)
    QT4_WRAP_CPP(PLUGIN_CPP_LIST ${PLUGIN_MOC_H})
  ENDIF(PLUGIN_MOC_H)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  
  IF(OPENCHERRY_STATIC)
    ADD_LIBRARY(${PLUGIN_TARGET} STATIC ${PLUGIN_CPP_LIST})
  ELSE(OPENCHERRY_STATIC)
    ADD_LIBRARY(${PLUGIN_TARGET} SHARED ${PLUGIN_CPP_LIST})
    FOREACH(plugin_dependency ${PLUGIN_DEPENDS_ON})
      STRING(REPLACE . _ plugin_deplib ${plugin_dependency})
      MESSAGE(STATUS "ADDING LINK LIBRARY: ${plugin_deplib}")
      TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} ${plugin_deplib})
    ENDFOREACH(plugin_dependency)
  ENDIF(OPENCHERRY_STATIC)

ENDMACRO(CREATE_PLUGIN)


# Macro to set specific Qt options, calls CREATE_PLUGIN at the end
MACRO(CREATE_QT_PLUGIN)
  
  IF(USE_QT AND DESIRED_QT_VERSION EQUAL 4)
    INCLUDE_DIRECTORIES(${QT_INCLUDES})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
  
    CREATE_PLUGIN()
  
    TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} ${QT_LIBRARIES})
  ENDIF(USE_QT AND DESIRED_QT_VERSION EQUAL 4)
  
ENDMACRO(CREATE_QT_PLUGIN)

