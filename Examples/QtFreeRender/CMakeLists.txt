project(QtFreeRender)
find_package(MITK)

# Check prerequisites for this application.
# We need the Mitk module.
MITK_CHECK_MODULE(result Mitk)
if(result)
  message(SEND_ERROR "MITK module(s) \"${result}\" not available from the MITK build at ${MITK_DIR}")
endif()

usFunctionGenerateExecutableInit(init_src_file IDENTIFIER ${PROJECT_NAME})

add_executable(${PROJECT_NAME} QtFreeRender.cpp ${init_src_file})
mitk_use_modules(TARGET ${PROJECT_NAME} MODULES Mitk)

# subproject support
set_property(TARGET ${PROJECT_NAME} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} ${PROJECT_NAME})
endforeach()

