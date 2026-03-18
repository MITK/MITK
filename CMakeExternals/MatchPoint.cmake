#-----------------------------------------------------------------------------
# MatchPoint
#-----------------------------------------------------------------------------
if(MITK_USE_MatchPoint)

  set(MatchPoint_SOURCE_DIR "" CACHE PATH "Location of the MatchPoint source directory")
  mark_as_advanced(MatchPoint_SOURCE_DIR)

  # Sanity checks
  if(DEFINED MatchPoint_DIR AND NOT EXISTS ${MatchPoint_DIR})
    message(FATAL_ERROR "MatchPoint_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT MatchPoint_DIR AND MatchPoint_SOURCE_DIR AND NOT EXISTS ${MatchPoint_SOURCE_DIR})
    message(FATAL_ERROR "MatchPoint_SOURCE_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj MatchPoint)
  set(proj_DEPENDENCIES Boost ITK)

  set(MatchPoint_DEPENDS ${proj})

  if(NOT MatchPoint_DIR)

    set(additional_cmake_args)

    if(MatchPoint_SOURCE_DIR)
      set(download_step SOURCE_DIR ${MatchPoint_SOURCE_DIR})
    else()
      set(download_step
          GIT_REPOSITORY https://github.com/MIC-DKFZ/MatchPoint.git
          GIT_TAG c4c8f0dfedadf4891eb03385c3e976e39d62b3c7 # 2026-03-10
         )
    endif()

    set(MatchPoint_VERSION "0.14")

    string(REPLACE "-DBOOST_ALL_DYN_LINK" "" modified_ep_common_args "${ep_common_args}")

    ExternalProject_Add(${proj}
       ${download_step}
       # INSTALL_COMMAND "${CMAKE_COMMAND} -P cmake_install.cmake"
       CMAKE_GENERATOR ${gen}
       CMAKE_GENERATOR_PLATFORM ${gen_platform}
       CMAKE_ARGS
         ${modified_ep_common_args}
         ${additional_cmake_args}
         -DBUILD_TESTING:BOOL=OFF
         -DITK_DIR:PATH=${ITK_DIR} #/src/ITK-build
         "-DBoost_DIR:PATH=${Boost_DIR}"
         -DMAP_USE_SYSTEM_GDCM:BOOL=ON
         -DMAP_DISABLE_ITK_IO_FACTORY_AUTO_REGISTER:BOOL=ON
         -DMAP_WRAP_Plastimatch:BOOL=ON
         -DMAP_BUILD_Ontology:BOOL=ON
         -DMAP_BUILD_Ontology_simple:BOOL=ON
         -DGDCM_DIR:PATH=${GDCM_DIR}
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(${proj}_DIR "${ep_prefix}/lib/cmake/MatchPoint-${MatchPoint_VERSION}")
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()

endif(MITK_USE_MatchPoint)
