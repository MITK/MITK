project(QuickRender)
find_package(MITK)

# Check prerequisites for this application.
# We need the Mitk module.
MITK_CHECK_MODULE(result Mitk QmlMitk)
if(result)
  message(SEND_ERROR "MITK module(s) \"${result}\" not available from the MITK build at ${MITK_DIR}")
endif()

# Set-up the build system to use the Mitk module
MITK_USE_MODULE(Mitk QmlMitk)

include_directories(${ALL_INCLUDE_DIRECTORIES})
link_directories(${ALL_LIBRARY_DIRS})

# handle qt 5 classes
qt5_add_resources(rcc_SRC QuickRender.qrc)

#add_executable(${PROJECT_NAME} QuickRender.cpp ${mocs_SRC})
add_executable(${PROJECT_NAME} main.cpp ${moc_SRC} ${rcc_SRC})
target_link_libraries(${PROJECT_NAME} ${ALL_LIBRARIES})

# subproject support
set_property(TARGET ${PROJECT_NAME} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} ${PROJECT_NAME})
endforeach()

