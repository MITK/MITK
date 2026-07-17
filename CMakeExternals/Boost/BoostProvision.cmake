# BoostProvision.cmake - resolve the dependency closure of a requested Boost
# library set from the committed map and fetch exactly that closure, sparse and
# shallow, into a superproject checkout.
#
# Dispatches the fetch: if BOOST_PYTHON points at a Python interpreter it runs
# fetch_boost.py (parallel); otherwise it loops fetch_one.cmake sequentially.
# Both paths call the same fetch_one.cmake, so the git logic lives in one place.
#
# Invoke (cmake -P):
#   cmake -DBOOST_SRC=<superproject> -DBOOST_LIBRARIES=<a;b;c> \
#         -DBOOST_MAP=<boost-deps.cmake> -DFETCH_ONE=<fetch_one.cmake> \
#         [-DBOOST_PYTHON=<python>] [-DFETCH_PY=<fetch_boost.py>] [-DJOBS=8] \
#         -P BoostProvision.cmake

cmake_minimum_required(VERSION 3.20)

foreach(_var BOOST_SRC BOOST_LIBRARIES BOOST_MAP FETCH_ONE)
  if(NOT DEFINED ${_var})
    message(FATAL_ERROR "BoostProvision.cmake: ${_var} is required")
  endif()
endforeach()
if(NOT DEFINED JOBS)
  set(JOBS 8)
endif()

# Undo the ExternalProject list separator so a multi-library request passed
# through a single -D argument arrives here as a real CMake list.
string(REPLACE "^^" ";" BOOST_LIBRARIES "${BOOST_LIBRARIES}")

include("${BOOST_MAP}")

# config and headers are always needed (depinst treats them as essentials).
set(_worklist ${BOOST_LIBRARIES} config headers)
set(_closure "")
while(_worklist)
  list(POP_FRONT _worklist _m)
  if(_m IN_LIST _closure)
    continue()
  endif()
  list(APPEND _closure "${_m}")
  string(REPLACE "/" "__" _mv "${_m}")
  if(DEFINED BOOST_MODULE_DEPS_${_mv})
    list(APPEND _worklist ${BOOST_MODULE_DEPS_${_mv}})
  endif()
endwhile()
list(SORT _closure)
list(LENGTH _closure _n)
message(STATUS "Boost: provisioning ${_n} modules for [${BOOST_LIBRARIES}]")

if(DEFINED BOOST_PYTHON AND NOT "${BOOST_PYTHON}" STREQUAL "")
  set(_list "")
  foreach(_m IN LISTS _closure)
    string(REPLACE "/" "__" _mv "${_m}")
    string(APPEND _list "${_m}|${BOOST_MODULE_URL_${_mv}}|${BOOST_MODULE_SHA_${_mv}}\n")
  endforeach()
  set(_listfile "${BOOST_SRC}/.boost_closure.txt")
  file(WRITE "${_listfile}" "${_list}")
  message(STATUS "Boost: parallel fetch (${JOBS} jobs) via ${BOOST_PYTHON}")
  execute_process(
    COMMAND "${BOOST_PYTHON}" "${FETCH_PY}" "${BOOST_SRC}" "${_listfile}"
            "${FETCH_ONE}" "${CMAKE_COMMAND}" "${JOBS}"
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "Boost: parallel fetch failed (exit ${_rc})")
  endif()
else()
  message(STATUS "Boost: sequential fetch (no Python)")
  foreach(_m IN LISTS _closure)
    string(REPLACE "/" "__" _mv "${_m}")
    execute_process(
      COMMAND "${CMAKE_COMMAND}"
              -DBOOST_SRC=${BOOST_SRC} -DMODULE=${_m}
              -DBOOST_URL=${BOOST_MODULE_URL_${_mv}}
              -DBOOST_SHA=${BOOST_MODULE_SHA_${_mv}}
              -P "${FETCH_ONE}"
      RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
      message(FATAL_ERROR "Boost: fetch failed for ${_m} (exit ${_rc})")
    endif()
  endforeach()
endif()
