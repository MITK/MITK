set(CPP_FILES

  # DicomImport
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
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageCorrectionFilter.cpp
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageCreationFilter.cpp

  # Properties
  IODataStructures/Properties/mitkBValueMapProperty.cpp
  IODataStructures/Properties/mitkGradientDirectionsProperty.cpp
  IODataStructures/Properties/mitkMeasurementFrameProperty.cpp
  IODataStructures/Properties/mitkDiffusionPropertyHelper.cpp
  IODataStructures/Properties/mitkNodePredicateIsDWI.cpp

  # Serializer
  IODataStructures/Properties/mitkBValueMapPropertySerializer.cpp
  IODataStructures/Properties/mitkGradientDirectionsPropertySerializer.cpp
  IODataStructures/Properties/mitkMeasurementFramePropertySerializer.cpp

  # DataStructures -> Odf
  IODataStructures/OdfImages/mitkOdfImageSource.cpp
  IODataStructures/OdfImages/mitkOdfImage.cpp
  IODataStructures/mitkShImage.cpp
  IODataStructures/mitkShImageSource.cpp

  # DataStructures -> Tensor
  IODataStructures/TensorImages/mitkTensorImage.cpp

  # DataStructures -> Peaks
  IODataStructures/mitkPeakImage.cpp

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

  # Registration Algorithms & Co.
  Algorithms/Registration/mitkRegistrationWrapper.cpp
  Algorithms/Registration/mitkPyramidImageRegistrationMethod.cpp
  # Algorithms/Registration/mitkRegistrationMethodITK4.cpp
  Algorithms/Registration/mitkDWIHeadMotionCorrectionFilter.cpp


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
  include/mitkDiffusionFunctionCollection.h

  # Rendering
  include/Rendering/mitkOdfVtkMapper2D.h

  # Reconstruction
  include/Algorithms/Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  include/Algorithms/Reconstruction/mitkTeemDiffusionTensor3DReconstructionImageFilter.h
  include/Algorithms/Reconstruction/itkAnalyticalDiffusionQballReconstructionImageFilter.h
  include/Algorithms/Reconstruction/itkDiffusionMultiShellQballReconstructionImageFilter.h
  include/Algorithms/Reconstruction/itkPointShell.h
  include/Algorithms/Reconstruction/itkOrientationDistributionFunction.h
  include/Algorithms/Reconstruction/itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h
  include/Algorithms/Reconstruction/itkDiffusionKurtosisReconstructionImageFilter.h
  include/Algorithms/Reconstruction/itkBallAndSticksImageFilter.h
  include/Algorithms/Reconstruction/itkMultiTensorImageFilter.h

  # Fitting functions
  include/Algorithms/Reconstruction/FittingFunctions/mitkAbstractFitter.h
  include/Algorithms/Reconstruction/FittingFunctions/mitkMultiTensorFitter.h
  include/Algorithms/Reconstruction/FittingFunctions/mitkBallStickFitter.h


  # MultishellProcessing
  include/Algorithms/Reconstruction/MultishellProcessing/itkRadialMultishellToSingleshellImageFilter.h
  include/Algorithms/Reconstruction/MultishellProcessing/itkDWIVoxelFunctor.h
  include/Algorithms/Reconstruction/MultishellProcessing/itkADCAverageFunctor.h
  include/Algorithms/Reconstruction/MultishellProcessing/itkKurtosisFitFunctor.h
  include/Algorithms/Reconstruction/MultishellProcessing/itkBiExpFitFunctor.h
  include/Algorithms/Reconstruction/MultishellProcessing/itkADCFitFunctor.h

  # Properties
  include/IODataStructures/Properties/mitkBValueMapProperty.h
  include/IODataStructures/Properties/mitkGradientDirectionsProperty.h
  include/IODataStructures/Properties/mitkMeasurementFrameProperty.h
  include/IODataStructures/Properties/mitkDiffusionPropertyHelper.h

  include/IODataStructures/DiffusionWeightedImages/mitkDiffusionImageTransformedCreationFilter.h

  # Algorithms
  include/Algorithms/itkDiffusionOdfGeneralizedFaImageFilter.h
  include/Algorithms/itkDiffusionOdfPrepareVisualizationImageFilter.h
  include/Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.h
  include/Algorithms/itkTensorDerivedMeasurementsFilter.h
  include/Algorithms/itkBrainMaskExtractionImageFilter.h
  include/Algorithms/itkB0ImageExtractionImageFilter.h
  include/Algorithms/itkB0ImageExtractionToSeparateImageFilter.h
  include/Algorithms/itkTensorImageToDiffusionImageFilter.h
  include/Algorithms/itkTensorToL2NormImageFilter.h
  include/Algorithms/itkGaussianInterpolateImageFunction.h
  include/Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.h
  include/Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.h
  include/Algorithms/itkDiffusionTensorPrincipalDirectionImageFilter.h
  include/Algorithms/itkCartesianToPolarVectorImageFilter.h
  include/Algorithms/itkPolarToCartesianVectorImageFilter.h
  include/Algorithms/itkDistanceMapFilter.h
  include/Algorithms/itkProjectionFilter.h
  include/Algorithms/itkResidualImageFilter.h
  include/Algorithms/itkExtractChannelFromRgbaImageFilter.h
  include/Algorithms/itkTensorReconstructionWithEigenvalueCorrectionFilter.h
  include/Algorithms/itkMergeDiffusionImagesFilter.h
  include/Algorithms/itkFiniteDiffOdfMaximaExtractionFilter.h
  include/Algorithms/itkShCoefficientImageImporter.h
  include/Algorithms/itkShCoefficientImageExporter.h
  include/Algorithms/itkOdfMaximaExtractionFilter.h
  include/Algorithms/itkResampleDwiImageFilter.h
  include/Algorithms/itkDwiGradientLengthCorrectionFilter.h
  include/Algorithms/itkAdcImageFilter.h
  include/Algorithms/itkDwiNormilzationFilter.h
  include/Algorithms/itkSplitDWImageFilter.h
  include/Algorithms/itkRemoveDwiChannelFilter.h
  include/Algorithms/itkExtractDwiChannelFilter.h
  include/Algorithms/itkFlipPeaksFilter.h
  include/Algorithms/itkShToOdfImageFilter.h

  include/Algorithms/Registration/mitkDWIHeadMotionCorrectionFilter.h
  include/Algorithms/itkNonLocalMeansDenoisingFilter.h
  include/Algorithms/itkVectorImageToImageFilter.h


)

set( TOOL_FILES
)

