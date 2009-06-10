PROJECT(MBILOG)

SET(MBILOG_HEADERS
  mbilog.h
)
  
SET(MBILOG_SOURCES
  mbilog.cpp
)

ADD_LIBRARY(mbilog ${MBILOG_HEADERS} ${MBILOG_SOURCES})

OPTION(MBILOG_ENABLE_DEBUG_MESSAGES "Enable extra debug log output" OFF) 
MARK_AS_ADVANCED(MBILOG_ENABLE_DEBUG_MESSAGES)

IF(MBILOG_ENABLE_DEBUG_MESSAGES)
  SET(MBILOG_DEFINITIONS "-DMBILOG_ENABLE_DEBUG")
ENDIF(MBILOG_ENABLE_DEBUG_MESSAGES)

SET(mbilog_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mbilogConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/mbilogConfig.cmake" @ONLY)

# mbilog is independent of mitk, and cant use mitk macros i.e. MITK_CREATE_MODULE_CONF( mbilog )
# configuring happens through ../CMakeList.txt and mbilogConfig.cmake.in
