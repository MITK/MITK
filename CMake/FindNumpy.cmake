#  Variables set by this script
#  NUMPY_FOUND
#  NUMPY_INCLUDE_DIR
find_package(PackageHandleStandardArgs)

if(DEFINED Numpy_DIR AND EXISTS ${Numpy_DIR})
  set(NUMPY_INCLUDE_DIR ${Numpy_DIR}/core/include)
  MESSAGE("NUMPY_DIR: " ${Numpy_DIR})
  MESSAGE("NUMPY_INCLUDE_DIR: " ${NUMPY_INCLUDE_DIR})
  MESSAGE("PYTHON_EXECUTABLE: " ${PYTHON_EXECUTABLE})
else()
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
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Numpy DEFAULT_MSG NUMPY_INCLUDE_DIR)

MARK_AS_ADVANCED (
  NUMPY_INCLUDE_DIR
)
