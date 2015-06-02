#MITK_CREATE_MODULE(
#  INCLUDE_DIRS ${MITK_BIN_DIR}
#  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
#  DEPENDS MitkOpenCVVideoSupport
#  EXPORT_DEFINE MITK_TOFHARDWARE_EXPORT
#  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
#)



#Set Subfolder according to moduls
set( biophotonicsHardware_module_dir
      SpectroCam
)

# add_subdirectories for each module
foreach(biophotonicsHardware_module_dir ${biophotonicsHardware_module_dir})
  add_subdirectory(${biophotonicsHardware_module_dir})
endforeach()




#IF(BUILD_TESTING)
#  add_subdirectory(Testing)
#ENDIF(BUILD_TESTING)
