include(MITKIGTHardware.cmake)

if(MITK_USE_MICRON_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_MICRON_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_MICRON_TRACKER_LIB})
endif(MITK_USE_MICRON_TRACKER)

if(MITK_USE_OPTITRACK_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_OPTITRACK_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_OPTITRACK_TRACKER_LIB})
  add_definitions( -DMITK_USE_OPTITRACK_TRACKER )
endif(MITK_USE_OPTITRACK_TRACKER)

if(MITK_USE_MICROBIRD_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_USE_MICROBIRD_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_USE_MICROBIRD_TRACKER_LIB})
endif(MITK_USE_MICROBIRD_TRACKER)

if(MITK_USE_POLHEMUS_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_POLHEMUS_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_POLHEMUS_TRACKER_LIB})
endif(MITK_USE_POLHEMUS_TRACKER)

MITK_CREATE_MODULE(
  INCLUDE_DIRS
    PUBLIC Algorithms Common DataManagement ExceptionHandling IO Rendering TrackingDevices TestingHelper
    PRIVATE ${ADDITIONAL_INCLUDE_DIRS}
  DEPENDS PUBLIC MitkImageStatistics MitkSceneSerialization MitkIGTBase MitkOpenIGTLink
  PACKAGE_DEPENDS ITK|ITKRegistrationCommon tinyxml OpenIGTLink
  ADDITIONAL_LIBS "${ADDITIONAL_LIBS}"
)

if(MitkIGT_IS_ENABLED)
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/ClaronMicron.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/IntuitiveDaVinci.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIAurora.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIAurora_Dome.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIAuroraCompactFG_Dome.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIAuroraPlanarFG_Dome.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIAuroraTabletopFG_Dome.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIAuroraTabletopFG_Prototype_Dome.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIPolarisOldModel.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIPolarisSpectra.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIPolarisSpectraExtendedPyramid.stl )
  MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Modules/IGT/Resources/NDIPolarisVicra.stl )
endif()

if(NOT MODULE_IS_ENABLED)
  message(STATUS "IGTTutorialStep1 won't be built. Missing: ${_RESULT}")
else()
## create IGT config
configure_file(mitkIGTConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTConfig.h @ONLY)

# add test programm for serial communication classADD_EXECUTABLE(SerialCommunicationTest IGTTrackingDevices/mitkSerialCommunicationTest.cpp)target_link_libraries(SerialCommunicationTest mitkIGT Mitk tinyxml PocoXML)

add_subdirectory(autoload/DeviceRegistry)

add_subdirectory(Tutorial)

add_subdirectory(Testing)

endif()

