# Remove the Python DLL from the bin folder. It is automatically copied there
# when the dependencies are resolved. We want to use the Python DLL from
# our python folder instead. It is preloaded by the MitkPreloadPython module.

file(GLOB python_dll_candidates "${CMAKE_INSTALL_PREFIX}/bin/python3*.dll")
foreach(python_dll_candidate IN LISTS python_dll_candidates)
  get_filename_component(name "${python_dll_candidate}" NAME)
  if(name MATCHES "^python3[0-9]+\\.dll$")
    file(REMOVE "${python_dll_candidate}")
  endif()
endforeach()
