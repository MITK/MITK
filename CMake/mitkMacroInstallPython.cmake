
macro(MITK_INSTALL_PYTHON _python_libs _python_dirs)
  if(UNIX)
    set(PYTHON_LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  else(WIN32)
    set(PYTHON_LIB_SUFFIX .pyd)
  endif()

  # SimpleITK
  if(MITK_USE_SimpleITK)
    install(FILES "${SimpleITK_DIR}/Wrapping/SimpleITK.py"
                  DESTINATION bin/Python/SimpleITK )
    install(FILES "${SimpleITK_DIR}/Wrapping/__init__.py"
                  DESTINATION bin/Python/SimpleITK )
    install(FILES "${SimpleITK_DIR}/Wrapping/_SimpleITK${PYTHON_LIB_SUFFIX}"
                  DESTINATION bin/Python/SimpleITK )
    if(UNIX AND NOT APPLE)
        install(CODE "file(RPATH_REMOVE
                    FILE \"\${CMAKE_INSTALL_PREFIX}/bin/Python/SimpleITK/_SimpleITK${PYTHON_LIB_SUFFIX}\")")
    endif()

    list(APPEND _python_libs "Python/SimpleITK/_SimpleITK${PYTHON_LIB_SUFFIX}")
    list(APPEND _python_dirs "${SimpleITK_DIR}/../SimpleITK-install/lib")
  endif()

  # install OpenCV python wrapping
  if(MITK_USE_OpenCV)
    list(APPEND _python_libs "cv2${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(FILES "${OpenCV_DIR}/lib/cv2${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION bin)

    if(UNIX AND NOT APPLE)
      install(CODE "file(RPATH_REMOVE
                         FILE \"\${CMAKE_INSTALL_PREFIX}/bin/cv2${CMAKE_SHARED_LIBRARY_SUFFIX}\")")
    endif()

    list(APPEND _python_dirs "${OpenCV_DIR}/lib")
  endif()

  # install VTK python wrapping
  find_package(VTK REQUIRED)

  set(_VTK_PYTHON_TARGETS )

  # find all vtk python wrapped targets
  foreach(_lib ${VTK_LIBRARIES})
    # exclude system libs
    if(${_lib} MATCHES "^vtk.+")
      # use only python wrapped modules ( targets end with PythonD )
      if(TARGET ${_lib}PythonD)
        list(APPEND _VTK_PYTHON_TARGETS ${_lib}Python)
      endif()
    endif()
  endforeach()

  # install the python modules and loaders
  foreach(_target ${_VTK_PYTHON_TARGETS})
    # get the properties of the python wrapped target
    if( CMAKE_BUILD_TYPE STREQUAL "Debug")
      get_target_property(_target_lib   "${_target}D" IMPORTED_LOCATION_DEBUG)
    else()
      get_target_property(_target_lib "${_target}D" IMPORTED_LOCATION_RELEASE)
    endif()
    get_filename_component(_filepath "${_target_lib}" PATH)

    install(FILES "${_filepath}/${_target}${PYTHON_LIB_SUFFIX}" DESTINATION bin)

    if(UNIX AND NOT APPLE )
      install(CODE "file(RPATH_REMOVE
                    FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_target}${PYTHON_LIB_SUFFIX}\")")
    endif()
    list(APPEND _python_libs "${_target}${PYTHON_LIB_SUFFIX}")
  endforeach()

  # install vtk python. This folder contains all *.py files for VTK module loading.
  install(DIRECTORY "${VTK_DIR}/Wrapping/Python/vtk"
          DESTINATION bin/Python
          USE_SOURCE_PERMISSIONS
          COMPONENT Runtime)

  list(APPEND _python_dirs "${VTK_DIR}/lib")

  # install the python runtime from the superbuild
  if(NOT MITK_USE_SYSTEM_PYTHON)
    if(UNIX)
      list(APPEND _python_dirs "${Python_DIR}/lib")
      # install python stuff
      install(DIRECTORY "${Python_DIR}/lib/python2.7"
            DESTINATION bin/Python/lib
            USE_SOURCE_PERMISSIONS
            COMPONENT Runtime)
      install(FILES "${Python_DIR}/include/python2.7/pyconfig.h" DESTINATION bin/Python/include/python2.7)
    else(WIN32)
      list(APPEND _python_dirs "${Python_DIR}/libs")
      list(APPEND _python_dirs "${Python_DIR}/bin")

      install(DIRECTORY "${Python_DIR}/Lib"
            DESTINATION bin/Python
            USE_SOURCE_PERMISSIONS
            COMPONENT Runtime)
      install(DIRECTORY "${Python_DIR}/include"
            DESTINATION bin/Python
            USE_SOURCE_PERMISSIONS
            COMPONENT Runtime)
    endif()
  endif()

  if(Numpy_DIR)
    install(DIRECTORY "${Numpy_DIR}"
            DESTINATION bin/Python/numpy
            USE_SOURCE_PERMISSIONS
            COMPONENT Runtime)
  endif()

  list(REMOVE_DUPLICATES _python_dirs)

endmacro()

