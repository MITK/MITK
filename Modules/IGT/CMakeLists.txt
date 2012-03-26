INCLUDE(MITKIGTHardware.cmake)

IF(MITK_USE_MICRON_TRACKER)
  SET(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_MICRON_TRACKER_INCLUDE_DIR})
  SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_MICRON_TRACKER_LIB})
ENDIF(MITK_USE_MICRON_TRACKER)

IF(MITK_USE_MICROBIRD_TRACKER)
  SET(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_USE_MICROBIRD_TRACKER_INCLUDE_DIR})
  SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_USE_MICROBIRD_TRACKER_LIB})
ENDIF(MITK_USE_MICROBIRD_TRACKER)

MITK_CREATE_MODULE(MitkIGT
  SUBPROJECTS MITK-IGT
  INCLUDE_DIRS IGTFilters IGTTrackingDevices IGTToolManagement
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS Mitk tinyxml SceneSerialization
  ADDITIONAL_LIBS "${ADDITIONAL_LIBS}"
)

MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/ClaronMicron.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/IntuitiveDaVinci.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIAurora.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIAurora_Dome.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIAuroraCompactFG_Dome.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIAuroraPlanarFG_Dome.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIAuroraTabletopFG_Dome.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIAuroraTabletopFG_Prototype_Dome.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIPolaris.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/NDIPolarisVicra.stl )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/IGTTrackingDevices/TrackingVolumeData/StandardVolume.stl )

MITK_CHECK_MODULE(_RESULT MitkIGT)
IF(_RESULT)
  MESSAGE(STATUS "IGTTutorialStep1 won't be built. Missing: ${_RESULT}")
ELSE(_RESULT)
## create IGT config
CONFIGURE_FILE(mitkIGTConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTConfig.h @ONLY)

# add test programm for serial communication classADD_EXECUTABLE(SerialCommunicationTest IGTTrackingDevices/mitkSerialCommunicationTest.cpp)TARGET_LINK_LIBRARIES(SerialCommunicationTest mitkIGT Mitk tinyxml PocoXML)

ADD_SUBDIRECTORY(IGTTutorial)

ADD_SUBDIRECTORY(Testing)
ENDIF(_RESULT)

