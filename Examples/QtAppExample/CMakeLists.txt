cmake_minimum_required(VERSION 2.8.4)

project(QtAppExample)

find_package(MITK REQUIRED)

# Check that MITK has been build with Qt support
if(NOT MITK_USE_QT)
  message(SEND_ERROR "MITK needs to be built with MITK_USE_QT set to ON")
endif()

# Check prerequisites for this application.
# We need the Qmitk module.
MITK_CHECK_MODULE(result Qmitk)
if(result)
  message(SEND_ERROR "MITK module(s) \"${result}\" not available from the MITK build at ${MITK_DIR}")
endif()

# Set-up the build system to use the Qmitk module
MITK_USE_MODULE(Qmitk)
include_directories(${ALL_INCLUDE_DIRECTORIES})
link_directories(${ALL_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} Step1.cpp)
target_link_libraries(${PROJECT_NAME} ${ALL_LIBRARIES})


# This is for subproject support inside the MITK source tree.
# Ignore/remove the following code if you build an external application.
set_property(TARGET ${PROJECT_NAME} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} ${PROJECT_NAME})
endforeach()

