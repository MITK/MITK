# install_headers.cmake - copy the fetched Boost closure's headers into the
# install tree. Used for header-only provisioning (and to add the header-only
# closure on top of a compiled build), so no libraries need to be built to make
# Boost headers available.
#
#   cmake -DBOOST_SRC=<superproject> -DBOOST_INSTALL=<prefix> -P install_headers.cmake

cmake_minimum_required(VERSION 3.20)

foreach(_var BOOST_SRC BOOST_INSTALL)
  if(NOT DEFINED ${_var})
    message(FATAL_ERROR "install_headers.cmake: ${_var} is required")
  endif()
endforeach()

# Every fetched module keeps its headers under libs/<module>/include/boost;
# merge them into <prefix>/include/boost. Two globs cover flat and nested
# module paths (e.g. libs/algorithm, libs/numeric/conversion).
file(GLOB _include_dirs LIST_DIRECTORIES true
  "${BOOST_SRC}/libs/*/include"
  "${BOOST_SRC}/libs/*/*/include")

foreach(_inc IN LISTS _include_dirs)
  if(IS_DIRECTORY "${_inc}/boost")
    file(COPY "${_inc}/boost" DESTINATION "${BOOST_INSTALL}/include")
  endif()
endforeach()
