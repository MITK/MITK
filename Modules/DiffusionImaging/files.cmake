set(CPP_FILES

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
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImageSerializer.cpp

  # DataStructures -> QBall
  IODataStructures/QBallImages/mitkQBallImageSource.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageReader.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageWriter.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageIOFactory.cpp
  IODataStructures/QBallImages/mitkNrrdQBallImageWriterFactory.cpp
  IODataStructures/QBallImages/mitkQBallImage.cpp
  IODataStructures/QBallImages/mitkQBallImageSerializer.cpp

  # DataStructures -> Tensor
  IODataStructures/TensorImages/mitkTensorImageSource.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageReader.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageWriter.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageIOFactory.cpp
  IODataStructures/TensorImages/mitkNrrdTensorImageWriterFactory.cpp
  IODataStructures/TensorImages/mitkTensorImage.cpp
  IODataStructures/TensorImages/mitkTensorImageSerializer.cpp

# DataStructures -> FiberBundleX
  IODataStructures/FiberBundleX/mitkFiberBundleX.cpp
  IODataStructures/FiberBundleX/mitkFiberBundleXWriter.cpp
  IODataStructures/FiberBundleX/mitkFiberBundleXReader.cpp
  IODataStructures/FiberBundleX/mitkFiberBundleXIOFactory.cpp
  IODataStructures/FiberBundleX/mitkFiberBundleXWriterFactory.cpp
  IODataStructures/FiberBundleX/mitkFiberBundleXSerializer.cpp
  IODataStructures/FiberBundleX/mitkFiberBundleXThreadMonitor.cpp

  # DataStructures -> PlanarFigureComposite
  IODataStructures/PlanarFigureComposite/mitkPlanarFigureComposite.cpp

  # DataStructures -> Tbss
  IODataStructures/TbssImages/mitkTbssImageSource.cpp
  IODataStructures/TbssImages/mitkTbssRoiImageSource.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageReader.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageIOFactory.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageReader.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageIOFactory.cpp
  IODataStructures/TbssImages/mitkTbssImage.cpp
  IODataStructures/TbssImages/mitkTbssRoiImage.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriter.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriterFactory.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageWriter.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageWriterFactory.cpp
  IODataStructures/TbssImages/mitkTbssImporter.cpp

  # DataStructures Connectomics
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetwork.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkReader.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkIOFactory.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkSerializer.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkWriter.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkWriterFactory.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkDefinitions.cpp
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsConstantsManager.cpp

  # Rendering
  Rendering/vtkMaskedProgrammableGlyphFilter.cpp
  Rendering/mitkCompositeMapper.cpp
  Rendering/mitkVectorImageVtkGlyphMapper3D.cpp
  Rendering/vtkOdfSource.cxx
  Rendering/vtkThickPlane.cxx
  Rendering/mitkOdfNormalizationMethodProperty.cpp
  Rendering/mitkOdfScaleByProperty.cpp
  Rendering/mitkFiberBundleXMapper2D.cpp
  Rendering/mitkFiberBundleXMapper3D.cpp
  Rendering/mitkFiberBundleXThreadMonitorMapper3D.cpp
  Rendering/mitkTbssImageMapper.cpp
  Rendering/mitkPlanarFigureMapper3D.cpp

  # Rendering Connectomics
  Rendering/mitkConnectomicsNetworkMapper3D.cpp
  Rendering/mitkConnectomicsRenderingSchemeProperty.cpp
  Rendering/mitkConnectomicsRenderingEdgeFilteringProperty.cpp
  Rendering/mitkConnectomicsRenderingNodeFilteringProperty.cpp
  Rendering/mitkConnectomicsRenderingEdgeColorParameterProperty.cpp
  Rendering/mitkConnectomicsRenderingEdgeRadiusParameterProperty.cpp
  Rendering/mitkConnectomicsRenderingNodeColorParameterProperty.cpp
  Rendering/mitkConnectomicsRenderingNodeRadiusParameterProperty.cpp
  Rendering/mitkConnectomicsRenderingNodeThresholdParameterProperty.cpp
  Rendering/mitkConnectomicsRenderingEdgeThresholdParameterProperty.cpp

# Interactions
  Interactions/mitkFiberBundleInteractor.cpp

  # Algorithms
  Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.cpp
  Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.cpp
  Algorithms/mitkTractAnalyzer.cpp

  # Algorithms Connectomics
  Algorithms/Connectomics/mitkConnectomicsNetworkCreator.cpp
  Algorithms/Connectomics/mitkConnectomicsHistogramBase.cpp
  Algorithms/Connectomics/mitkConnectomicsDegreeHistogram.cpp
  Algorithms/Connectomics/mitkConnectomicsShortestPathHistogram.cpp
  Algorithms/Connectomics/mitkConnectomicsBetweennessHistogram.cpp
  Algorithms/Connectomics/mitkConnectomicsHistogramCache.cpp
  Algorithms/Connectomics/mitkConnectomicsSyntheticNetworkGenerator.cpp
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingPermutationBase.cpp
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingPermutationModularity.cpp
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingManager.cpp
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingCostFunctionBase.cpp
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingCostFunctionModularity.cpp
  Algorithms/Connectomics/itkConnectomicsNetworkToConnectivityMatrixImageFilter.cpp

  # Tractography
  Tractography/GibbsTracking/mitkParticleGrid.cpp
  Tractography/GibbsTracking/mitkMetropolisHastingsSampler.cpp
  Tractography/GibbsTracking/mitkEnergyComputer.cpp
  Tractography/GibbsTracking/mitkGibbsEnergyComputer.cpp
  Tractography/GibbsTracking/mitkFiberBuilder.cpp

  # Function Collection
  mitkDiffusionFunctionCollection.cpp
)

set(H_FILES
  # function Collection
  mitkDiffusionFunctionCollection.h

  # Rendering
  Rendering/mitkDiffusionImageMapper.h
  Rendering/mitkTbssImageMapper.h
  Rendering/mitkOdfVtkMapper2D.h
  Rendering/mitkFiberBundleXMapper3D.h
  Rendering/mitkFiberBundleXMapper2D.h
  Rendering/mitkFiberBundleXThreadMonitorMapper3D.h
  Rendering/mitkPlanarFigureMapper3D.h

  # Rendering Connectomics
  Rendering/mitkConnectomicsNetworkMapper3D.h
  Rendering/mitkConnectomicsRenderingProperties.h
  Rendering/mitkConnectomicsRenderingSchemeProperty.h
  Rendering/mitkConnectomicsRenderingEdgeFilteringProperty.h
  Rendering/mitkConnectomicsRenderingNodeFilteringProperty.h
  Rendering/mitkConnectomicsRenderingEdgeColorParameterProperty.h
  Rendering/mitkConnectomicsRenderingEdgeRadiusParameterProperty.h
  Rendering/mitkConnectomicsRenderingNodeColorParameterProperty.h
  Rendering/mitkConnectomicsRenderingNodeRadiusParameterProperty.h
  Rendering/mitkConnectomicsRenderingNodeThresholdParameterProperty.h
  Rendering/mitkConnectomicsRenderingEdgeThresholdParameterProperty.h

  # Reconstruction
  Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Reconstruction/mitkTeemDiffusionTensor3DReconstructionImageFilter.h
  Reconstruction/itkAnalyticalDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkDiffusionMultiShellQballReconstructionImageFilter.h
  Reconstruction/itkPointShell.h
  Reconstruction/itkOrientationDistributionFunction.h
  Reconstruction/itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h
  Reconstruction/itkRegularizedIVIMLocalVariationImageFilter.h
  Reconstruction/itkRegularizedIVIMReconstructionFilter.h
  Reconstruction/itkRegularizedIVIMReconstructionSingleIteration.h

  # IO Datastructures
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImage.h
  IODataStructures/TbssImages/mitkTbssImporter.h

  # DataStructures -> FiberBundleX
  IODataStructures/FiberBundleX/mitkFiberBundleX.h
  IODataStructures/FiberBundleX/mitkFiberBundleXWriter.h
  IODataStructures/FiberBundleX/mitkFiberBundleXReader.h
  IODataStructures/FiberBundleX/mitkFiberBundleXIOFactory.h
  IODataStructures/FiberBundleX/mitkFiberBundleXWriterFactory.h
  IODataStructures/FiberBundleX/mitkFiberBundleXSerializer.h
  IODataStructures/FiberBundleX/mitkFiberBundleXThreadMonitor.h

  # Datastructures Connectomics
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetwork.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkReader.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkIOFactory.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkSerializer.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkWriter.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkWriterFactory.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsNetworkDefinitions.h
  IODataStructures/ConnectomicsNetwork/mitkConnectomicsConstantsManager.h

  # Tractography
  Tractography/itkGibbsTrackingFilter.h
  Tractography/itkStochasticTractographyFilter.h
  Tractography/itkStreamlineTrackingFilter.h
  Tractography/GibbsTracking/mitkParticle.h
  Tractography/GibbsTracking/mitkParticleGrid.h
  Tractography/GibbsTracking/mitkMetropolisHastingsSampler.h
  Tractography/GibbsTracking/mitkSimpSamp.h
  Tractography/GibbsTracking/mitkEnergyComputer.h
  Tractography/GibbsTracking/mitkGibbsEnergyComputer.h
  Tractography/GibbsTracking/mitkSphereInterpolator.h
  Tractography/GibbsTracking/mitkFiberBuilder.h

  # Algorithms
  Algorithms/itkDiffusionQballGeneralizedFaImageFilter.h
  Algorithms/itkDiffusionQballPrepareVisualizationImageFilter.h
  Algorithms/itkTensorDerivedMeasurementsFilter.h
  Algorithms/itkBrainMaskExtractionImageFilter.h
  Algorithms/itkB0ImageExtractionImageFilter.h
  Algorithms/itkB0ImageExtractionToSeparateImageFilter.h
  Algorithms/itkTensorImageToDiffusionImageFilter.h
  Algorithms/itkTensorToL2NormImageFilter.h
  Algorithms/itkTractDensityImageFilter.h
  Algorithms/itkTractsToFiberEndingsImageFilter.h
  Algorithms/itkTractsToRgbaImageFilter.h
  Algorithms/itkGaussianInterpolateImageFunction.h
  Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.h
  Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.h
  Algorithms/itkDiffusionTensorPrincipalDirectionImageFilter.h
  Algorithms/itkCartesianToPolarVectorImageFilter.h
  Algorithms/itkPolarToCartesianVectorImageFilter.h
  Algorithms/itkDistanceMapFilter.h
  Algorithms/itkProjectionFilter.h
  Algorithms/itkSkeletonizationFilter.h
  Algorithms/itkResidualImageFilter.h
  Algorithms/itkExtractChannelFromRgbaImageFilter.h
  Algorithms/itkTensorReconstructionWithEigenvalueCorrectionFilter.h
  Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.h
  Algorithms/itkMergeDiffusionImagesFilter.h
  Algorithms/itkDwiPhantomGenerationFilter.h
  Algorithms/itkFiniteDiffOdfMaximaExtractionFilter.h
  Algorithms/itkMrtrixPeakImageConverter.h
  Algorithms/itkFslPeakImageConverter.h
  Algorithms/itkFslShCoefficientImageConverter.h
  Algorithms/itkOdfMaximaExtractionFilter.h
  Algorithms/itkFibersFromPlanarFiguresFilter.h
  Algorithms/itkTractsToDWIImageFilter.h
  Algorithms/itkTractsToVectorImageFilter.h
  Algorithms/itkResampleDwiImageFilter.h

  # Algorithms Connectomics
  Algorithms/Connectomics/mitkConnectomicsNetworkCreator.h
  Algorithms/Connectomics/mitkConnectomicsHistogramBase.h
  Algorithms/Connectomics/mitkConnectomicsDegreeHistogram.h
  Algorithms/Connectomics/mitkConnectomicsShortestPathHistogram.h
  Algorithms/Connectomics/mitkConnectomicsBetweennessHistogram.h
  Algorithms/Connectomics/mitkConnectomicsHistogramCache.h
  Algorithms/Connectomics/mitkConnectomicsSyntheticNetworkGenerator.h
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingPermutationBase.h
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingPermutationModularity.h
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingManager.h
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingCostFunctionBase.h
  Algorithms/Connectomics/mitkConnectomicsSimulatedAnnealingCostFunctionModularity.h
  Algorithms/Connectomics/itkConnectomicsNetworkToConnectivityMatrixImageFilter.h

  SignalModels/mitkDiffusionSignalModel.h
  SignalModels/mitkTensorModel.h
  SignalModels/mitkBallModel.h
  SignalModels/mitkStickModel.h
  SignalModels/mitkDiffusionNoiseModel.h
  SignalModels/mitkRicianNoiseModel.h
  SignalModels/mitkKspaceArtifact.h
  SignalModels/mitkGibbsRingingArtifact.h
  SignalModels/mitkT2SmearingArtifact.h
)

set( TOOL_FILES
)

if(WIN32)
endif(WIN32)

#MITK_MULTIPLEX_PICTYPE( Algorithms/mitkImageRegistrationMethod-TYPE.cpp )
