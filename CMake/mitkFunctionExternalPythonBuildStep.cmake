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

  message("Running ${proj} ${step}:${_python_executable} ${_command}")

  set(_workdir ${_bin_dir}/${proj}-src)
  set(_prefixdir ${_bin_dir}/${proj}-cmake)
  set(_python ${_python_executable})
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
     OUTPUT_VARIABLE output
     ERROR_VARIABLE error
  )
  set(output_file "${_prefixdir}/${proj}_${step}_step_output.txt")
  file(WRITE ${output_file} ${output})

  set(error_file "${_prefixdir}/${proj}_${step}_step_error.txt")
  file(WRITE ${error_file} ${error})

  if(NOT ${result} EQUAL 0)
    message(FATAL_ERROR "Error in: ${proj}: ${error}")
  endif()
endfunction()

