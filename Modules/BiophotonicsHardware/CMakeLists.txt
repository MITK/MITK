#MITK_CREATE_MODULE(
#  INCLUDE_DIRS ${MITK_BIN_DIR}
#  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
#  DEPENDS MitkOpenCVVideoSupport
#  EXPORT_DEFINE MITK_TOFHARDWARE_EXPORT
#  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
#)



#Set Subfolder according to moduls
set( biophotonicsHardware_modul_dir
      SpectroCam
)

# add_subdirectories for each modul
foreach(biophotonicsHardware_modul_dir ${biophotonicsHardware_modul_dir})
  add_subdirectory(${biophotonicsHardware_modul_dir})
endforeach()


#add subdir for MiniApps
add_subdirectory(MiniApps)


IF(BUILD_TESTING)
  add_subdirectory(Testing)
ENDIF(BUILD_TESTING)
