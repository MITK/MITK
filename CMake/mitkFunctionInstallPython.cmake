#! This CMake macro installs the python runtime with
#! all python related libraries and toolkits. Py files are
#! searched one by one with a globbing expression to generate a list used
#! by the mac and NSIS installer.
#!
#! params:
#!  _python_libs Returns a list of the installed libraries. Used to fixup the bundle.
#! _app_bundle App bundle name in case of a Apple bundle.
#!
function(mitkFunctionInstallPython _python_libs_out _search_path_out _app_bundle)

  # install the python runtime from the superbuild
  if(NOT MITK_USE_SYSTEM_PYTHON)

    # find package sets the python version numbers
    find_package(PythonLibs REQUIRED)
    find_package(PythonInterp REQUIRED)

    set(_python_libs )
    set(_search_path )

    # set the destination bundle
    set(_destination bin)
    if(APPLE)
      set(_destination ${_app_bundle})
    endif()

    if(UNIX)
      list(APPEND _python_dirs "${MITK_EXTERNAL_PROJECT_PREFIX}/lib")
      set(PYTHON_LIB_SUFFIX ".so")
      set(_py_include_postfix python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/) # add the slash here so windows and unix will work with same code
      set(_python_runtime_dir python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/)
    else() #WIN32
      list(APPEND _python_dirs "${MITK_EXTERNAL_PROJECT_PREFIX}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/Lib")
      #list(APPEND _python_dirs "${Python_DIR}/bin")
      set(PYTHON_LIB_SUFFIX .pyd)
      set(_py_include_postfix )
      set(_python_runtime_dir )
      set(_search_path "${MITK_EXTERNAL_PROJECT_PREFIX}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/bin")
    endif()

    file(GLOB_RECURSE item RELATIVE "${_python_dirs}/${_python_runtime_dir}" "${_python_dirs}/${_python_runtime_dir}*")
    foreach(f ${item})
      get_filename_component(_filepath "${f}" PATH)
      #message("f: " ${f})
      #message("filepath: " ${_filepath})
      install(FILES "${_python_dirs}/${_python_runtime_dir}${f}" DESTINATION "${_destination}/Python/lib/${_python_runtime_dir}${_filepath}")

      #MESSAGE("in: " "${_python_dirs}/${_python_runtime_dir}${f}")
      #MESSAGE("out: " "${_destination}/Python/lib/${_python_runtime_dir}${_filepath}")
      if(${f} MATCHES "(${PYTHON_LIB_SUFFIX})$")
        #MESSAGE("F: " "Python/lib/${_python_runtime_dir}${f}")
        list(APPEND _python_libs "Python/lib/${_python_runtime_dir}${f}")
        if(UNIX AND NOT APPLE)
          install(CODE "file(RPATH_REMOVE
          FILE \"\${CMAKE_INSTALL_PREFIX}/bin/Python/lib/${_python_runtime_dir}${f}\")")
        endif()
      endif()
    endforeach()

    # config will by read out at runtime
    install(FILES "${PYTHON_INCLUDE_DIR}/pyconfig.h" DESTINATION ${_destination}/Python/include/${_py_include_postfix})

    set(${_python_libs_out} ${_python_libs} PARENT_SCOPE)
    set(${_search_path_out} ${_search_path} PARENT_SCOPE)
  endif()
endfunction()

