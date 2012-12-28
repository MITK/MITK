#Define the platform string
IF(WIN32)
  IF(CMAKE_CL_64)
    SET(_PLATFORM_STRING "W64")
  ELSE(CMAKE_CL_64)
    SET(_PLATFORM_STRING "W32")
  ENDIF(CMAKE_CL_64)
ELSE(WIN32)
  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    SET(_PLATFORM_STRING "L64")
  ELSE(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    SET(_PLATFORM_STRING "L32")
  ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
ENDIF(WIN32)

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
add_subdirectory(PMD)
add_subdirectory(MesaSR4000)

CONFIGURE_FILE(mitkToFConfig.h.in ${PROJECT_BINARY_DIR}/mitkToFConfig.h @ONLY)

#foreach(tofhardwaresubfolder_dir ${tofhardwaresubfolder_dirs})
#  add_subdirectory(${tofhardwaresubfolder_dirs})
#endforeach()
