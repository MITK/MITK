#! CMake function that runs a python build step from a setup.py script,
#! e.g. python setup.py build. This is used to build external python
#! libraries like numpy. The function takes the necessary build steps,
#! python executable etc. as arguments. The specific build command executed
#! by the runtime is passed as ARGN.
#!
#! params:
#!  proj - The name of the project
#!  step - Buildstep used to label the generated outputs, e.g. configure;build;install
#!  _python_executable - The python executable
#!  _bin_dir - The current binary from where commands are executed relative to it
#!  ARGN - Python command that will be run in the _bin_dir/proj-src folder
#!         e.g. setup.py build --someflags
#!
function(mitkFunctionExternalPythonBuildStep proj step _python_executable _bin_dir)

  # the specific python build command run by this step
  set(_command ${ARGN})

  message("Running ${proj} ${step}: ${_python_executable} ${_command}")

  set(_workdir "${_bin_dir}/ep/src/${proj}")
  set(_prefixdir "${_bin_dir}/ep/tmp")
  set(_python "${_python_executable}")

  foreach(_dir "${_workdir}" "${_prefixdir}")
    if(NOT EXISTS "${_dir}")
      message(SEND_ERROR "The directory ${_dir} does not exist")
    endif()
  endforeach()

  # escape spaces
  if(UNIX)
    STRING(REPLACE " " "\ " _workdir ${_workdir})
    STRING(REPLACE " " "\ " _prefixdir ${_prefixdir})
    STRING(REPLACE " " "\ " _python ${_python})
  endif()

  execute_process(
     COMMAND ${_python} ${_command}
     WORKING_DIRECTORY ${_workdir}
     RESULT_VARIABLE result
     #ERROR_QUIET
     ERROR_VARIABLE error
     OUTPUT_VARIABLE output
     #OUTPUT_QUIET
  )

  if(NOT ${result} EQUAL 0)
    message("Error in: ${proj}: ${error}")
    message("Output in: ${proj}: ${output}")
  endif()
endfunction()

