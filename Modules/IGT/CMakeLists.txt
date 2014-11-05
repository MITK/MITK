include(MITKIGTHardware.cmake)

if(MITK_USE_MICRON_TRACKER)
  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_MICRON_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_MICRON_TRACKER_LIB})
endif(MITK_USE_MICRON_TRACKER)

if(MITK_USE_OPTITRACK_TRACKER)
  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_OPTITRACK_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_OPTITRACK_TRACKER_LIB})
  add_definitions( -DMITK_USE_OPTITRACK_TRACKER )
endif(MITK_USE_OPTITRACK_TRACKER)

if(MITK_USE_MICROBIRD_TRACKER)
  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_USE_MICROBIRD_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_USE_MICROBIRD_TRACKER_LIB})
endif(MITK_USE_MICROBIRD_TRACKER)

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-IGT
  INCLUDE_DIRS Algorithms Common DataManagement ExceptionHandling IO Rendering TrackingDevices TestingHelper
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS MitkImageStatistics MitkSceneSerialization MitkIGTBase
  PACKAGE_DEPENDS ITK|ITKRegistrationCommon tinyxml
  ADDITIONAL_LIBS "${ADDITIONAL_LIBS}"
  #WARNINGS_AS_ERRORS disabled for release 2014-03 because of bug 17463
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


add_subdirectory(Tutorial)

add_subdirectory(Testing)
endif()

