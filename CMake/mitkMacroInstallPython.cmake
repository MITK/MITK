
macro(MITK_INSTALL_PYTHON)

  find_package(VTK REQUIRED)

foreach(_lib ${VTK_LIBRARIES})
  # exclude system libs
  if(${_lib} MATCHES "^vtk.+")
    # use only python wrapped targets
    if(TARGET ${_lib}PythonD)

      # get target properties
      get_target_property(_target_lib_debug  ${_lib}PythonD IMPORTED_LOCATION_DEBUG)
      get_target_property(_target_lib_release ${_lib}PythonD IMPORTED_LOCATION_RELEASE)
      get_filename_component(_target_filename_debug "${_target_lib_debug}" NAME)
      get_filename_component(_target_filename_release "${_target_lib_release}" NAME)

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
    endif()
  endif()
endforeach()

endmacro(MITK_INSTALL_PYTHON)

