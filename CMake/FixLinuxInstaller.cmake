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

# Fix pyMITK package by renaming pyMITK.py to __init__.py.

file(GLOB lib_dirs "${CMAKE_INSTALL_PREFIX}/python/lib/python3.*")
foreach(lib_dir IN LISTS lib_dirs)
  if(IS_DIRECTORY "${lib_dir}")
    set(pymitk_dir "${lib_dir}/site-packages/pyMITK")
    if(EXISTS "${pymitk_dir}/pyMITK.py")
      file(RENAME "${pymitk_dir}/pyMITK.py" "${pymitk_dir}/__init__.py")
      break()
    endif()
  endif()
endforeach()
