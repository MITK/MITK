include(MITKIGTHardware.cmake)

if(MITK_USE_MICRON_TRACKER)
  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_MICRON_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_MICRON_TRACKER_LIB})
endif(MITK_USE_MICRON_TRACKER)

if(MITK_USE_MICROBIRD_TRACKER)
  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_USE_MICROBIRD_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_USE_MICROBIRD_TRACKER_LIB})
endif(MITK_USE_MICROBIRD_TRACKER)

MITK_CREATE_MODULE(MitkIGT
  SUBPROJECTS MITK-IGT
  INCLUDE_DIRS IGTFilters IGTTrackingDevices IGTToolManagement IGTExceptionHandling
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS Mitk tinyxml SceneSerialization
  ADDITIONAL_LIBS "${ADDITIONAL_LIBS}"
)

if(MitkIGT_IS_ENABLED)
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
endif()

MITK_CHECK_MODULE(_RESULT MitkIGT)
if(_RESULT)
  message(STATUS "IGTTutorialStep1 won't be built. Missing: ${_RESULT}")
else(_RESULT)
## create IGT config
configure_file(mitkIGTConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTConfig.h @ONLY)

# add test programm for serial communication classADD_EXECUTABLE(SerialCommunicationTest IGTTrackingDevices/mitkSerialCommunicationTest.cpp)target_link_libraries(SerialCommunicationTest mitkIGT Mitk tinyxml PocoXML)

add_subdirectory(IGTTutorial)

add_subdirectory(Testing)
endif(_RESULT)

