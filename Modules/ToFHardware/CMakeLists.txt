#Define the platform string
mitkMacroGetPMDPlatformString(_PLATFORM_STRING)

MITK_CREATE_MODULE(mitkToFHardware
  SUBPROJECTS MITK-ToF
  INCLUDE_DIRS ${MITK_BIN_DIR}
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS Mitk MitkExt mitkOpenCVVideoSupport MitkIGT LegacyAdaptors mitkCameraCalibration
  EXPORT_DEFINE MITK_TOFHARDWARE_EXPORT
  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
)

add_subdirectory(Testing)
add_subdirectory(Kinect)
add_subdirectory(PMD)
add_subdirectory(MesaSR4000)

# Install external libraries which are not directly linked to a MITK module
include(${CMAKE_CURRENT_SOURCE_DIR}/mitkToFHardwareInstallRules.cmake)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mitkToFHardwareInstallRules.cmake
               ${PROJECT_BINARY_DIR}/mitkToFHardwareInstallRules.cmake @COPYONLY)
CONFIGURE_FILE(mitkToFConfig.h.in ${PROJECT_BINARY_DIR}/mitkToFConfig.h @ONLY)

#foreach(tofhardwaresubfolder_dir ${tofhardwaresubfolder_dirs})
#  add_subdirectory(${tofhardwaresubfolder_dirs})
#endforeach()
