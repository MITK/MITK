#Define the platform string
mitkMacroGetPMDPlatformString(_PLATFORM_STRING)

MITK_CREATE_MODULE(
  DEPENDS MitkOpenCVVideoSupport MitkIGTBase MitkCameraCalibration MitkIpPic
  PACKAGE_DEPENDS PRIVATE ITK|IONRRD
  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
)

# set(tofHardware_srcs )
# usFunctionEmbedResources(tofHardware_srcs
                         # LIBRARY_NAME mitkToFHardware
                         # ROOT_DIR ${MITK_DATA_DIR}/ToF-Data/CalibrationFiles
                         # FILES  Default_Parameters.xml Kinect_IR_camera.xml Kinect_RGB_camera.xml Mesa-SR4000_Camera.xml PMDCamBoard_camera.xml PMDCamCube2_camera.xml PMDCamCube3_camera.xml
                        # )

add_subdirectory(Testing)
add_subdirectory(Kinect)
add_subdirectory(KinectV2)
add_subdirectory(PMD)
add_subdirectory(MesaSR4000)

# Install external libraries which are not directly linked to a MITK module
include(${CMAKE_CURRENT_SOURCE_DIR}/mitkToFHardwareInstallRules.cmake)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mitkToFHardwareInstallRules.cmake
               ${PROJECT_BINARY_DIR}/mitkToFHardwareInstallRules.cmake COPYONLY)
CONFIGURE_FILE(mitkToFConfig.h.in ${PROJECT_BINARY_DIR}/mitkToFConfig.h @ONLY)

#foreach(tofhardwaresubfolder_dir ${tofhardwaresubfolder_dirs})
#  add_subdirectory(${tofhardwaresubfolder_dirs})
#endforeach()
