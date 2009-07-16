SET(CPP_FILES 
  DicomImport/mitkDicomDiffusionVolumesReader.cpp
  DicomImport/mitkGroupDiffusionHeadersFilter.cpp
  DicomImport/mitkDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkGEDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkPhilipsDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkSiemensDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkSiemensMosaicDicomDiffusionVolumeHeaderReader.cpp
  DiffusionWeightedImages/mitkDiffusionVolumeHeaderInformation.cpp
  DiffusionWeightedImages/mitkDiffusionVolumesSource.cpp
  DiffusionWeightedImages/mitkNrrdDiffusionVolumesReader.cpp
  DiffusionWeightedImages/mitkNrrdDiffusionVolumesWriter.cpp
  DiffusionWeightedImages/mitkDiffusionImagingObjectFactory.cpp
  DiffusionWeightedImages/mitkNrrdDiffusionVolumesIOFactory.cpp
  DiffusionWeightedImages/mitkNrrdDiffusionVolumesWriterFactory.cpp
)

SET(H_FILES 
  Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkPointShell.h
  DiffusionWeightedImages/mitkDiffusionVolumes.h 
)

SET( TOOL_FILES
)

IF(WIN32)
ENDIF(WIN32)

#MITK_MULTIPLEX_PICTYPE( Algorithms/mitkImageRegistrationMethod-TYPE.cpp ) 
