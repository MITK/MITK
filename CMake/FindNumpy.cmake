#  Variables set by this script
#  NUMPY_FOUND
#  NUMPY_INCLUDE_DIR
find_package(PackageHandleStandardArgs)

# search for the runtime if it's not already set
if( NOT DEFINED PYTHON_EXECUTABLE )
  FIND_PACKAGE(PythonLibs REQUIRED)
  FIND_PACKAGE(PythonInterp REQUIRED)
endif()

set(_python ${PYTHON_EXECUTABLE})
if(UNIX)
  STRING(REPLACE " " "\ " _python ${PYTHON_EXECUTABLE})
endif()

execute_process (
  COMMAND ${_python} -c "import os; os.environ['DISTUTILS_USE_SDK']='1'; import numpy.distutils; print numpy.distutils.misc_util.get_numpy_include_dirs()[0]"
  OUTPUT_VARIABLE output
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if(DEFINED output AND EXISTS ${output} )
  set (NUMPY_INCLUDE_DIR ${output})
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Numpy DEFAULT_MSG NUMPY_INCLUDE_DIR)

MARK_AS_ADVANCED (
  NUMPY_INCLUDE_DIR
)
