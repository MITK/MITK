
macro(MITK_INSTALL_PYTHON _python_libs _python_dirs)

  # install ITK python wrapping
  file(GLOB _libs "${ITK_DIR}/lib/_ITK*" )
  file(GLOB _py_files "${ITK_DIR}/lib/*.py" )
  file(GLOB _py_generators "${ITK_DIR}/Wrapping/Generators/Python/*.py")

  install(FILES ${_libs} DESTINATION bin/Python/itk)
  install(FILES ${_py_files} DESTINATION bin/bin/Python/itk)
  install(FILES ${_py_generators} DESTINATION bin/Python/itk)
  install(DIRECTORY "${ITK_DIR}/Wrapping/Generators/Python/Configuration"
          DESTINATION bin/Python/itk
          USE_SOURCE_PERMISSIONS
          COMPONENT Runtime)
  install(DIRECTORY "${ITK_DIR}/Wrapping/Generators/Python/itkExtras"
          DESTINATION bin/Python/itk
          USE_SOURCE_PERMISSIONS
          COMPONENT Runtime)

  foreach(lib ${_libs})
    get_filename_component(_libname "${lib}" NAME)
    list(APPEND _python_libs "Python/itk/${_libname}")

    if(UNIX AND NOT APPLE)
      install(CODE "file(RPATH_REMOVE
                           FILE \"\${CMAKE_INSTALL_PREFIX}/bin/Python/itk/${_libname}\")")
    endif()
  endforeach()

  list(APPEND _python_dirs "${ITK_DIR}/lib")

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

    install(FILES "${_filepath}/${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION bin)

    install(CODE "file(RPATH_REMOVE
                         FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}\")")

    list(APPEND _python_libs "${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endforeach()

  # install vtk python. This folder contains all *.py files for VTK module loading.
  install(DIRECTORY "${VTK_DIR}/Wrapping/Python/vtk"
          DESTINATION bin/Python
          USE_SOURCE_PERMISSIONS
          COMPONENT Runtime)

  list(APPEND _python_dirs "${VTK_DIR}/lib")

  # install the python runtime from the superbuild
  if(NOT MITK_USE_SYSTEM_PYTHON)
    list(APPEND _python_dirs "${Python_DIR}/lib")
    #ToDo: install python stuff
    install(DIRECTORY "${Python_DIR}/lib/python2.7"
            DESTINATION bin/Python/lib
            USE_SOURCE_PERMISSIONS
            COMPONENT Runtime)
    install(FILES "${Python_DIR}/include/python2.7/pyconfig.h" DESTINATION bin/Python/include/python2.7)
  endif()

  list(REMOVE_DUPLICATES _python_dirs)

endmacro()

