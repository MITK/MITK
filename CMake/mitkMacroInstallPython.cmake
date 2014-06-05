


function(MITK_FUNCTION_INSTALL_PYTHON_LIB _target_filename _target_lib _target_conf)

  install(FILES ${_target_lib}
                DESTINATION bin
                CONFIGURATIONS ${_target_conf})

  if(UNIX AND NOT APPLE)
     install(CODE "file(RPATH_REMOVE
                           FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_target_filename}\")")
   endif()
endfunction()

macro(MITK_INSTALL_PYTHON)

  find_package(VTK REQUIRED)

  # all libs with a python loader module
  set(_VTK_PYTHON_TARGETS )
  # Wrapping tools and dependencies
  set(_VTK_PYTHON_WRAPPED_MODULES vtkWrappingPythonCore vtkWrappingTools)
  # additional utilities
  set(_VTK_UTILITY_DEPS
    verdict
    vtksqlite
    vtkproj4
    vtkhdf5
    vtkhdf5_hl
    vtklibxml2
    vtkgl2ps
    vtkNetCDF_cxx
    vtkNetCDF
    vtkexoIIc
    vtkoggtheora )

  foreach(t ${_VTK_UTILITY_DEPS})
    list(APPEND _VTK_PYTHON_WRAPPED_MODULES ${t})
  endforeach()

  # find all vtk python wrapped targets
  foreach(_lib ${VTK_LIBRARIES})
    # exclude system libs
    if(${_lib} MATCHES "^vtk.+")
      # use only python wrapped modules
      if(TARGET ${_lib}PythonD)
        list(APPEND _VTK_PYTHON_TARGETS ${_lib}Python)
        list(APPEND _VTK_PYTHON_WRAPPED_MODULES ${_lib})
      endif()
    endif()
  endforeach()

  # install all targets that are python wrapped
  foreach(_lib ${_VTK_PYTHON_WRAPPED_MODULES})
    get_target_property(_target_lib_debug   ${_lib} IMPORTED_LOCATION_DEBUG)
    get_target_property(_target_lib_release ${_lib} IMPORTED_LOCATION_RELEASE)
    get_filename_component(_target_filename_debug   "${_target_lib_debug}" NAME)
    get_filename_component(_target_filename_release "${_target_lib_release}" NAME)

    if(_target_lib_debug)
      MITK_FUNCTION_INSTALL_PYTHON_LIB(${_target_filename_debug} ${_target_lib_debug} "Debug")
    endif()

    if(_target_lib_release)
      MITK_FUNCTION_INSTALL_PYTHON_LIB(${_target_filename_release} ${_target_lib_release} "Release")
    endif()
  endforeach()

  # install the python modules and loaders
  foreach(_target ${_VTK_PYTHON_TARGETS})

    set(_lib ${_target}D)

    # get target properties
    get_target_property(_target_lib_debug   ${_lib} IMPORTED_LOCATION_DEBUG)
    get_target_property(_target_lib_release ${_lib} IMPORTED_LOCATION_RELEASE)
    get_filename_component(_target_filename_debug   "${_target_lib_debug}" NAME)
    get_filename_component(_target_filename_release "${_target_lib_release}" NAME)

    get_filename_component(_filepath_debug   ${_target_lib_debug} PATH)
    get_filename_component(_filepath_release ${_target_lib_release} PATH)

    #MESSAGE(${_lib})
    #MESSAGE(${_filepath_debug})

    if(_target_lib_debug)
      MITK_FUNCTION_INSTALL_PYTHON_LIB(${_target_filename_debug} ${_target_lib_debug} "Debug")
    endif()

    if(_target_lib_release)
      MITK_FUNCTION_INSTALL_PYTHON_LIB(${_target_filename_release} ${_target_lib_release} "Release")
    endif()

    # if the target is a wrapped module install the python module loader
    # and make sure the vtk module is installed
    if(_filepath_debug)
      set(_python_mod_file ${_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
      MITK_FUNCTION_INSTALL_PYTHON_LIB( ${_python_mod_file} "${_filepath_debug}/${_python_mod_file}" "Debug")
    endif()

    if(_filepath_release)
      set(_python_mod_file ${_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
      MITK_FUNCTION_INSTALL_PYTHON_LIB(${_python_mod_file} "${_filepath_release}/${_python_mod_file}" "Release")
    endif()

  endforeach()

  # install vtk python
  install(DIRECTORY ${VTK_DIR}/Wrapping/Python/vtk
          DESTINATION bin
          USE_SOURCE_PERMISSIONS
          COMPONENT Runtime)


endmacro(MITK_INSTALL_PYTHON)

