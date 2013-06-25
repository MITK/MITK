
mitkMacroGetPMDPlatformString(_PLATFORM_STRING)

#Only for CamCube
if(MITK_USE_TOF_PMDCAMCUBE)
  MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcube3.${_PLATFORM_STRING}.pap CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.${_PLATFORM_STRING}.ppp CONFIGURATIONS Release)
  if(_PLATFORM_STRING MATCHES "W32")
    MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/pmdfile.W32.pcp CONFIGURATIONS Release)
  endif()
endif()

#-------------------------------Begin CamBoard hardware-------------------------------------------
if(MITK_USE_TOF_PMDCAMBOARD)
  if(_PLATFORM_STRING MATCHES "W32")
    MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboard.W32.pap CONFIGURATIONS Release)
    MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboardproc.W32.ppp CONFIGURATIONS Release)
    MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/pmdfile.W32.pcp CONFIGURATIONS Release)
  elseif(_PLATFORM_STRING MATCHES "W64")
#    MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboard.W64.pap CONFIGURATIONS Release)
#    MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboardproc.W64.ppp CONFIGURATIONS Release)
  endif()
endif()

#----------------------------------------Begin PMD O3 hardware--------------------------------------------

if(WIN32 AND MITK_USE_TOF_PMDO3)
  MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/o3d.W32.pcp CONFIGURATIONS Release)
ENDIF()
