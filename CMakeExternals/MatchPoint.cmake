#-----------------------------------------------------------------------------
# MatchPoint
#-----------------------------------------------------------------------------

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
set(proj_DEPENDENCIES ITK)

set(MatchPoint_DEPENDS ${proj})

if(NOT MatchPoint_DIR)

  if(MatchPoint_SOURCE_DIR)
    set(download_step SOURCE_DIR ${MatchPoint_SOURCE_DIR})
  else()
    set(download_step
        SVN_REPOSITORY https://svn.inet.dkfz-heidelberg.de/sbr/Sources/SBR-Projects/MatchPoint/trunk
        #SOURCE_DIR ${ep_prefix}/${proj}
       )
  endif()

  ExternalProject_Add(${proj}
     ${download_step}
     INSTALL_COMMAND ""
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DBUILD_TESTING:BOOL=OFF
       -DITK_DIR:PATH=${ITK_DIR} #/src/ITK-build
       -DMAP_USE_SYSTEM_GDCM:BOOL=ON
       -DMAP_DISABLE_ITK_IO_FACTORY_AUTO_REGISTER:BOOL=ON
       -DMAP_WRAP_Plastimatch:BOOL=ON
       -DGDCM_DIR:PATH=${GDCM_DIR}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  # MatchPoint does not support "make install" yet
  # set(${proj}_DIR ${ep_prefix})
  #  mitkFunctionInstallExternalCMakeProject(${proj})
  ExternalProject_Get_Property(${proj} binary_dir)
  set(MatchPoint_DIR ${binary_dir})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
