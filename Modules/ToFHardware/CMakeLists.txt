include(mitkToFHardware.cmake)

#if(MITK_USE_TOF_PMDCAMCUBE)
#  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_PMD_SDK_DIR}/include)
#  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_PMD_LIB})
#endif(MITK_USE_TOF_PMDCAMCUBE)

#if(MITK_USE_TOF_O3)
#  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_PMD_SDK_DIR}/include)
#  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_PMD_LIB})
#endif(MITK_USE_TOF_O3)

#if(MITK_USE_TOF_PMDCAMBOARD)
#  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_PMD_SDK_DIR}/include)
#  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_PMD_LIB})
#endif(MITK_USE_TOF_PMDCAMBOARD)

if(MITK_USE_TOF_MESASR4000)
  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_MESA_SDK_DIR}/include)
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_MESA_LIB})
endif(MITK_USE_TOF_MESASR4000)

#IF(MITK_USE_TOF_KINECT)
#SET(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_KINECT_INCLUDE_DIR})
#SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_KINECT_LIB})
#ENDIF(MITK_USE_TOF_KINECT)

MITK_CREATE_MODULE(mitkToFHardware
  SUBPROJECTS MITK-ToF
  INCLUDE_DIRS ${MITK_BIN_DIR}
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS Mitk MitkExt mitkOpenCVVideoSupport MitkIGT LegacyAdaptors
  EXPORT_DEFINE MITK_TOFHARDWARE_EXPORT
  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
)

add_subdirectory(Testing)
add_subdirectory(Kinect)
add_subdirectory(PMDCamcube)
#add_subdirectory(PMDCamBoard)
#add_subdirectory(PMD03D)
#add_subdirectory(MesaSR4000)
#add_subdirectory()
#add_subdirectory()


#set(tofhardwaresubfolder_dirs
#    Kinect
#    Testing
#)

#foreach(tofhardwaresubfolder_dir ${tofhardwaresubfolder_dirs})
#  add_subdirectory(${tofhardwaresubfolder_dirs})
#endforeach()

 
