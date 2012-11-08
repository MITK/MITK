include(mitkToFHardware.cmake)

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

#foreach(tofhardwaresubfolder_dir ${tofhardwaresubfolder_dirs})
#  add_subdirectory(${tofhardwaresubfolder_dirs})
#endforeach()
