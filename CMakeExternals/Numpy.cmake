
if( MITK_USE_Python )

  # Sanity checks
  if(DEFINED Numpy_DIR AND NOT EXISTS ${Numpy_DIR})
    message(FATAL_ERROR "Numpy_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if( NOT DEFINED Numpy_DIR )
    set(proj Numpy)
    set(proj_DEPENDENCIES )
    set(Numpy_DEPENDS ${proj})

    if(NOT MITK_USE_SYSTEM_PYTHON)
      list(APPEND proj_DEPENDENCIES Python)
    endif()

    #set(Numpy_URL "https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/numpy-1.6.1.tar.gz")
    #set(Numpy_MD5 "2bce18c08fc4fce461656f0f4dd9103e")
    set(Numpy_URL "https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/numpy-1.4.1.tar.gz")
    set(Numpy_MD5 "5c7b5349dc3161763f7f366ceb96516b")

    ExternalProject_Add(${proj}
      URL ${Numpy_URL}
      URL_MD5 ${Numpy_MD5}
      SOURCE_DIR ${proj}-src
      PREFIX ${proj}-cmake
      BUILD_IN_SOURCE 1
      INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install
      CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/${proj}-src/setup.py config
      BUILD_COMMAND   ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/${proj}-src/setup.py build --fcompiler=none
      INSTALL_COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/${proj}-src/setup.py install --prefix=${CMAKE_BINARY_DIR}/${proj}-install

      DEPENDS
        ${${proj}_DEPENDENCIES}
    )

    set(Numpy_DIR ${CMAKE_BINARY_DIR}/${proj}-install/lib/python2.7/site-packages)

   else()
     mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
   endif()
endif()

