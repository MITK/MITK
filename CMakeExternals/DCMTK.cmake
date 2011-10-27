#-----------------------------------------------------------------------------
# DCMTK
#-----------------------------------------------------------------------------

IF(MITK_USE_DCMTK)

  # Sanity checks
  IF(DEFINED DCMTK_DIR AND NOT EXISTS ${DCMTK_DIR})
    MESSAGE(FATAL_ERROR "DCMTK_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()
  
  SET(proj DCMTK)
  SET(proj_DEPENDENCIES )
  SET(DCMTK_DEPENDS ${proj})

  IF(NOT DEFINED DCMTK_DIR)
    IF(UNIX)
      SET(DCMTK_CXX_FLAGS "-fPIC")
      SET(DCMTK_C_FLAGS "-fPIC")
    ENDIF(UNIX)
    IF(DCMTK_DICOM_ROOT_ID)
      SET(DCMTK_CXX_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
      SET(DCMTK_C_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
    ENDIF()
    ExternalProject_Add(${proj}
      URL http://mitk.org/download/thirdparty/dcmtk-3.6.0.tar.gz
      INSTALL_DIR ${proj}-install
      PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchDCMTK-3.6.cmake 
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
         ${ep_common_args}
         -DDCMTK_OVERWRITE_WIN32_COMPILER_FLAGS:BOOL=OFF
         -DBUILD_SHARED_LIBS:BOOL=OFF
         "-DCMAKE_CXX_FLAGS:STRING=${ep_common_CXX_FLAGS} ${DCMTK_CXX_FLAGS}"
         "-DCMAKE_C_FLAGS:STRING=${ep_common_C_FLAGS} ${DCMTK_C_FLAGS}"
         -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
         -DDCMTK_WITH_ZLIB:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_OPENSSL:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_PNG:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_TIFF:BOOL=OFF  # see bug #9894
         -DDCMTK_WITH_XML:BOOL=OFF  # see bug #9894
         -DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=ON
      DEPENDS ${proj_DEPENDENCIES}
      )
    SET(DCMTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)
    
  ELSE()
  
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
  ENDIF()
ENDIF()     
