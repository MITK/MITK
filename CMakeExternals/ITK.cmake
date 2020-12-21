#-----------------------------------------------------------------------------
# ITK
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj ITK)
set(proj_DEPENDENCIES GDCM)

if(MITK_USE_OpenCV)
  list(APPEND proj_DEPENDENCIES OpenCV)
endif()

if(MITK_USE_HDF5)
  list(APPEND proj_DEPENDENCIES HDF5)
endif()

set(ITK_DEPENDS ${proj})

if(NOT DEFINED ITK_DIR)

  set(additional_cmake_args -DUSE_WRAP_ITK:BOOL=OFF)

  if(MITK_USE_OpenCV)
    list(APPEND additional_cmake_args
         -DModule_ITKVideoBridgeOpenCV:BOOL=ON
         -DOpenCV_DIR:PATH=${OpenCV_DIR}
        )
  endif()

  # Keep the behaviour of ITK 4.3 which by default turned on ITK Review
  # see MITK bug #17338
  list(APPEND additional_cmake_args
    -DModule_ITKReview:BOOL=ON
    -DModule_ITKOpenJPEG:BOOL=ON # for 4.7, the OpenJPEG is needed by review but the variable must be set
    -DModule_IsotropicWavelets:BOOL=ON
  )

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  mitk_query_custom_ep_vars()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     UPDATE_COMMAND ""
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/InsightToolkit-4.13.3.tar.gz
     URL_MD5 d1c10c8288b47577d718a71190444815
     PATCH_COMMAND
       ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.13.3.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_GENERATOR_PLATFORM ${gen_platform}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DBUILD_EXAMPLES:BOOL=OFF
       -DITK_USE_SYSTEM_GDCM:BOOL=ON
       -DGDCM_DIR:PATH=${GDCM_DIR}
       -DITK_USE_SYSTEM_HDF5:BOOL=ON
       -DHDF5_DIR:PATH=${HDF5_DIR}
       ${${proj}_CUSTOM_CMAKE_ARGS}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
       ${${proj}_CUSTOM_CMAKE_CACHE_ARGS}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
       ${${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(ITK_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
