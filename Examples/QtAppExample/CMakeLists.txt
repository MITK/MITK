cmake_minimum_required(VERSION 2.8.5)

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
  message("${PROJECT_NAME} won't be built, missing dependency: ${result}")
else()

  add_executable(${PROJECT_NAME} Step1.cpp)

  # Set-up the build system to use the Qmitk module
  mitk_use_modules(TARGET ${PROJECT_NAME} MODULES Qmitk)

  # This is for subproject support inside the MITK source tree.
  # Ignore/remove the following code if you build an external application.
  set_property(TARGET ${PROJECT_NAME} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
  foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
    add_dependencies(${subproject} ${PROJECT_NAME})
  endforeach()

endif()
