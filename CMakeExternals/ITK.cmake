#-----------------------------------------------------------------------------
# ITK
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj ITK)
set(proj_DEPENDENCIES GDCM)

set(ITK_DEPENDS ${proj})

if(NOT DEFINED ITK_DIR)

  set(additional_cmake_args )
  if(MINGW)
    set(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF)
  endif()

  if(MITK_USE_Python)
    list(APPEND additional_cmake_args
         -DUSE_WRAP_ITK:BOOL=ON
         -DITK_USE_REVIEW:BOOL=ON
         -DCableSwig_DIR:PATH=${CableSwig_DIR}
         -DWRAP_ITK_JAVA:BOOL=OFF
         -DWRAP_ITK_TCL:BOOL=OFFF
         -DWRAP_unsigned_char:BOOL=ON
        )
  endif()

  if(GDCM_IS_2_0_18)
    set(ITK_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchITK-3.20.cmake)
  endif()

  ExternalProject_Add(${proj}
     SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
     BINARY_DIR ${proj}-build
     PREFIX ${proj}-cmake
     URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/InsightToolkit-3.20.1.tar.gz
     URL_MD5 90342ffa78bd88ae48b3f62866fbf050
     INSTALL_COMMAND ""
     PATCH_COMMAND ${ITK_PATCH_COMMAND}
     CMAKE_GENERATOR ${gen}
     CMAKE_ARGS
       ${ep_common_args}
       ${additional_cmake_args}
       -DBUILD_TESTING:BOOL=OFF
       -DBUILD_EXAMPLES:BOOL=OFF
       -DITK_USE_SYSTEM_GDCM:BOOL=ON
       -DGDCM_DIR:PATH=${GDCM_DIR}
     DEPENDS ${proj_DEPENDENCIES}
    )

  set(ITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
