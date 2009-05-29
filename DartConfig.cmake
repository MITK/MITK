OPTION(USE_KITWARE_DARTBOARD "Use the Kitware Dartboard for Build Submissions." OFF)

# internal cdash
set(CTEST_PROJECT_NAME "MITK")
set(CTEST_NIGHTLY_START_TIME "21:00:00 CEST")
set(NIGHTLY_START_TIME "21:00:00 CEST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "mbits")
set(CTEST_DROP_LOCATION "/cdash/submit.php?project=MITK")
set(CTEST_DROP_SITE_CDASH TRUE)
set(DROP_METHOD "http")
set(DROP_SITE "mbits")
set(DROP_LOCATION "/cdash/submit.php?project=MITK")


# configure dart for submission to kitware
IF(USE_KITWARE_DARTBOARD)
#
# Dart server to submit results (used by client)
#
    SET(DROP_METHOD "http")
    SET(DROP_SITE "my.cdash.org")
    SET(DROP_LOCATION "/submit.php?project=MITK")
    SET(CTEST_DROP_METHOD "http")
    SET(CTEST_DROP_SITE "my.cdash.org")
    SET(CTEST_DROP_LOCATION "/submit.php?project=MITK")
    SET(CTEST_DROP_SITE_CDASH TRUE)

    SET(CTEST_NIGHTLY_START_TIME "21:00 CEST")
    SET (NIGHTLY_START_TIME "21:00:00 CEST")	
    
    SET (DOXYGEN_URL "http://docs.mitk.org/nightly/" )
        
ENDIF(USE_KITWARE_DARTBOARD)

IF(EXISTS ${MBI_DART_WORKDIR}/notes.txt)
  MAKE_DIRECTORY(${MBI_DART_WORKDIR}/bin/Testing/Notes)
  CONFIGURE_FILE(${MBI_DART_WORKDIR}/notes.txt ${MBI_DART_WORKDIR}/bin/Testing/Notes/notes.txt COPYONLY)
ENDIF(EXISTS ${MBI_DART_WORKDIR}/notes.txt)
IF(EXISTS ${ITK_DIR}/ITKConfig.cmake)
  INCLUDE(${ITK_DIR}/ITKConfig.cmake)
ENDIF(EXISTS ${ITK_DIR}/ITKConfig.cmake)

IF(EXISTS ${VTK_DIR}/VTKConfig.cmake) 
   # this second check is necessary since sometimes on the very first
   # CMake run VTKConfig exists but VTKConfigQt doesn't exist yet, breaking the
   # configure step
   IF(EXISTS ${VTK_DIR}/VTKConfigQt.cmake)
     INCLUDE(${VTK_DIR}/VTKConfig.cmake)
   ENDIF(EXISTS ${VTK_DIR}/VTKConfigQt.cmake)
ENDIF(EXISTS ${VTK_DIR}/VTKConfig.cmake)

IF(CMAKE_COMPILER_IS_GNUCXX)
  MESSAGE(STATUS "Compiler is GCC")
  EXEC_PROGRAM(${CMAKE_CXX_COMPILER} 
    ARGS --version
    OUTPUT_VARIABLE COMPILER_OUTPUT
  )
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  MESSAGE(STATUS "Compiler is not GCC")
  EXEC_PROGRAM(${CMAKE_CXX_COMPILER} 
    OUTPUT_VARIABLE COMPILER_OUTPUT
  )
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
MESSAGE(STATUS "Compiler version output: ${COMPILER_OUTPUT}")

SET(BUILD_NOTES_DIR ${PROJECT_BINARY_DIR}/Testing/Notes)
MAKE_DIRECTORY(${BUILD_NOTES_DIR})
SET(BUILD_NOTES_FILE ${BUILD_NOTES_DIR}/build-notes.txt)
FILE(WRITE ${BUILD_NOTES_FILE} "Toolkit versions for this build:
ITK_DIR: ${ITK_DIR}
VTK_DIR: ${VTK_DIR}
VTK version: ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${VTK_BUILD_VERSION}
ITK version: ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}.${ITK_VERSION_PATCH}
COMPILER output string: ${COMPILER_OUTPUT}
")

# if we are part of a larger project, they maybe use other Dart configuration
INCLUDE(../DartConfig.cmake OPTIONAL)
