cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(QtAppExample)

# Check that MITK has been build with Qt support
if(NOT MITK_USE_QT)
  message(SEND_ERROR "MITK needs to be built with MITK_USE_QT set to ON")
endif()

# Create the executable - it needs the MitkQtWidgetsExt module
mitk_create_executable(${PROJECT_NAME} DEPENDS MitkQtWidgets)
