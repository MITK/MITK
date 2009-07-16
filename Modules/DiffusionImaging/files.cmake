SET(CPP_FILES 
  DicomImport/mitkDicomDiffusionVolumesReader.cpp
  DicomImport/mitkGroupDiffusionHeadersFilter.cpp
  DicomImport/mitkDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkGEDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkPhilipsDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkSiemensDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkSiemensMosaicDicomDiffusionVolumeHeaderReader.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionVolumeHeaderInformation.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionVolumesSource.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesReader.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesWriter.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesIOFactory.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesWriterFactory.cpp
  DiffusionWeightedImages/mitkDiffusionImagingObjectFactory.cpp
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
