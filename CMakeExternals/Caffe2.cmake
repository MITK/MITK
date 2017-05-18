#-----------------------------------------------------------------------------
# Caffe2
#-----------------------------------------------------------------------------

if(MITK_USE_Caffe2)

  # Sanity checks
  if(DEFINED Caffe2_DIR AND NOT EXISTS ${Caffe2_DIR})
    message(FATAL_ERROR "Caffe2_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Caffe2)
  set(proj_DEPENDENCIES )
  set(Caffe2_DEPENDS ${proj})
  
  if(WIN32)
    set(COPY_COMMAND copy)
  else()
    set(COPY_COMMAND cp)
  endif()

  if(NOT DEFINED Caffe2_DIR)

    ExternalProject_Add(${proj}
       URL /home/neher/Downloads/caffe2.tar.gz
       PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Caffe2.patch
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
         -DBUILD_TEST:BOOL=OFF
         -DBUILD_PYTHON:BOOL=OFF
         -DBUILD_BINARY:BOOL=OFF
         -DGFLAGS_ROOT_DIR:PATH=${ep_prefix}
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(Caffe2_DIR ${ep_prefix})    
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_Caffe2)
