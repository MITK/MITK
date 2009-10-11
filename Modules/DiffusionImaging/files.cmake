SET(CPP_FILES 
  DicomImport/mitkDicomDiffusionVolumesReader.cpp
  DicomImport/mitkGroupDiffusionHeadersFilter.cpp
  DicomImport/mitkDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkGEDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkPhilipsDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkSiemensDicomDiffusionVolumeHeaderReader.cpp
  DicomImport/mitkSiemensMosaicDicomDiffusionVolumeHeaderReader.cpp
  IODataStructures/mitkDiffusionImagingObjectFactory.cpp
  # DWI
  IODataStructures/DiffusionWeightedImages/mitkDiffusionVolumeHeaderInformation.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionVolumesSource.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesReader.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesWriter.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesIOFactory.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionVolumesWriterFactory.cpp
  # QBall
  IODataStructures/QBallImages/mitkQBallImageSource.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageReader.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageWriter.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageIOFactory.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageWriterFactory.cpp
  IODataStructures/QBallImages/mitkQBallImage.cpp
  # Tensor
  IODataStructures/TensorImages/mitkTensorImageSource.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageReader.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageWriter.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageIOFactory.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageWriterFactory.cpp
  IODataStructures/TensorImages/mitkTensorImage.cpp
  Rendering/vtkMaskedGlyph2D.cpp
  Rendering/vtkMaskedGlyph3D.cpp
  Rendering/vtkMaskedProgrammableGlyphFilter.cpp
  Rendering/mitkVectorImageMapper2D.cpp
  Rendering/mitkVectorImageVtkGlyphMapper3D.cpp
  Rendering/vtkOdfSource.cxx
  Rendering/vtkThickPlane.cxx
  Rendering/mitkOdfNormalizationMethodProperty.cpp
  Rendering/mitkOdfScaleByProperty.cpp
)

SET(H_FILES 
  Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkPointShell.h
  Reconstruction/itkOrientationDistributionFunction.h
  IODataStructures/DiffusionWeightedImages/mitkDiffusionVolumes.h 
  Rendering/mitkOdfVtkMapper2D.h
)

SET( TOOL_FILES
)

IF(WIN32)
ENDIF(WIN32)

#MITK_MULTIPLEX_PICTYPE( Algorithms/mitkImageRegistrationMethod-TYPE.cpp ) 
