
if( MITK_USE_Python )
  if( NOT Numpy_DIR )
    set(proj Numpy)
    set(proj_DEPENDENCIES )
    set(Numpy_DEPENDS ${proj})

    if(NOT MITK_USE_SYSTEM_PYTHON)
      list(APPEND proj_DEPENDENCIES Python)
    endif()

    ExternalProject_Add(${proj}
      URL "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/numpy-1.4.1.tar.gz"
      SOURCE_DIR ${proj}-src
      PREFIX ${proj}-cmake
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/${proj}-src/setup.py config
      BUILD_COMMAND   ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/${proj}-src/setup.py build
      INSTALL_COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/${proj}-src/setup.py install

      DEPENDS
        ${${proj}_DEPENDENCIES}
    )

    set(Numpy_DIR ${Python_Dir}/lib/python2.7/site-packages/numpy)

   else()
     mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
   endif()
endif()

