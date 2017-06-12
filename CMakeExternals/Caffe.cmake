#-----------------------------------------------------------------------------
# Caffe
#-----------------------------------------------------------------------------

if(MITK_USE_Caffe)

  # Sanity checks
  if(DEFINED Caffe_DIR AND NOT EXISTS ${Caffe_DIR})
    message(FATAL_ERROR "Caffe_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Caffe)
  set(proj_DEPENDENCIES )
  set(Caffe_DEPENDS ${proj})

  if(WIN32)
    set(COPY_COMMAND copy)
  else()
    set(COPY_COMMAND cp)
  endif()
  
  if(NOT DEFINED Caffe_DIR)
  
    find_path(Boost_INCLUDE_DIR boost)
    find_library(Boost_LIBRARY boost_system)    
    get_filename_component(Boost_LIBRARY_DIR ${Boost_LIBRARY} DIRECTORY)
        
    #set(revision_tag 63cad96)
    ExternalProject_Add(${proj}
       GIT_REPOSITORY https://github.com/BVLC/caffe.git
       #GIT_TAG ${revision_tag}
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
         -DCMAKE_CXX_FLAGS=-std=c++11
         -DCMAKE_CXX_STANDARD=11
         -DUSE_OPENMP:BOOL=ON
         -DBoost_INCLUDE_DIR:PATH=${Boost_INCLUDE_DIR}
         -DBoost_LIBRARY_DIR_RELEASE:PATH=${Boost_LIBRARY_DIR}
         -DBoost_LIBRARY_DIR_DEBUG:PATH=${Boost_LIBRARY_DIR}
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(Caffe_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_Caffe)
