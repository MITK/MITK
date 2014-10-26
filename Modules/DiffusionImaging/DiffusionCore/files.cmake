set(CPP_FILES

  # DicomImport
  DicomImport/mitkDicomDiffusionImageReader.cpp
  # DicomImport/mitkGroupDiffusionHeadersFilter.cpp
  DicomImport/mitkDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkGEDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkPhilipsDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkSiemensDicomDiffusionImageHeaderReader.cpp
  DicomImport/mitkSiemensMosaicDicomDiffusionImageHeaderReader.cpp

  DicomImport/mitkDiffusionDICOMFileReader.cpp
  DicomImport/mitkDiffusionHeaderDICOMFileReader.cpp
  DicomImport/mitkDiffusionHeaderSiemensDICOMFileReader.cpp
  DicomImport/mitkDiffusionHeaderSiemensDICOMFileHelper.cpp
  DicomImport/mitkDiffusionHeaderSiemensMosaicDICOMFileReader.cpp
  DicomImport/mitkDiffusionHeaderGEDICOMFileReader.cpp
  DicomImport/mitkDiffusionHeaderPhilipsDICOMFileReader.cpp


  # DataStructures -> DWI
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageHeaderInformation.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageSource.cpp

  IODataStructures/DiffusionWeightedImages/mitkImageToDiffusionImageSource.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageCorrectionFilter.cpp

  # DataStructures -> QBall
  IODataStructures/QBallImages/mitkQBallImageSource.cpp
  IODataStructures/QBallImages/mitkQBallImage.cpp

  # DataStructures -> Tensor
  IODataStructures/TensorImages/mitkTensorImage.cpp

  Rendering/vtkMaskedProgrammableGlyphFilter.cpp
  Rendering/mitkVectorImageVtkGlyphMapper3D.cpp
  Rendering/vtkOdfSource.cxx
  Rendering/vtkThickPlane.cxx
  Rendering/mitkOdfNormalizationMethodProperty.cpp
  Rendering/mitkOdfScaleByProperty.cpp

  # Algorithms
  Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.cpp
  Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.cpp
  Algorithms/itkDwiGradientLengthCorrectionFilter.cpp
  Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.h

  # Registration Algorithms & Co.
  Algorithms/Registration/mitkRegistrationWrapper.cpp
  Algorithms/Registration/mitkPyramidImageRegistrationMethod.cpp
  # Algorithms/Registration/mitkRegistrationMethodITK4.cpp


  # MultishellProcessing
  Algorithms/Reconstruction/MultishellProcessing/itkADCAverageFunctor.cpp
  Algorithms/Reconstruction/MultishellProcessing/itkADCFitFunctor.cpp
  Algorithms/Reconstruction/MultishellProcessing/itkKurtosisFitFunctor.cpp
  Algorithms/Reconstruction/MultishellProcessing/itkBiExpFitFunctor.cpp

  # Function Collection
  mitkDiffusionFunctionCollection.cpp
)

set(H_FILES
  # function Collection
  mitkDiffusionFunctionCollection.h

  # Rendering
  Rendering/mitkDiffusionImageMapper.h
  Rendering/mitkOdfVtkMapper2D.h

  # Reconstruction
  Algorithms/Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Algorithms/Reconstruction/mitkTeemDiffusionTensor3DReconstructionImageFilter.h
  Algorithms/Reconstruction/itkAnalyticalDiffusionQballReconstructionImageFilter.h
  Algorithms/Reconstruction/itkDiffusionMultiShellQballReconstructionImageFilter.h
  Algorithms/Reconstruction/itkPointShell.h
  Algorithms/Reconstruction/itkOrientationDistributionFunction.h
  Algorithms/Reconstruction/itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h

  # MultishellProcessing
  Algorithms/Reconstruction/MultishellProcessing/itkRadialMultishellToSingleshellImageFilter.h
  Algorithms/Reconstruction/MultishellProcessing/itkDWIVoxelFunctor.h
  Algorithms/Reconstruction/MultishellProcessing/itkADCAverageFunctor.h
  Algorithms/Reconstruction/MultishellProcessing/itkKurtosisFitFunctor.h
  Algorithms/Reconstruction/MultishellProcessing/itkBiExpFitFunctor.h
  Algorithms/Reconstruction/MultishellProcessing/itkADCFitFunctor.h

  # IO Datastructures
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImage.h

  # Algorithms
  Algorithms/itkDiffusionQballGeneralizedFaImageFilter.h
  Algorithms/itkDiffusionQballPrepareVisualizationImageFilter.h
  Algorithms/itkTensorDerivedMeasurementsFilter.h
  Algorithms/itkBrainMaskExtractionImageFilter.h
  Algorithms/itkB0ImageExtractionImageFilter.h
  Algorithms/itkB0ImageExtractionToSeparateImageFilter.h
  Algorithms/itkTensorImageToDiffusionImageFilter.h
  Algorithms/itkTensorToL2NormImageFilter.h
  Algorithms/itkGaussianInterpolateImageFunction.h
  Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.h
  Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.h
  Algorithms/itkDiffusionTensorPrincipalDirectionImageFilter.h
  Algorithms/itkCartesianToPolarVectorImageFilter.h
  Algorithms/itkPolarToCartesianVectorImageFilter.h
  Algorithms/itkDistanceMapFilter.h
  Algorithms/itkProjectionFilter.h
  Algorithms/itkResidualImageFilter.h
  Algorithms/itkExtractChannelFromRgbaImageFilter.h
  Algorithms/itkTensorReconstructionWithEigenvalueCorrectionFilter.h
  Algorithms/itkMergeDiffusionImagesFilter.h
  Algorithms/itkDwiPhantomGenerationFilter.h
  Algorithms/itkFiniteDiffOdfMaximaExtractionFilter.h
  Algorithms/itkMrtrixPeakImageConverter.h
  Algorithms/itkFslPeakImageConverter.h
  Algorithms/itkShCoefficientImageImporter.h
  Algorithms/itkShCoefficientImageExporter.h
  Algorithms/itkOdfMaximaExtractionFilter.h
  Algorithms/itkResampleDwiImageFilter.h
  Algorithms/itkDwiGradientLengthCorrectionFilter.h
  Algorithms/itkAdcImageFilter.h
  Algorithms/itkDwiNormilzationFilter.h
  Algorithms/itkSplitDWImageFilter.h
  Algorithms/itkRemoveDwiChannelFilter.h
  Algorithms/itkExtractDwiChannelFilter.h

  Algorithms/Registration/mitkDWIHeadMotionCorrectionFilter.h
  Algorithms/mitkDiffusionImageToDiffusionImageFilter.h
  Algorithms/itkNonLocalMeansDenoisingFilter.h
  Algorithms/itkVectorImageToImageFilter.h


)

set( TOOL_FILES
)

