#
# Packaging
#
if( MITK_PYTHON_EGG OR MITK_PYTHON_WHEEL )
  if( NOT MITK_PYTHON_USE_VIRTUALENV )
    message( STATUS "Not using MITK's virtualenv for distribution!\n
Using unknown versions of pip, setuptools and/or wheel packages/" )
  endif()

  set(bdist_setup "${MITK_Python_BINARY_DIR}/Packaging/setup.py")
  set(bdist_commands "")

  if( MITK_PYTHON_EGG )
    set(bdist_commands "bdist_egg")
  endif()

  if( MITK_PYTHON_WHEEL )
    set(bdist_commands ${bdist_commands} "bdist_wheel")
  endif()

  set( MITK_PYTHON_PLAT_NAME "" CACHE STRING
    "Optional value passed to setup.py with the '--plat-name' argument")

  if( NOT "${MITK_PYTHON_PLAT_NAME}" STREQUAL "" )
    set(bdist_commands ${bdist_commands} "--plat-name" "${MITK_PYTHON_PLAT_NAME}")
  endif()

  add_custom_target( dist.Python
    ${VIRTUAL_PYTHON_EXECUTABLE} ${bdist_setup} ${bdist_commands}
    WORKING_DIRECTORY ${MITK_Python_BINARY_DIR}
    DEPENDS ${SWIG_MODULE_pyMITK_REAL_NAME}
    COMMENT "Creating Python binary distribution" )
  set_property(TARGET dist.Python PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Wrapping")

  if( MITK_PYTHON_USE_VIRTUALENV )
    add_dependencies( dist.Python PythonVirtualEnv)
  endif()
  add_dependencies( dist dist.Python )
elseif()
  message( STATUS "Not creating dist.Python target since MITK_FORBID_DOWNLOADS is enabled" )
endif()
