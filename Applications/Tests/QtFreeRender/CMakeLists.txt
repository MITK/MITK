project(QtFreeRender)
find_package(MITK)
include_directories(${MITK_INCLUDE_DIRS})
link_directories(${MITK_LINK_DIRECTORIES})
add_executable(QtFreeRender QtFreeRender.cpp)
target_link_libraries(QtFreeRender ${MITK_LIBRARIES} ITKCommon.lib)

