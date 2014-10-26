#-----------------------------------------------------------------------------
# Eigen
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED Eigen_DIR AND NOT EXISTS ${Eigen_DIR})
  message(FATAL_ERROR "Eigen_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj Eigen)
set(proj_DEPENDENCIES )
set(Eigen_DEPENDS ${proj})

if(NOT DEFINED Eigen_DIR)

  ExternalProject_Add(${proj}
    PREFIX ${CMAKE_BINARY_DIR}/${proj}-cmake
    URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/Eigen-3.2.2-headers-only.tar.gz
    URL_MD5 d0a7fe82ab7bd39bf577afebe287aa20
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-src
  )

  set(Eigen_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src)

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
