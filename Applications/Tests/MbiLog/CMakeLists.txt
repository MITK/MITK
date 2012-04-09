project(MbiLogExample)
find_package(MITK)
include_directories(${MITK_INCLUDE_DIRS})
link_directories(${MITK_LINK_DIRECTORIES})
add_executable(MitkLogExample MitkLogExample.cpp)
target_link_libraries(MitkLogExample ${MITK_LIBRARIES})

