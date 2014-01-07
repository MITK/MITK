cmake_minimum_required(VERSION 2.8.5)

project(QtAppExample)

find_package(MITK REQUIRED)

# Check that MITK has been build with Qt support
if(NOT MITK_USE_QT)
  message(SEND_ERROR "MITK needs to be built with MITK_USE_QT set to ON")
endif()

# Create the executable - it needs the Qmitk module
mitk_create_executable(${PROJECT_NAME} DEPENDS Qmitk)
