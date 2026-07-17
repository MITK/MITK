#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

#[[ Sanity checks ]]
if(DEFINED Boost_ROOT AND NOT EXISTS ${Boost_ROOT})
  message(FATAL_ERROR "Boost_ROOT variable is defined but corresponds to non-existing directory")
endif()

string(REPLACE "^^" ";" MITK_USE_Boost_LIBRARIES "${MITK_USE_Boost_LIBRARIES}")

set(proj Boost)
set(proj_DEPENDENCIES )
set(Boost_DEPENDS ${proj})

#[[ Provisioning fetches the internal-dependency closure of
    MITK_USE_Boost_LIBRARIES from a committed map (CMakeExternals/Boost/) and
    builds exactly that set. When MITK builds its standalone Python, the closure
    is fetched in parallel; otherwise a sequential pure-CMake fetch is used.
    Using Python only adds a build-order dependency on Python3, which is
    registered before Boost in ExternalProjectList.cmake for that reason. ]]
if(MITK_USE_Python3)
  if(WIN32)
    set(boost_python "${Python3_ROOT_DIR}/python.exe")
  else()
    set(boost_python "${Python3_ROOT_DIR}/bin/python3")
  endif()
  list(APPEND proj_DEPENDENCIES Python3)
else()
  set(boost_python "")
endif()

if(NOT DEFINED Boost_ROOT AND NOT MITK_USE_SYSTEM_Boost)

  set(Boost_ROOT ${ep_prefix})
  set(Boost_DIR "${Boost_ROOT}/lib/cmake/Boost-1.91.0")

  if(WIN32)
    set(BOOST_LIBRARYDIR "${Boost_ROOT}/lib")
  endif()

  find_package(Git REQUIRED)

  set(provision_dir "${CMAKE_CURRENT_LIST_DIR}/Boost")

  #[[ Transport the library set through single -D arguments using the
      ExternalProject list separator; the nested build and BoostProvision.cmake
      decode it back into a list. ]]
  string(REPLACE ";" "^^" boost_libraries "${MITK_USE_Boost_LIBRARIES}")

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    #[[ When upgrading Boost, bump this tag AND regenerate the dependency map:
        clone the new tag with --recurse-submodules, then run
        Boost/generate_boost_map.py <checkout> Boost/boost-deps.cmake ]]
    GIT_TAG boost-1.91.0
    GIT_SHALLOW 1
    GIT_SUBMODULES "" #[[ Superproject only; the closure is fetched selectively below. ]]
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR_PLATFORM ${gen_platform}
    CMAKE_ARGS
      ${ep_common_args}
      -DBOOST_INSTALL_LAYOUT:STRING=system
      -DBOOST_INCLUDE_LIBRARIES:STRING=${boost_libraries}
    CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
      -DBUILD_TESTING:BOOL=OFF
    CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
    DEPENDS ${proj_DEPENDENCIES}
  )

  #[[ After clone, before configure: fetch the CMake build infrastructure
      (superproject submodules) and the sparse dependency closure of the
      requested libraries. ]]
  ExternalProject_Add_Step(${proj} provision
    COMMAND ${GIT_EXECUTABLE} submodule update --init --depth 1
            tools/cmake tools/build tools/boost_install
    COMMAND ${CMAKE_COMMAND}
            -DBOOST_SRC=<SOURCE_DIR>
            -DBOOST_LIBRARIES=${boost_libraries}
            -DBOOST_MAP=${provision_dir}/boost-deps.cmake
            -DFETCH_ONE=${provision_dir}/fetch_one.cmake
            -DFETCH_PY=${provision_dir}/fetch_boost.py
            -DBOOST_PYTHON=${boost_python}
            -P ${provision_dir}/BoostProvision.cmake
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
  )

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
