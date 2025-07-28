# Remove the Python shared library from the bin folder. It is automatically
# copied there when the dependencies are resolved. We want to use the Python
# shared library from our python folder instead. It is preloaded by the
# MitkPreloadPython module.

file(GLOB python_so_candidates "${CMAKE_INSTALL_PREFIX}/bin/libpython3.*")
foreach(python_so_candidate IN LISTS python_so_candidates)
  get_filename_component(name "${python_so_candidate}" NAME)
  if(name MATCHES "^libpython3\\.[0-9]+\\.so")
    file(REMOVE "${python_so_candidate}")
  endif()
endforeach()

