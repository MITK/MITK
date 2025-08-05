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
