#  Variables set by this script
#  NUMPY_FOUND
#  NUMPY_INCLUDE_DIR

find_package(PackageHandleStandardArgs)

# numpy dir defined, own numpy deployed in python runtime
if(DEFINED Numpy_DIR AND EXISTS ${Numpy_DIR})
  if(EXISTS ${Numpy_DIR}/core/include)
    set(NUMPY_INCLUDE_DIR  ${Numpy_DIR}/core/include)
    MESSAGE("NUMPY: " ${NUMPY_INCLUDE_DIR})
  endif()
else() #numpy dir not defined
  execute_process (
     COMMAND ${PYTHON_EXECUTABLE} -c "import os; os.environ['DISTUTILS_USE_SDK']='1'; import numpy.distutils; print numpy.distutils.misc_util.get_numpy_include_dirs()[0]"
     OUTPUT_VARIABLE output
     OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(DEFINED output AND EXISTS ${output} )
    set (NUMPY_INCLUDE_DIR ${output})
  endif()
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Numpy DEFAULT_MSG NUMPY_INCLUDE_DIR)

MESSAGE("found: " ${NUMPY_FOUND})

MARK_AS_ADVANCED (
  NUMPY_INCLUDE_DIR
)

