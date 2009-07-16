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
 # IODataStructures/QBallImages/mitkQBallVolumeSource.cpp
 # IODataStructures/QBallImages/mitkNrrdQBallVolumeReader.cpp
 # IODataStructures/QBallImages/mitkNrrdQBallVolumeWriter.cpp
 # IODataStructures/QBallImages/mitkNrrdQBallVolumeIOFactory.cpp
 # IODataStructures/QBallImages/mitkNrrdQBallVolumeWriterFactory.cpp
  IODataStructures/mitkDiffusionImagingObjectFactory.cpp
)

SET(H_FILES 
  Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkPointShell.h
  IODataStructures/DiffusionWeightedImages/mitkDiffusionVolumes.h 
 # IODataStructures/QBallImages/mitkQBallVolume.h
)

SET( TOOL_FILES
)

IF(WIN32)
ENDIF(WIN32)

#MITK_MULTIPLEX_PICTYPE( Algorithms/mitkImageRegistrationMethod-TYPE.cpp ) 
