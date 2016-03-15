MITK_CREATE_MODULE(DataCollection #<-- module name
  SUBPROJECTS
  INCLUDE_DIRS DataHolder ReaderWriter Iterators Utilities#<-- sub-folders of module
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS MitkDiffusionCore MitkDiffusionIO MitkFiberTracking MitkLegacyIO #<-- modules on which your module depends on
  PACKAGE_DEPENDS Qt5|Core Eigen
#  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)
 ADD_SUBDIRECTORY(Testing)
endif()
