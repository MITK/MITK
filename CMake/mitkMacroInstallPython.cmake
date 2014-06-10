
get_filename_component(InstallPython_cmake_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${InstallPython_cmake_dir}/GetPrerequisites.cmake")

function(MITK_FUNCTION_INSTALL_PYTHON_LIB _target_filename _target_lib _target_conf)

  install(FILES ${_target_lib}
                DESTINATION bin
                CONFIGURATIONS ${_target_conf})

  if(UNIX AND NOT APPLE)
     install(CODE "file(RPATH_REMOVE
                           FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_target_filename}\")")
   endif()
endfunction()

function(MITK_FUNCTION_INSTALL_PYTHON_MODULE _lib _conf)
  #MESSAGE("INSTALL : " ${_lib})
  get_filename_component(_target_filename "${_lib}" NAME)
  get_filename_component(_lib_path "${_lib}" PATH)

  MITK_FUNCTION_INSTALL_PYTHON_LIB("${_target_filename}" "${_lib}" "${_conf}")

  # install all dependencies to this python library
  get_prerequisites(${_lib} prereqs 1 1 bin "${_lib_path}")

  foreach(_dep ${prereqs})
    #MESSAGE("DEPENDENCIE: " ${_dep})
    get_filename_component(_target_filename "${_dep}" NAME)
    get_filename_component(_realpath "${_dep}" REALPATH)
    get_filename_component(_realname "${_realpath}" NAME)

    if(UNIX AND NOT APPLE)
      # if the realpath is different from the linked dependency
      # we got a symlink
      if( NOT("${_dep}" STREQUAL "${_realpath}"))
        # install symlink
        install(FILES ${_dep} DESTINATION bin)
      endif()
    endif()

    MITK_FUNCTION_INSTALL_PYTHON_LIB("${_realname}" "${_realpath}" "${_conf}")
  endforeach()
endfunction()

function(MITK_INSTALL_VTK_PYTHON)
  find_package(VTK REQUIRED)

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

  # install the python modules and loaders
  foreach(_target ${_VTK_PYTHON_TARGETS})
    # get target properties
    get_target_property(_target_lib_debug   "${_target}D" IMPORTED_LOCATION_DEBUG)
    get_filename_component(_filepath_debug "${_target_lib_debug}" PATH)
    get_target_property(_target_lib_release "${_target}D" IMPORTED_LOCATION_RELEASE)
    get_filename_component(_filepath_release "${_target_lib_release}" PATH)

    MESSAGE("${_filepath_debug}/${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(_target_lib_debug)
      MITK_FUNCTION_INSTALL_PYTHON_MODULE("${_filepath_debug}/${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}" "Debug")
    endif()

    if(_target_lib_release)
      MITK_FUNCTION_INSTALL_PYTHON_MODULE("${_filepath_release}/$${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}" "Release")
    endif()
  endforeach()

  # install vtk python
  install(DIRECTORY ${VTK_DIR}/Wrapping/Python/vtk
          DESTINATION bin/Python
          USE_SOURCE_PERMISSIONS
          COMPONENT Runtime)

endfunction()

function(MITK_INSTALL_CV_PYTHON)
  find_package(OpenCV REQUIRED)

  set(_build_type "Release")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_build_type "Debug")
  endif()

  MITK_FUNCTION_INSTALL_PYTHON_MODULE("${OpenCV_DIR}/lib/cv2${CMAKE_SHARED_LIBRARY_SUFFIX}" "${_build_type}")
endfunction()

macro(MITK_INSTALL_PYTHON)
  MITK_INSTALL_VTK_PYTHON()

  if(MITK_USE_OpenCV)
    MITK_INSTALL_CV_PYTHON()
  endif()
endmacro(MITK_INSTALL_PYTHON)

