project(FunctionalityGenerator)

set(GENERATE_FUNCTIONALITY OFF CACHE BOOL "Turn this on for final generation of functionality")
set(FUNCTIONALITY_NAME "" CACHE STRING "Name of the new functionality")
find_file(FUNCTIONALITY_TEMPLATE QmitkFunctionalityTemplate 
  ${PROJECT_SOURCE_DIR}/../../../CMake
  DOC "Path to the functionality template") 

set(FUNCTIONALITY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${FUNCTIONALITY_NAME})

if(GENERATE_FUNCTIONALITY)
  if(FUNCTIONALITY_TEMPLATE)
    message("Generating ${FUNCTIONALITY_NAME} from template ${FUNCTIONALITY_TEMPLATE} in directory ${FUNCTIONALITY_DIR} ...")
    file(GLOB_RECURSE TEMPLATE_FILES RELATIVE ${FUNCTIONALITY_TEMPLATE} ${FUNCTIONALITY_TEMPLATE}/*)
    foreach(F ${TEMPLATE_FILES})
      if(NOT F MATCHES ".svn/")
      string(REPLACE QmitkFunctionalityTemplate ${FUNCTIONALITY_NAME} NEW_RELPATH ${F})
      message(STATUS "Generating ${FUNCTIONALITY_DIR}/${NEW_RELPATH}")
      if(EXISTS ${FUNCTIONALITY_DIR}/${NEW_RELPATH})
        message(FATAL_ERROR "Won't overwrite existing functionality. Please remove all files from ${FUNCTIONALITY_DIR}!") 
      endif(EXISTS ${FUNCTIONALITY_DIR}/${NEW_RELPATH})
      configure_file(${FUNCTIONALITY_TEMPLATE}/${F} ${FUNCTIONALITY_DIR}/${NEW_RELPATH} @ONLY)
      endif(NOT F MATCHES ".svn/")
    endforeach(F)
    set(GENERATE_FUNCTIONALITY OFF CACHE BOOL "Turn this on for final generation of functionality" FORCE)
    message("New Functionality \"${FUNCTIONALITY_NAME}\" has been generated in \"${FUNCTIONALITY_DIR}\"."
            "Add this directory to QFunctionalities to add it to the MainApp!")
  endif(FUNCTIONALITY_TEMPLATE)
endif(GENERATE_FUNCTIONALITY)

