SET(CPP_FILES 

  # Reconstruction
  Reconstruction/QuadProg.cpp
  Reconstruction/Array.cpp

  # Algorithms
  Algorithms/itkDiffusionQballGeneralizedFaImageFilter.h
  Algorithms/itkDiffusionQballPrepareVisualizationImageFilter.h

  # DicomImport
  DicomImport/mitkDicomDiffusionImageReader.cpp
  DicomImport/mitkGroupDiffusionHeadersFilter.cpp
  DicomImport/mitkDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkGEDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkPhilipsDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkSiemensDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkSiemensMosaicDicomDiffusionImageHeaderReader.cpp

  # DataStructures
  IODataStructures/mitkDiffusionImagingObjectFactory.cpp

  # DataStructures -> DWI
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageHeaderInformation.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageSource.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionImageReader.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionImageWriter.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionImageIOFactory.cpp
  IODataStructures/DiffusionWeightedImages/mitkNrrdDiffusionImageWriterFactory.cpp

  # DataStructures -> QBall
  IODataStructures/QBallImages/mitkQBallImageSource.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageReader.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageWriter.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageIOFactory.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageWriterFactory.cpp
  IODataStructures/QBallImages/mitkQBallImage.cpp

  # DataStructures -> Tensor
  IODataStructures/TensorImages/mitkTensorImageSource.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageReader.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageWriter.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageIOFactory.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageWriterFactory.cpp
  IODataStructures/TensorImages/mitkTensorImage.cpp

  # Rendering
  Rendering/vtkMaskedGlyph2D.cpp
  Rendering/vtkMaskedGlyph3D.cpp
  Rendering/vtkMaskedProgrammableGlyphFilter.cpp
  Rendering/mitkVectorImageMapper2D.cpp
  Rendering/mitkCompositeMapper.cpp
  Rendering/mitkVectorImageVtkGlyphMapper3D.cpp
  Rendering/vtkOdfSource.cxx
  Rendering/vtkThickPlane.cxx
  Rendering/mitkOdfNormalizationMethodProperty.cpp
  Rendering/mitkOdfScaleByProperty.cpp
)

SET(H_FILES 
  Rendering/mitkDiffusionImageMapper.h
  Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkAnalyticalDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkPointShell.h
  Reconstruction/itkOrientationDistributionFunction.h
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImage.h 
  Rendering/mitkOdfVtkMapper2D.h
)

SET( TOOL_FILES
)

IF(WIN32)
ENDIF(WIN32)

#MITK_MULTIPLEX_PICTYPE( Algorithms/mitkImageRegistrationMethod-TYPE.cpp ) 
