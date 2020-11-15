file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkAffineBaseDataInteractor3D.cpp
  mitkAffineImageCropperInteractor.cpp
  mitkApplyDiffImageOperation.cpp
  mitkBoundingObject.cpp
  mitkBoundingObjectGroup.cpp
  mitkCellOperation.cpp
  mitkClippingPlaneInteractor3D.cpp
  mitkColorSequence.cpp
  mitkColorSequenceCycleH.cpp
  mitkColorSequenceRainbow.cpp
  mitkCompressedImageContainer.cpp
  mitkCone.cpp
  mitkCuboid.cpp
  mitkCylinder.cpp
  mitkDataStorageSelection.cpp
  mitkEllipsoid.cpp
  mitkGridRepresentationProperty.cpp
  mitkGridVolumeMapperProperty.cpp
  mitkLabeledImageLookupTable.cpp
  mitkLabeledImageVolumeCalculator.cpp
  mitkLineOperation.cpp
  mitkLookupTableSource.cpp
  mitkMultiStepper.cpp
  mitkPlane.cpp
  mitkSurfaceDeformationDataInteractor3D.cpp
  mitkUnstructuredGrid.cpp
  mitkUnstructuredGridSource.cpp
  mitkVideoSource.cpp

  mitkColorConversions.cpp
)

set(RESOURCE_FILES
  Interactions/AffineInteraction3D.xml
  Interactions/AffineMouseConfig.xml
  Interactions/AffineKeyConfig.xml
  Interactions/ClippingPlaneInteraction3D.xml
  Interactions/ClippingPlaneTranslationConfig.xml
  Interactions/ClippingPlaneRotationConfig.xml
  Interactions/ClippingPlaneDeformationConfig.xml
  Interactions/CropperDeformationConfig.xml
)
