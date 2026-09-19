#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

#[[ Sanity checks ]]
if(DEFINED Boost_ROOT AND NOT EXISTS ${Boost_ROOT})
  message(FATAL_ERROR "Boost_ROOT variable is defined but corresponds to non-existing directory")
endif()

string(REPLACE "^^" ";" MITK_USE_Boost_HEADER_LIBRARIES "${MITK_USE_Boost_HEADER_LIBRARIES}")
string(REPLACE "^^" ";" MITK_USE_Boost_COMPILED_LIBRARIES "${MITK_USE_Boost_COMPILED_LIBRARIES}")

set(proj Boost)
set(proj_DEPENDENCIES )
set(Boost_DEPENDS ${proj})

#[[ Provisioning fetches the dependency closure of the requested libraries from a
    committed map (CMakeExternals/Boost/). When MITK builds its own standalone
    Python, the closure is fetched in parallel; otherwise a sequential pure-CMake
    fetch is used. Using Python only adds a build-order dependency on Python3,
    which is registered before Boost in ExternalProjectList.cmake for that reason.
    Gate on Python3_ROOT_DIR, not MITK_USE_Python3: an externally provided
    Python3_DIR leaves the former unset, and deriving an interpreter path from it
    would point at a non-existent binary (see the guard in SuperBuild.cmake). ]]
if(Python3_ROOT_DIR)
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

  #[[ The dependency closure of every requested library (header-only and
      compiled) is fetched so all headers are available. ]]
  set(boost_all_libraries ${MITK_USE_Boost_HEADER_LIBRARIES} ${MITK_USE_Boost_COMPILED_LIBRARIES})
  list(REMOVE_DUPLICATES boost_all_libraries)

  #[[ Boost's own CMake builds these and installs a BoostConfig.cmake, so
      downstream find_package(Boost) resolves in config mode. Only compiled
      libraries produce binaries. With no compiled libraries, build just the
      header-only "headers" library so a config is still generated; the full
      header-only closure is installed on top afterwards. ]]
  set(boost_build_libraries ${MITK_USE_Boost_COMPILED_LIBRARIES})
  if(NOT boost_build_libraries)
    set(boost_build_libraries headers)
  endif()

  #[[ Transport lists through single -D arguments using the ExternalProject list
      separator; BoostProvision.cmake and the nested build decode them. ]]
  string(REPLACE ";" "^^" boost_all_arg "${boost_all_libraries}")
  string(REPLACE ";" "^^" boost_build_arg "${boost_build_libraries}")

  #[[ When upgrading Boost, bump this tag AND regenerate the dependency map:
      clone the new tag with --recurse-submodules, then run
      Boost/generate_boost_map.py <checkout> Boost/boost-deps.cmake ]]
  set(boost_tag boost-1.91.0)

  #[[ Copies the fetched header-only closure into the install tree. ]]
  set(boost_install_headers
    ${CMAKE_COMMAND} -DBOOST_SRC=<SOURCE_DIR> -DBOOST_INSTALL=<INSTALL_DIR>
                     -P ${provision_dir}/install_headers.cmake)

  #[[ Build the requested Boost libraries with Boost's own CMake support and
      let it install a BoostConfig.cmake. Header-only libraries (including the
      "headers" fallback) become INTERFACE targets; only compiled libraries
      produce binaries. The full header-only closure is installed on top. ]]
  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG ${boost_tag}
    GIT_SHALLOW 1
    GIT_SUBMODULES "" #[[ Superproject only; the closure is fetched selectively. ]]
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR_PLATFORM ${gen_platform}
    CMAKE_ARGS
      ${ep_common_args}
      -DBOOST_INSTALL_LAYOUT:STRING=system
      -DBOOST_INCLUDE_LIBRARIES:STRING=${boost_build_arg}
    CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
      -DBUILD_TESTING:BOOL=OFF
    CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
    DEPENDS ${proj_DEPENDENCIES}
  )

  #[[ BoostRoot needs its CMake infrastructure submodules. ]]
  ExternalProject_Add_Step(${proj} fetch_tools
    COMMAND ${GIT_EXECUTABLE} submodule update --init --depth 1
            tools/cmake tools/build tools/boost_install
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
  )

  #[[ Install the full header-only closure on top of the built libraries. ]]
  ExternalProject_Add_Step(${proj} install_headers
    COMMAND ${boost_install_headers}
    DEPENDEES install
  )

  #[[ After the clone, fetch the dependency closure of all requested
      libraries so it is present when Boost's CMake configures. ]]
  ExternalProject_Add_Step(${proj} provision
    COMMAND ${CMAKE_COMMAND}
            -DBOOST_SRC=<SOURCE_DIR>
            -DBOOST_LIBRARIES=${boost_all_arg}
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
