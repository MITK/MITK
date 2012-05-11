project(mbilog)

set(MBILOG_HEADERS
  mbilog.h
  mbilogLoggingTypes.h
  mbilogTextDictionary.h
  mbilogExports.h
  mbilogLogMessage.h
  mbilogBackendBase.h
  mbilogTextBackendBase.h
  mbilogBackendCout.h
)

set(MBILOG_SOURCES
  mbilog.cpp
  mbilogLogMessage.cpp
  mbilogBackendCout.cpp
  mbilogBackendBase.cpp
  mbilogTextBackendBase.cpp
)

option(MBILOG_ENABLE_DEBUG_MESSAGES "Enable extra debug log output" OFF) 
mark_as_advanced(MBILOG_ENABLE_DEBUG_MESSAGES)

if(MBILOG_ENABLE_DEBUG_MESSAGES)
  set(_define_enable_debug "#define MBILOG_ENABLE_DEBUG")
endif(MBILOG_ENABLE_DEBUG_MESSAGES)

set(mbilog_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mbilogConfig.cmake.in"
"${CMAKE_CURRENT_BINARY_DIR}/mbilogConfig.cmake" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mbilogConfig.h.in"
"${CMAKE_CURRENT_BINARY_DIR}/mbilogConfig.h" @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(mbilog ${MBILOG_HEADERS} ${MBILOG_SOURCES})


# mbilog is independent of mitk, and cant use mitk macros i.e. MITK_CREATE_MODULE_CONF( mbilog )
# configuring happens through ../CMakeList.txt and mbilogConfig.cmake.in