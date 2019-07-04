#  Variables set by this script
#  NUMPY_FOUND
#  NUMPY_INCLUDE_DIR
find_package(PackageHandleStandardArgs)

set(_python ${Python3_EXECUTABLE})
if(UNIX)
  STRING(REPLACE " " "\\ " _python ${Python3_EXECUTABLE})
endif()

execute_process (
  COMMAND ${_python} -c "import numpy.distutils; print( numpy.distutils.misc_util.get_numpy_include_dirs()[0])"
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
  RESULT_VARIABLE result
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if(DEFINED output AND EXISTS ${output} )
  set (NUMPY_INCLUDE_DIR ${output})
else()
  message(STATUS "python executable >${_python}<")
  message(STATUS "numpy search output >${output}<")
  message(STATUS "numpy search error >${error}<")
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Numpy DEFAULT_MSG NUMPY_INCLUDE_DIR)

MARK_AS_ADVANCED (
  NUMPY_INCLUDE_DIR
)
