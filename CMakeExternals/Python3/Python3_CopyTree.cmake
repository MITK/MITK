#[[ Replaces <DST> with a copy of <SRC>.

      cmake -DSRC=<dir> -DDST=<dir> -P Python3_CopyTree.cmake

    Unlike cmake -E copy_directory, file(COPY) keeps symbolic links. The
    CPython archives link several names to one binary (python, python3 and
    python3.<minor>; libpython3.<minor>.so and its soname), so dereferencing
    them would install each of these binaries more than once. ]]

cmake_minimum_required(VERSION 3.28)

foreach(var SRC DST)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Python3_CopyTree.cmake: ${var} is required")
  endif()
endforeach()

file(REMOVE_RECURSE "${DST}")
file(COPY "${SRC}/" DESTINATION "${DST}")
