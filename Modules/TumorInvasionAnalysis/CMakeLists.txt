MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Classification src/ImageFilters src/ReaderWriter
  DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking MitkDiffusionIO MitkDataCollection MitkSegmentation MitkCLVigraRandomForest #<-- modules on which your module depends
  PACKAGE_DEPENDS PUBLIC ITK|ITKDiffusionTensorImage+ITKLevelSets Qt5|Core
  WARNINGS_AS_ERRORS
)


if (MODULE_IS_ENABLED)
  add_subdirectory(test)
  ADD_SUBDIRECTORY(MiniApps)
endif()
