set( VIDEOINPUT_LIB "" CACHE FILEPATH "Search for the videoInput.lib file" )
set( VIDEOINPUT_HEADER_DIR "" CACHE PATH "The path containing VideoInput.h" )

#message(STATUS "now in videoinput")

if(CMAKE_CL_64)
  set( STRMBASD_LIB "" CACHE FILEPATH "Search for the strmbasd.lib file. Provided by DirectShow, if not found, download Microsoft Platform SDK and go to the Samples_Multimedia_DirectShow_BaseClasses folder and compile the BaseClasses solution to generate the strmbasd.lib. Place it in the same folder as the videoInput.lib")
endif(CMAKE_CL_64)

set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:atlthunk" )
set( VIDEOINPUT_LIBS ${VIDEOINPUT_LIB} ${STRMBASD_LIB} )
include_directories( ${VIDEOINPUT_HEADER_DIR} )
add_definitions(-DMITK_USE_VIDEOINPUT_LIB)
list(APPEND ALL_LIBRARIES ${VIDEOINPUT_LIBS})
list(APPEND ALL_INCLUDE_DIRECTORIES ${VIDEOINPUT_HEADER_DIR})

