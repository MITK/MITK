#-----------------------------------------------------------------------------
# DCMTK
#-----------------------------------------------------------------------------

if(MITK_USE_DCMTK)

  # Sanity checks
  if(DEFINED DCMTK_DIR AND NOT EXISTS ${DCMTK_DIR})
    message(FATAL_ERROR "DCMTK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj DCMTK)
  set(proj_DEPENDENCIES )
  set(DCMTK_DEPENDS ${proj})

  if(NOT DEFINED DCMTK_DIR)
    if(UNIX)
      set(DCMTK_CXX_FLAGS "-fPIC")
      set(DCMTK_C_FLAGS "-fPIC")
    endif(UNIX)
    if(DCMTK_DICOM_ROOT_ID)
      set(DCMTK_CXX_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
      set(DCMTK_C_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
    endif()


    set (dcmtk_shared_flags "-DBUILD_SHARED_LIBS:BOOL=${MITK_DCMTK_BUILD_SHARED_LIBS}")
    if (NOT MITK_DCMTK_BUILD_SHARED_LIBS)
      set (dcmtk_shared_flags ${dcmtk_shared_flags} "-DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=ON")
    endif()

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/dcmtk-3.6.1_20120222.tar.gz
      URL_MD5 86fa9e0f91e4e0c6b44d513ea48391d6
      INSTALL_DIR ${proj}-install
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
         ${ep_common_args}
         -DDCMTK_OVERWRITE_WIN32_COMPILER_FLAGS:BOOL=OFF
         ${dcmtk_shared_flags}
         "-DCMAKE_CXX_FLAGS:STRING=${ep_common_CXX_FLAGS} ${DCMTK_CXX_FLAGS}"
         "-DCMAKE_C_FLAGS:STRING=${ep_common_C_FLAGS} ${DCMTK_C_FLAGS}"
         -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
         -DDCMTK_INSTALL_BINDIR:STRING=bin/${CMAKE_CFG_INTDIR}
         -DDCMTK_INSTALL_LIBDIR:STRING=lib/${CMAKE_CFG_INTDIR}
         -DDCMTK_WITH_DOXYGEN:BOOL=OFF
         -DDCMTK_WITH_ZLIB:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_OPENSSL:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_PNG:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_TIFF:BOOL=OFF  # see bug #9894
         -DDCMTK_WITH_XML:BOOL=OFF  # see bug #9894
         -DDCMTK_WITH_ICONV:BOOL=OFF  # see bug #9894
      DEPENDS ${proj_DEPENDENCIES}
      )
    set(DCMTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif()
