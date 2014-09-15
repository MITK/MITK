#
#  Windows python 2.7.3 patches for the CMake build system
#  https://github.com/davidsansome/python-cmake-buildsystem/tree/master/cmake/patches-win32
#
set(in ${CMAKE_CURRENT_LIST_DIR}/msvc9compiler.py.patched)
set(out ${PYTHON_SOURCE_DIR}/Lib/distutils/msvc9compiler.py)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${in} ${out})
