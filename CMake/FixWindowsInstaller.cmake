# Fix pyMITK package by renaming pyMITK.py to __init__.py.

set(pymitk_dir "${CMAKE_INSTALL_PREFIX}/python/Lib/site-packages/pyMITK")
if(EXISTS "${pymitk_dir}/pyMITK.py")
  file(RENAME "${pymitk_dir}/pyMITK.py" "${pymitk_dir}/__init__.py")
endif()
