project(QtFreeRender)
find_package(MITK)

find_package(ITK)
include(${ITK_USE_FILE})
find_package(VTK)
include(${VTK_USE_FILE})

include_directories(${MITK_INCLUDE_DIRS})
include_directories(${MODULES_CONF_DIRS})

link_directories(${MITK_LINK_DIRECTORIES})
add_executable(${PROJECT_NAME} QtFreeRender.cpp)
target_link_libraries(${PROJECT_NAME} ${MITK_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} )

# subproject support
set_property(TARGET ${PROJECT_NAME} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} ${PROJECT_NAME})
endforeach()

