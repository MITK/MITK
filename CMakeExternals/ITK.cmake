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

if(MITK_USE_VTK)
  list(APPEND proj_DEPENDENCIES VTK)
endif()

set(ITK_DEPENDS ${proj})

if(NOT DEFINED ITK_DIR)

  set(additional_cmake_args )
  if(MINGW)
    set(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF)
  endif()

  list(APPEND additional_cmake_args
       -DUSE_WRAP_ITK:BOOL=OFF
      )

  if(MITK_USE_OpenCV)
    list(APPEND additional_cmake_args
         -DModule_ITKVideoBridgeOpenCV:BOOL=ON
         -DOpenCV_DIR:PATH=${OpenCV_DIR}
        )
  endif()

  if(MITK_USE_OpenCL)
    list(APPEND additional_cmake_args
         -DITK_USE_GPU:BOOL=ON
         -DOPENCL_INCLUDE_DIRS:PATH=${OpenCL_INCLUDE_DIR}
         -DOPENCL_LIBRARIES:FILEPATH=${OpenCL_LIBRARY}
        )
  endif()

  # Keep the behaviour of ITK 4.3 which by default turned on ITK Review
  # see MITK bug #17338
  list(APPEND additional_cmake_args
    -DModule_ITKReview:BOOL=ON
  # for 4.7, the OpenJPEG is needed by review but the variable must be set
    -DModule_ITKOpenJPEG:BOOL=ON
  )

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  set(vcl_constexpr_patch)
  if(GCC_VERSION VERSION_LESS 4.8 AND GCC_VERSION VERSION_GREATER 4)
    set(vcl_constexpr_patch
      COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.7.1-gcc-4.6.patch
    )
  endif()

  # this will be in ITK 4.8, see https://issues.itk.org/jira/browse/ITK-3361
  set(vcl_gcc5_patch)
  if(GCC_VERSION VERSION_GREATER 4)
    set(vcl_gcc5_patch
      COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.7.1-gcc-5.patch
    )
  endif()

  if(MITK_USE_VTK)
    list(APPEND additional_cmake_args
      -DModule_ITKVtkGlue:BOOL=ON
      -DVTK_DIR:PATH=${VTK_DIR}
    )
  endif()

  if(MITK_USE_VTK)
    list(APPEND additional_cmake_args
      -DModule_ITKVtkGlue:BOOL=ON
      -DVTK_DIR:PATH=${VTK_DIR}
    )
  endif()

  if(MITK_PDB)
    list(APPEND additional_cmake_args
      "-DCMAKE_CXX_FLAGS_RELEASE:STRING=/MD /Zi /O2 /Ob1 /DNDEBUG"
      "-DCMAKE_SHARED_LINKER_FLAGS_RELEASE:STRING=/DEBUG /INCREMENTAL /OPT:REF /OPT:ICF"
      "-DCMAKE_EXE_LINKER_FLAGS_RELEASE:STRING=/DEBUG /INCREMENTAL /OPT:REF /OPT:ICF"
      "-DCMAKE_MODULE_LINKER_FLAGS_RELEASE:STRING=/DEBUG /INCREMENTAL /OPT:REF /OPT:ICF"
    )
  endif()

  ExternalProject_Add(${proj}
     LIST_SEPARATOR ${sep}
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/InsightToolkit-4.13.1.tar.xz
     URL_MD5 bc7296e7faccdcb5656a7669d4d875d2
     # work with external GDCM
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK_search_gpu.patch
       COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK_fixed_4D_progress.patch
       COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK_rescaling_fix.patch
       COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.9-accurate-spacingZ.patch
       COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK_itkIsoContourDistanceImageFilter.patch
       COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-any-gcc.patch
       COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK_glad_cl.patch
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DBUILD_EXAMPLES:BOOL=OFF
       -DITK_USE_SYSTEM_GDCM:BOOL=ON
       -DGDCM_DIR:PATH=${GDCM_DIR}
       -DITK_USE_SYSTEM_HDF5:BOOL=OFF
       -DHDF5_DIR:PATH=${HDF5_DIR}
     CMAKE_CACHE_ARGS
       ${ep_common_cache_args}
     CMAKE_CACHE_DEFAULT_ARGS
       ${ep_common_cache_default_args}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(ITK_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

  # Install pdb files
  if (WIN32 AND MITK_PDB)
    message("TESTTESTTEST")
    ExternalProject_Get_Property(${proj} binary_dir)
    message("${binary_dir}/bin/${CMAKE_BUILD_TYPE}")
    message("${ep_prefix}/bin")
    INSTALL(DIRECTORY ${binary_dir}/bin/${CMAKE_BUILD_TYPE}
      DESTINATION ${ep_prefix}/bin
      CONFIGURATIONS Release
      FILES_MATCHING
      PATTERN *.pdb
    )
  endif()

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
