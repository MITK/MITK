OPTION(USE_KITWARE_DARTBOARD "Use the Kitware Dartboard for Build Submissions." OFF)


# never use kitware dashboard, since it doesn't work at the moment (10.10.2007)
SET (DROP_METHOD xmlrpc)
SET (DROP_SITE "http://makalu:8081")
SET (DROP_LOCATION "MITK-public")
SET (NIGHTLY_START_TIME "21:00:00 CET")	
SET (DART_TESTING_TIMEOUT 900)

IF(FALSE)

# configure dart for submission to kitware
IF(USE_KITWARE_DARTBOARD)
	#
	# Dart server to submit results (used by client)
	#
	SET (DROP_METHOD http)
	SET (DROP_SITE "public.kitware.com")
	SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
	SET (DROP_SITE_USER "ftpuser")
	SET (DROP_SITE_PASSWORD "public")
	SET (TRIGGER_SITE "http://public.kitware.com/cgi-bin/Submit-MITK-TestingResults.pl")
	SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/MITK-rollup-dashboard.sh")
        SET (NIGHTLY_START_TIME "21:00:00 EST")	
	#
        # SET (CVS_WEB_URL "http://mbi-srv/docs/programming/cvs/viewcvs/cgi/viewcvs.cgi/ip%2b%2b/")
	# SET (DOXYGEN_URL "http://mbi-srv.inet.dkfz-heidelberg.de/docs/image_processing/mitk/doxygen/" )
        
ENDIF(USE_KITWARE_DARTBOARD)
ENDIF(FALSE)

IF(EXISTS ${MBI_DART_WORKDIR}/notes.txt)
  MAKE_DIRECTORY(${MBI_DART_WORKDIR}/bin/Testing/Notes)
  CONFIGURE_FILE(${MBI_DART_WORKDIR}/notes.txt ${MBI_DART_WORKDIR}/bin/Testing/Notes/notes.txt COPYONLY)
ENDIF(EXISTS ${MBI_DART_WORKDIR}/notes.txt)
IF(EXISTS ${ITK_DIR}/ITKConfig.cmake)
  INCLUDE(${ITK_DIR}/ITKConfig.cmake)
ENDIF(EXISTS ${ITK_DIR}/ITKConfig.cmake)
IF(EXISTS ${VTK_DIR}/VTKConfig.cmake)
  INCLUDE(${VTK_DIR}/VTKConfig.cmake)
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
