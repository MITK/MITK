cmake_minimum_required(VERSION 2.8.4)

project(GLEW)

set(${PROJECT_NAME}_MAJOR_VERSION 1)
set(${PROJECT_NAME}_MINOR_VERSION 10)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

find_package(OpenGL REQUIRED)

if(BUILD_SHARED_LIBS)
  add_definitions(-DGLEW_BUILD)
else()
  add_definitions(-DGLEW_STATIC)
endif()

add_library(glew src/glew.c)

set(include_dirs ${OPENGL_INCLUDE_DIR} ${GLEW_SOURCE_DIR}/include)
target_include_directories(glew
  PUBLIC "$<BUILD_INTERFACE:${include_dirs}>"
         "$<INSTALL_INTERFACE:include>"
)
target_link_libraries(glew PUBLIC ${OPENGL_gl_LIBRARY})
set_target_properties(glew PROPERTIES
                      SOVERSION ${${PROJECT_NAME}_VERSION})

set(${PROJECT_NAME}_LIBRARIES glew)

# Install support

install(TARGETS ${${PROJECT_NAME}_LIBRARIES} EXPORT ${PROJECT_NAME}_TARGETS
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/GL
  DESTINATION include
)

# Config files
configure_file(
  ${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  @ONLY
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}ConfigVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
)

export(EXPORT ${PROJECT_NAME}_TARGETS
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
)

set(config_package_location lib/cmake/${PROJECT_NAME})
install(EXPORT ${PROJECT_NAME}_TARGETS
  FILE ${PROJECT_NAME}Targets.cmake
  DESTINATION ${config_package_location}
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${config_package_location}
)
