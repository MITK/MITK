PROJECT(MITK)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# this should be set to NEW at some point, see bug #1402
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Clear the cache variables containing plugin base directories
SET(MITK_PLUGIN_SOURCE_DIRS "" CACHE INTERNAL "List of base plugin source directories" FORCE)
SET(MITK_PLUGIN_BINARY_DIRS "" CACHE INTERNAL "List of base plugin binary directories" FORCE)

# only configure dart here if not already configured in the 
# parent directory
IF(NOT MITK_ALL_BUILD)
  INCLUDE(CTest)
ENDIF(NOT MITK_ALL_BUILD)

INCLUDE(CMake/CoreHelpers.cmake)
INCLUDE(CMake/FunctionalityHelpers.cmake)
SET(KITNAME "MITK")
SET(LIBPOSTFIX "")
SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")

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

ADD_SUBDIRECTORY(Utilities)

OPTION(MITK_USE_OPENCHERRY "Build the openCherry platform" OFF)
IF(MITK_USE_OPENCHERRY)
  ADD_SUBDIRECTORY(openCherry)
  SET(openCherry_DIR ${CMAKE_CURRENT_BINARY_DIR}/openCherry CACHE PATH "The directory containing a CMake configuration file for openCherry" FORCE)
  INCLUDE(${openCherry_DIR}/openCherryConfig.cmake)
  INCLUDE(CMake/MITKPluginHelpers.cmake)
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

IF(UNIX)
  OPTION(BUILD_SHARED_LIBS "Build MITK with shared libraries." ON)
ELSE(UNIX)
  OPTION(BUILD_SHARED_LIBS "Build MITK with shared libraries." OFF)
  IF(BUILD_SHARED_LIBS)
    MESSAGE(FATAL_ERROR 
	  "Sorry. General BUILD_SHARED_LIBS is not available yet for MITK. Please look at MITK_BUILD_SHARED_CORE for creating dlls.")
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(UNIX)

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
Utilities/IIL4MITK Utilities/pic2vtk Utilities/tinyxml)
  SET(${KITNAME}_INCLUDE_DIRS ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
ENDFOREACH(dir)
  
IF(WIN32)
  SET(${${KITNAME}_INCLUDE_DIRS} ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipPic/win32)
ENDIF(WIN32)


SET(${KITNAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/itkUNC ${${KITNAME}_INCLUDE_DIRS})
SET(MITK_LINK_DIRECTORIES ${LIBRARY_OUTPUT_PATH} ${ITK_LIBRARY_DIRS} ${VTK_LIBRARY_DIRS})
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
SET(MITK_IGT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Core/IGT/IGTFilters ${CMAKE_CURRENT_SOURCE_DIR}/Core/IGT/IGTTrackingDevices) 

IF(MITK_USE_QT)
  FIND_PACKAGE(Qt4)
  SET(Q${KITNAME}_INCLUDE_DIRS ${${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Qmitk ${PROJECT_BINARY_DIR}/Qmitk)
    FOREACH(dir QmitkApplicationBase QmitkModels QmitkPropertyObservers) 
      SET(Q${KITNAME}_INCLUDE_DIRS ${Q${KITNAME}_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Qmitk/${dir})
    ENDFOREACH(dir)
    SET(Q${KITNAME}_INCLUDE_DIRS ${Q${KITNAME}_INCLUDE_DIRS} ${QWT_INCLUDE_DIR})
  SET(QMITK_LIBRARIES Qmitk ${MITK_LIBRARIES} ${QT_LIBRARIES})
ENDIF(MITK_USE_QT)

#### find out version number #####

FIND_PACKAGE(Subversion)
IF(Subversion_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} SVN_INFO)
ENDIF(Subversion_FOUND)

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
CONFIGURE_FILE(${KITNAME}Config.cmake.in ${PROJECT_BINARY_DIR}/${KITNAME}Config.cmake @ONLY)

ADD_SUBDIRECTORY(Documentation)
#ADD_SUBDIRECTORY(CoreUI)

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
FILE(MAKE_DIRECTORY ${MITK_HELPPAGES_OUTPUT_DIR}/html)
INSTALL(DIRECTORY ${MITK_HELPPAGES_OUTPUT_DIR}/html DESTINATION doc)

#OPENCV DLLs may be added elsewhere to install target

#STATEMACHINE XML
INSTALL(FILES ${PROJECT_SOURCE_DIR}/Core/Interactions/StateMachine.xml DESTINATION bin)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/Config/mitkLevelWindowPresets.xml DESTINATION bin)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/QFunctionalities/QmitkRigidRegistration/mitkRigidRegistrationPresets.xml DESTINATION bin)

IF(WIN32)
  #QT
  FILE(GLOB qtdlllist ${QT_INCLUDE_DIR}/../bin/*.dll)
  FOREACH(qtlib ${qtdlllist})
    INSTALL(FILES ${qtdlllist} DESTINATION bin)
  ENDFOREACH(qtlib)
 
  #VTK DLLs install target
  FILE(GLOB vtkdlllist ${VTK_RUNTIME_LIBRARY_DIRS}/release/*.dll)
  FOREACH(vtklib ${vtkdlllist})
    INSTALL(FILES ${vtkdlllist} DESTINATION bin)
  ENDFOREACH(vtklib)
  
  # install ITK _release_ dll , this will probably crash with a MITK debug version. Also this
  # will fail if ITK starts using more than one directory for its libs.
  FILE(GLOB itkdlllist ${ITK_LIBRARY_DIRS}/release/*.dll)
  FOREACH(itklib ${itkdlllist})
    INSTALL(FILES ${itkdlllist} DESTINATION bin)
  ENDFOREACH(itklib)
ENDIF(WIN32)
   

  # include required mfc libraries
  INCLUDE(InstallRequiredSystemLibraries)
  # include CPack model once all variables are set
  INCLUDE(CPack)

