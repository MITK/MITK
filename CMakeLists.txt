PROJECT(MITK)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# this should be set to NEW at some point, see bug #1402
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
SET(MITK_MODULES_CONF_DIR ${MITK_BINARY_DIR}/modulesConf CACHE INTERNAL "Modules Conf")

# Clear the cache variables containing plugin base directories
SET(MITK_MODULES_PLUGIN_SOURCE_DIRS "" CACHE INTERNAL "List of base plugin source directories" FORCE)
SET(MITK_MODULES_PLUGIN_OUTPUT_DIRS "" CACHE INTERNAL "List of base plugin output directories" FORCE)
SET(MITK_CORE_PLUGIN_OUTPUT_DIRS "" CACHE INTERNAL "List of base plugin output directories" FORCE)
SET(MITK_CORE_PLUGIN_SOURCE_DIRS "" CACHE INTERNAL "List of base plugin source directories" FORCE)

# only configure dart here if not already configured in the 
# parent directory
IF(NOT MITK_ALL_BUILD)
  INCLUDE(CTest)
ENDIF(NOT MITK_ALL_BUILD)

INCLUDE(CMake/CoreHelpers.cmake)
INCLUDE(CMake/ToolExtensionHelpers.cmake)
INCLUDE(CMake/FunctionalityHelpers.cmake)
SET(KITNAME "MITK")
SET(LIBPOSTFIX "")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE STRING "Global output directory for runtime files")
# maybe change the next to directories to "lib" ?
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE STRING "Global output directory for library files")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE STRING "Global output directory for archive files")
MARK_AS_ADVANCED(CMAKE_RUNTIME_OUTPUT_DIRECTORY CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

# MITK_VERSION
SET(MITK_VERSION_MAJOR "0")
SET(MITK_VERSION_MINOR "13")
SET(MITK_VERSION_PATCH "1")
SET(MITK_VERSION_STRING "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")

IF(NOT MITK_DIR)
  SET(MITK_DIR ${CMAKE_CURRENT_BINARY_DIR})
ENDIF(NOT MITK_DIR)

# some targets in Utilities also depend on Qt. Use this option
# to decide if they should be build
OPTION(MITK_USE_QT "Use Trolltech's Qt library" ON)
IF(MITK_USE_QT)
  # find the package at the very beginning, so that QT4_FOUND is available
  FIND_PACKAGE(Qt4 REQUIRED)
ENDIF()

IF(NOT UNIX AND NOT MINGW)
  SET(MITK_WIN32_FORCE_STATIC "STATIC" CACHE INTERNAL "Use this variable to always build static libraries on non-unix platforms")
ENDIF()

# we need to define a Windows version
IF(MINGW)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0500")
ENDIF()

ADD_SUBDIRECTORY(Utilities)

OPTION(MITK_USE_OPENCHERRY "Build the openCherry platform" ON)
IF(MITK_USE_OPENCHERRY)
  SET(mbilog_DIR "${mbilog_BINARY_DIR}")
  ADD_SUBDIRECTORY(openCherry)
  SET(openCherry_DIR ${CMAKE_CURRENT_BINARY_DIR}/openCherry CACHE PATH "The directory containing a CMake configuration file for openCherry" FORCE)
  INCLUDE(${openCherry_DIR}/openCherryConfig.cmake)
  INCLUDE(CMake/MITKPluginHelpers.cmake)
  
  SET(OPENCHERRY_USE_QT ${MITK_USE_QT} CACHE BOOL "Use the Qt GUI toolkit" FORCE)
ENDIF(MITK_USE_OPENCHERRY)

IF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Utilities/itk-bin)
  SET(ITK_DIR "${CMAKE_CURRENT_BINARY_DIR}/Utilities/itk-bin")
ENDIF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Utilities/itk-bin)

FIND_PACKAGE(ITK)

# TODO: check if necessary
INCLUDE(CMakeImportBuildSettings)
IF(ITK_BUILD_SETTINGS_FILE)
  CMAKE_IMPORT_BUILD_SETTINGS(${ITK_BUILD_SETTINGS_FILE})
ENDIF(ITK_BUILD_SETTINGS_FILE)

IF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Utilities/vtk-bin)
  SET(VTK_DIR "${CMAKE_CURRENT_BINARY_DIR}/Utilities/vtk-bin")
ENDIF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Utilities/vtk-bin)
FIND_PACKAGE(VTK)
APPLY_VTK_FLAGS()

OPTION(BUILD_SHARED_LIBS "Build MITK with shared libraries." ON)

# TODO: check if necessary
OPTION(USE_ITKZLIB "Use the ITK zlib for pic compression." ON)
MARK_AS_ADVANCED(USE_ITKZLIB)

# build the MITK_INCLUDE_DIRS variable
SET(CORE_DIRECTORIES DataManagement Algorithms IO Rendering Interactions Controllers)

FOREACH(d ${CORE_DIRECTORIES})
  SET(${KITNAME}_INCLUDE_DIRS ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/Core/Code/${d})
ENDFOREACH(d)

SET(${KITNAME}_CORE_LIBRARIES ${${KITNAME}_CORE_LIBRARIES} mitkCore)
SET(${KITNAME}_INCLUDE_DIRS ${PROJECT_BINARY_DIR} ${${KITNAME}_INCLUDE_DIRS} ${ITK_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS})

FOREACH(dir Utilities Utilities/ipPic
Utilities/IIL4MITK Utilities/pic2vtk Utilities/tinyxml Utilities/mbilog)
  SET(${KITNAME}_INCLUDE_DIRS ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
ENDFOREACH(dir)
  
IF(WIN32)
  SET(${${KITNAME}_INCLUDE_DIRS} ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipPic/win32)
ENDIF(WIN32)


SET(${KITNAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/itkUNC ${${KITNAME}_INCLUDE_DIRS})
SET(MITK_LINK_DIRECTORIES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${ITK_LIBRARY_DIRS} ${VTK_LIBRARY_DIRS})
LINK_DIRECTORIES(${MITK_LINK_DIRECTORIES})

SET(VTK_FOR_MITK_LIBRARIES
  vtkGraphics vtkCommon vtkFiltering vtkftgl vtkGraphics vtkHybrid vtkImaging vtkIO vtkParallel vtkRendering vtkVolumeRendering vtkWidgets ${VTK_JPEG_LIBRARIES} ${VTK_PNG_LIBRARIES} ${VTK_ZLIB_LIBRARIES} ${VTK_EXPAT_LIBRARIES} ${VTK_FREETYPE_LIBRARIES}
)

# TODO: maybe solve this with lib depends mechanism of CMake
SET(UTIL_FOR_MITK_LIBRARIES mitkIpPic mitkIpFunc)
SET(LIBRARIES_FOR_MITK_CORE
  ${LIBRARIES_FOR_${KITNAME}_CORE} ${UTIL_FOR_${KITNAME}_LIBRARIES} ${VTK_FOR_${KITNAME}_LIBRARIES} ${ITK_LIBRARIES}
)

SET(ANN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ann/include)
SET(IPSEGMENTATION_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipSegmentation)

ADD_SUBDIRECTORY(Core)

SET(MITK_LIBRARIES ${MITK_CORE_LIBRARIES} ${LIBRARIES_FOR_MITK_CORE} ${IPFUNC_LIBRARY} pic2vtk IIL4MITK ipSegmentation ann )

IF(MITK_USE_QT)
  FIND_PACKAGE(Qt4 REQUIRED)

  ADD_DEFINITIONS(-DQWT_DLL)
  SET(Q${KITNAME}_INCLUDE_DIRS ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Qmitk ${PROJECT_BINARY_DIR}/CoreUI/Qmitk)
    FOREACH(dir QmitkApplicationBase QmitkModels QmitkPropertyObservers) 
      SET(Q${KITNAME}_INCLUDE_DIRS ${Q${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Qmitk/${dir})
    ENDFOREACH(dir)
    SET(Q${KITNAME}_INCLUDE_DIRS ${Q${KITNAME}_INCLUDE_DIRS} ${QWT_INCLUDE_DIR})
  SET(QMITK_LIBRARIES Qmitk ${MITK_LIBRARIES} ${QT_LIBRARIES})
ENDIF(MITK_USE_QT)

#### find out version number #####

FIND_PACKAGE(Subversion)
IF(Subversion_FOUND AND MITK_USE_SUBVERSION)
  IF(EXISTS ${mitk-all_SOURCE_DIR})
    Subversion_WC_INFO(${mitk-all_SOURCE_DIR} SVN_INFO)
  ELSE(EXISTS ${mitk-all_SOURCE_DIR})
    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} SVN_INFO)
  ENDIF(EXISTS ${mitk-all_SOURCE_DIR})
  SET(MITK_SVN_REVISION ${SVN_INFO_WC_REVISION})
ELSE(Subversion_FOUND AND MITK_USE_SUBVERSION)
  SET(MITK_SVN_REVISION "n/a")
ENDIF(Subversion_FOUND AND MITK_USE_SUBVERSION)
CONFIGURE_FILE(mitkVersion.h.in ${PROJECT_BINARY_DIR}/mitkVersion.h)

# TODO: check what could be moved to subdirectories
#### mitkConfig.h ####
SET(MITK_CONFIG_H ${PROJECT_BINARY_DIR}/mitkConfig.h)
CONFIGURE_FILE(mitkConfig.h.in ${MITK_CONFIG_H})
SET(MITK_TESTING_CONFIG_H ${PROJECT_BINARY_DIR}/mitkTestingConfig.h)
SET(MITK_TEST_OUTPUT_DIR "${PROJECT_BINARY_DIR}/test_output")
IF(NOT EXISTS ${MITK_TEST_OUTPUT_DIR})
FILE(MAKE_DIRECTORY ${MITK_TEST_OUTPUT_DIR})
ENDIF(NOT EXISTS ${MITK_TEST_OUTPUT_DIR})
CONFIGURE_FILE(mitkTestingConfig.h.in ${MITK_TESTING_CONFIG_H})
#### MITKConfig.cmake ####
SET(${KITNAME}_INCLUDE_DIRS_CONFIG ${${KITNAME}_INCLUDE_DIRS})
SET(Q${KITNAME}_INCLUDE_DIRS_CONFIG ${Q${KITNAME}_INCLUDE_DIRS})
SET(VECMATH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/vecmath)
SET(IPFUNC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipFunc)
SET(UTILITIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

IF(MITK_USE_OPENCHERRY)
  OPTION(MITK_BUILD_ALL_PLUGINS "Build all MITK plugins (overriding selection)" OFF)
  MARK_AS_ADVANCED(MITK_BUILD_ALL_PLUGINS)
  
  IF(MITK_BUILD_ALL_PLUGINS)
    SET(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY(Documentation)
ADD_SUBDIRECTORY(CoreUI)
ADD_SUBDIRECTORY(Modules)

CONFIGURE_FILE(${KITNAME}Config.cmake.in ${PROJECT_BINARY_DIR}/${KITNAME}Config.cmake @ONLY)

ADD_SUBDIRECTORY(Applications)

# CPACK OPTIONS

FIND_PROGRAM(NSIS_MAKENSIS NAMES makensis
  PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
  DOC "Where is makensis.exe located"
  )
IF(NOT NSIS_MAKENSIS)
  SET(CPACK_GENERATOR ZIP)
ELSE(WIN32)
  SET(CPACK_GENERATOR "NSIS;ZIP")
ENDIF(NOT NSIS_MAKENSIS)

SET(MITK_CPACK_PACKAGE_EXECUTABLES "ExtApp;MITK - Ext Application\"\"CoreApp;MITK - Core Application" CACHE INTERNAL "Collecting windows shortcuts to executables")

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/MITKCPackOptions.cmake.in ${PROJECT_BINARY_DIR}/MITKCPackOptions.cmake @ONLY)

SET(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/MITKCPackOptions.cmake")

SET(CPACK_PACKAGE_NAME "MITK")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MITK is a medical image processing tool")
SET(CPACK_PACKAGE_VENDOR "German Cancer Research Center (DKFZ)")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/MITKCopyright.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/MITKCopyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MITK_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MITK_VERSION_MINOR}")
# append revision number if available
IF(SVN_INFO_WC_REVISION)
  SET(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}_r${SVN_INFO_WC_REVISION}")
ELSE(SVN_INFO_WC_REVISION)
  SET(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}")
ENDIF(SVN_INFO_WC_REVISION)
############################# INSTALL TARGETS ########################## 
INSTALL(FILES ${PROJECT_SOURCE_DIR}/mitk.ico DESTINATION bin)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/mitk.bmp DESTINATION bin)

# directory has to be created to make sure the installer does not report
# an error if doxygen was not invoked before.
IF (MITK_HELPPAGES_OUTPUT_DIR)
  FILE(MAKE_DIRECTORY ${MITK_HELPPAGES_OUTPUT_DIR}/html)
  INSTALL(DIRECTORY ${MITK_HELPPAGES_OUTPUT_DIR}/html DESTINATION doc)
ENDIF (MITK_HELPPAGES_OUTPUT_DIR)

#OPENCV DLLs may be added elsewhere to install target

#STATEMACHINE XML
INSTALL(FILES ${PROJECT_SOURCE_DIR}/Core/Code/Interactions/StateMachine.xml DESTINATION bin)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/Config/mitkLevelWindowPresets.xml DESTINATION bin)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/Config/mitkRigidRegistrationPresets.xml DESTINATION bin)

IF(WIN32)
  #QT
  IF(MITK_USE_QT)
    SET(QT_USE_QTSVG 1)
	SET(QT_USE_QTOPENGL 1)
    SET(QT_USE_QT3SUPPORT 1)
    INCLUDE(${QT_USE_FILE})
    
    SET(qtdebugdlls)
    SET(qtreleasedlls)
    FOREACH(qtlib ${QT_LIBRARIES})
      GET_FILENAME_COMPONENT(qtdllname ${qtlib} NAME_WE)
      # strip the leading "lib" from the filename for MINGW installs
      IF(MINGW)
        STRING(REPLACE "lib" "" qtdllname ${qtdllname})
      ENDIF()
      STRING(REGEX MATCH ".*d4" debugmatch ${qtdllname})
      IF(debugmatch)
        LIST(APPEND qtdebugdlls "${QT_INCLUDE_DIR}/../bin/${qtdllname}.dll")
      ELSE()
        STRING(REGEX MATCH ".*4" releasematch ${qtdllname})
      IF(releasematch)
        LIST(APPEND qtreleasedlls "${QT_INCLUDE_DIR}/../bin/${qtdllname}.dll")
      ENDIF()
    ENDIF()
    ENDFOREACH(qtlib)
  
    INSTALL(FILES ${qtdebugdlls} DESTINATION bin CONFIGURATIONS Debug)
    INSTALL(FILES ${qtreleasedlls} DESTINATION bin CONFIGURATIONS Release)
    
  ENDIF(MITK_USE_QT)
 
  #VTK DLLs install target
  IF(MINGW)
    FILE(GLOB vtkreleasedlllist ${VTK_RUNTIME_LIBRARY_DIRS}/*.dll)
    FILE(GLOB vtkdebugdlllist ${VTK_RUNTIME_LIBRARY_DIRS}/*.dll)
	
	# VTKs freetype library is buggy when used with gcc, need freetype > 2.3.9
	IF(VTK_FREETYPE_LIBRARIES)
	  INSTALL(FILES ${VTK_FREETYPE_LIBRARIES} DESTINATION bin)
	ENDIF()
  ELSE()
    FILE(GLOB vtkreleasedlllist ${VTK_RUNTIME_LIBRARY_DIRS}/release/*.dll)
    FILE(GLOB vtkdebugdlllist ${VTK_RUNTIME_LIBRARY_DIRS}/debug/*.dll)
  ENDIF()
  
  INSTALL(FILES ${vtkdebugdlllist} DESTINATION bin CONFIGURATIONS Debug)
  INSTALL(FILES ${vtkreleasedlllist} DESTINATION bin CONFIGURATIONS Release) 
  
  #ITK DLLs install target
  IF(MINGW)
    FILE(GLOB itkdebugdlllist ${ITK_LIBRARY_DIRS}/*.dll)
    FILE(GLOB itkreleasedlllist ${ITK_LIBRARY_DIRS}/*.dll)
  ELSE()
    FILE(GLOB itkdebugdlllist ${ITK_LIBRARY_DIRS}/debug/*.dll)
    FILE(GLOB itkreleasedlllist ${ITK_LIBRARY_DIRS}/release/*.dll)
  ENDIF()
  
  INSTALL(FILES ${itkdebugdlllist} DESTINATION bin CONFIGURATIONS Debug)
  INSTALL(FILES ${itkreleasedlllist} DESTINATION bin CONFIGURATIONS Release)
  
  #DCMTK Dlls install target (shared libs on gcc only)
  IF(MINGW AND DCMTK_ofstd_LIBRARY)
    SET(_dcmtk_libs
	    ${DCMTK_dcmdata_LIBRARY}
		${DCMTK_dcmimgle_LIBRARY}
		${DCMTK_dcmnet_LIBRARY}
		${DCMTK_ofstd_LIBRARY}
    )
	FOREACH(_dcmtk_lib ${_dcmtk_libs})
	  INSTALL(FILES ${_dcmtk_lib} DESTINATION bin)
	ENDFOREACH()
  ENDIF()
  
  #OpenCherry
  IF(MITK_USE_OPENCHERRY)
    IF(MINGW)
      INSTALL(FILES ${OPENCHERRY_PLUGINS_OUTPUT_DIR}/org.opencherry.osgi/bin/liborg_opencherry_osgid.dll DESTINATION bin CONFIGURATIONS Debug)
      INSTALL(FILES ${OPENCHERRY_PLUGINS_OUTPUT_DIR}/org.opencherry.osgi/bin/liborg_opencherry_osgi.dll DESTINATION bin CONFIGURATIONS Release)
    ELSE()
      INSTALL(FILES ${OPENCHERRY_PLUGINS_OUTPUT_DIR}/org.opencherry.osgi/bin/debug/liborg_opencherry_osgid.dll DESTINATION bin CONFIGURATIONS Debug)
      INSTALL(FILES ${OPENCHERRY_PLUGINS_OUTPUT_DIR}/org.opencherry.osgi/bin/release/liborg_opencherry_osgi.dll DESTINATION bin CONFIGURATIONS Release)
    ENDIF()
  ENDIF()
  
  #MinGW dll
  IF(MINGW)
    FIND_LIBRARY(MINGW_RUNTIME_DLL "mingwm10.dll" HINTS ${CMAKE_FIND_ROOT_PATH}/sys-root/mingw/bin)
	IF (MINGW_RUNTIME_DLL)
      INSTALL(FILES ${MINGW_RUNTIME_DLL} DESTINATION bin)
    ELSE()
      MESSAGE(SEND_ERROR "Could not find mingwm10.dll which is needed for a proper install")
    ENDIF()
	
	FIND_LIBRARY(MINGW_GCC_RUNTIME_DLL "libgcc_s_dw2-1.dll" HINTS ${CMAKE_FIND_ROOT_PATH}/sys-root/mingw/bin)
	IF (MINGW_GCC_RUNTIME_DLL)
      INSTALL(FILES ${MINGW_GCC_RUNTIME_DLL} DESTINATION bin)
    ELSE()
      MESSAGE(SEND_ERROR "Could not find libgcc_s_dw2-1.dll which is needed for a proper install")
    ENDIF()
  ENDIF()

 
ENDIF(WIN32)
   

# include required mfc libraries
INCLUDE(InstallRequiredSystemLibraries)
# include CPack model once all variables are set
INCLUDE(CPack)
