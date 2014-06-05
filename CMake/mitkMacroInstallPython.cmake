
macro(MITK_INSTALL_PYTHON)

find_package(VTK REQUIRED)

# Wrapping tools
#set(_VTK_PYTHON_TARGETS vtkWrappingPythonCore vtkWrappingTools)
set(_VTK_PYTHON_TARGETS )

# find all vtk python wrapped targets
foreach(_lib ${VTK_LIBRARIES})
  # exclude system libs
  if(${_lib} MATCHES "^vtk.+")
    # use only python wrapped modules
    if(TARGET ${_lib}PythonD)
      list(APPEND _VTK_PYTHON_TARGETS ${_lib}Python)
    endif()
  endif()
endforeach()

foreach(_target ${_VTK_PYTHON_TARGETS})

  # the python target library of the module
  set(_lib ${_target}D)

  # get target properties
  get_target_property(_target_lib_debug  ${_lib} IMPORTED_LOCATION_DEBUG)
  get_target_property(_target_lib_release ${_lib} IMPORTED_LOCATION_RELEASE)
  get_filename_component(_target_filename_debug "${_target_lib_debug}" NAME)
  get_filename_component(_target_filename_release "${_target_lib_release}" NAME)

  get_filename_component(_filepath_debug  ${_target_lib_debug} PATH)
  get_filename_component(_filepath_release  ${_target_lib_release} PATH)

  MESSAGE(${_lib})
  MESSAGE(${_filepath_debug})

   if(_filepath_debug)
     set(_python_mod_file_debug ${_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
     install(FILES "${_filepath_debug}/${_python_mod_file_debug}"
                   DESTINATION bin
                   CONFIGURATIONS Debug)
        install(CODE "file(RPATH_REMOVE
                              FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_python_mod_file_debug}\")")
        MESSAGE("${_filepath_debug}/${_python_mod_file_debug}")
   endif()

  #MESSAGE(${_target_lib_debug})

  if(_target_lib_debug)
    install(FILES ${_target_lib_debug}
                  DESTINATION bin
                  CONFIGURATIONS Debug)
  endif()

  if(_target_lib_release)
    install(FILES ${_target_lib_release}
                  DESTINATION bin
                  CONFIGURATIONS Release)
  endif()
  # remove rpath
  if(UNIX AND NOT APPLE)
     if(_target_filename_debug)
       install(CODE "file(RPATH_REMOVE
                             FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_target_filename_debug}\")")
     endif()

     if(_target_filename_release)
       install(CODE "file(RPATH_REMOVE
                               FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_target_filename_release}\")")
     endif()
  endif()
endforeach()

# install vtk python
install(DIRECTORY ${VTK_DIR}/Wrapping/Python/vtk
        DESTINATION bin
        USE_SOURCE_PERMISSIONS
        COMPONENT Runtime)


endmacro(MITK_INSTALL_PYTHON)

