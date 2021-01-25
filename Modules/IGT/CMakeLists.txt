include(MITKIGTHardware.cmake)

if(MITK_USE_MICRON_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_MICRON_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_MICRON_TRACKER_LIB})
endif()

if(MITK_USE_OPTITRACK_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_OPTITRACK_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_OPTITRACK_TRACKER_LIB})
  add_definitions( -DMITK_USE_OPTITRACK_TRACKER )
endif()

if(MITK_USE_MICROBIRD_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_USE_MICROBIRD_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_USE_MICROBIRD_TRACKER_LIB})
endif()

if(MITK_USE_POLHEMUS_TRACKER)
  set(ADDITIONAL_INCLUDE_DIRS ${ADDITIONAL_INCLUDE_DIRS} ${MITK_POLHEMUS_TRACKER_INCLUDE_DIR})
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_POLHEMUS_TRACKER_LIB})
endif()

mitk_create_module(
  INCLUDE_DIRS
    PUBLIC Algorithms Common DataManagement ExceptionHandling IO Rendering TrackingDevices TestingHelper
    PRIVATE ${ADDITIONAL_INCLUDE_DIRS}
  DEPENDS PUBLIC MitkImageStatistics MitkSceneSerialization MitkIGTBase MitkOpenIGTLink
  PACKAGE_DEPENDS PRIVATE ITK|RegistrationCommon OpenIGTLink tinyxml2
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

if(MODULE_IS_ENABLED)
  configure_file(mitkIGTConfig.h.in ${PROJECT_BINARY_DIR}/mitkIGTConfig.h @ONLY)

  add_subdirectory(autoload/DeviceRegistry)
  add_subdirectory(Tutorial)
  add_subdirectory(Testing)
endif()
