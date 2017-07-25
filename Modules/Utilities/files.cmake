file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  StringUtilities.cpp
  ThreadUtilities.cpp
  ThreadPoolUtilities.cpp
  internal/Invoker.cpp
  PathUtilities.cpp
)

set(MOC_H_FILES
  src/internal/Invoker.h
)
