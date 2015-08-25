MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Classification src/ImageFilters
  DEPENDS MitkCore MitkDiffusionCore MitkDataCollection MitkSegmentation MitkCLVigraRandomForest #<-- modules on which your module depends on
  PACKAGE_DEPENDS PUBLIC ITK|ITKDiffusionTensorImage+ITKLevelSets
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)

if (MODULE_IS_ENABLED)
 ADD_SUBDIRECTORY(MiniApps)
endif()
