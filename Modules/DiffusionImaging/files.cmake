SET(CPP_FILES

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

  # DataStructures -> FiberBundle
  IODataStructures/FiberBundle/mitkFiberBundle.cpp
  IODataStructures/FiberBundle/mitkFiberBundleWriter.cpp
  IODataStructures/FiberBundle/mitkFiberBundleReader.cpp
  IODataStructures/FiberBundle/mitkFiberBundleIOFactory.cpp
  IODataStructures/FiberBundle/mitkFiberBundleWriterFactory.cpp
  IODataStructures/FiberBundle/mitkFiberBundleSerializer.cpp
  IODataStructures/FiberBundle/mitkParticle.cpp
  IODataStructures/FiberBundle/mitkParticleGrid.cpp

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
  IODataStructures/TbssImages/mitkNrrdTbssImageReader.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageIOFactory.cpp
  IODataStructures/TbssImages/mitkTbssImage.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriter.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriterFactory.cpp


  # Rendering
  Rendering/vtkMaskedProgrammableGlyphFilter.cpp
  Rendering/mitkCompositeMapper.cpp
  Rendering/mitkVectorImageVtkGlyphMapper3D.cpp
  Rendering/vtkOdfSource.cxx
  Rendering/vtkThickPlane.cxx
  Rendering/mitkOdfNormalizationMethodProperty.cpp
  Rendering/mitkOdfScaleByProperty.cpp
  Rendering/mitkFiberBundleMapper2D.cpp
  Rendering/mitkFiberBundleMapper3D.cpp
  Rendering/mitkFiberBundleXMapper3D.cpp
  Rendering/mitkFiberBundleXThreadMonitorMapper3D.cpp
  
# Interactions
  Interactions/mitkFiberBundleInteractor.cpp

  # Algorithms
  Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.cpp
  Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.cpp
)

SET(H_FILES
  # Rendering
  Rendering/mitkDiffusionImageMapper.h
  Rendering/mitkOdfVtkMapper2D.h
  Rendering/mitkFiberBundleMapper2D.h
  Rendering/mitkFiberBundleMapper3D.h
  Rendering/mitkFiberBundleXMapper3D.h
  Rendering/mitkFiberBundleXThreadMonitorMapper3D.h

  # Reconstruction
  Reconstruction/itkDiffusionQballReconstructionImageFilter.h
  Reconstruction/mitkTeemDiffusionTensor3DReconstructionImageFilter.h
  Reconstruction/itkAnalyticalDiffusionQballReconstructionImageFilter.h
  Reconstruction/itkPointShell.h
  Reconstruction/itkOrientationDistributionFunction.h

  # IO Datastructures
  IODataStructures/DiffusionWeightedImages/mitkDiffusionImage.h
  IODataStructures/FiberBundle/itkSlowPolyLineParametricPath.h
  IODataStructures/TbssImages/mitkTbssImage.h

  # DataStructures -> FiberBundleX
  IODataStructures/FiberBundleX/mitkFiberBundleX.h
  IODataStructures/FiberBundleX/mitkFiberBundleXWriter.h
  IODataStructures/FiberBundleX/mitkFiberBundleXReader.h
  IODataStructures/FiberBundleX/mitkFiberBundleXIOFactory.h
  IODataStructures/FiberBundleX/mitkFiberBundleXWriterFactory.h
  IODataStructures/FiberBundleX/mitkFiberBundleXSerializer.h
  IODataStructures/FiberBundleX/mitkFiberBundleXThreadMonitor.h



  # Tractography
  Tractography/itkGibbsTrackingFilter.h

  # Algorithms
  Algorithms/itkDiffusionQballGeneralizedFaImageFilter.h
  Algorithms/itkDiffusionQballPrepareVisualizationImageFilter.h
  Algorithms/itkTensorDerivedMeasurementsFilter.h
  Algorithms/itkBrainMaskExtractionImageFilter.h
  Algorithms/itkB0ImageExtractionImageFilter.h
  Algorithms/itkTensorImageToDiffusionImageFilter.h
  Algorithms/itkTensorToL2NormImageFilter.h
  Algorithms/itkTractsToProbabilityImageFilter.h
  Algorithms/itkTractsToDWIImageFilter.h
  Algorithms/itkTractsToFiberEndingsImageFilter.h
  Algorithms/itkGaussianInterpolateImageFunction.h
  Algorithms/mitkPartialVolumeAnalysisHistogramCalculator.h
  Algorithms/mitkPartialVolumeAnalysisClusteringCalculator.h
  Algorithms/itkDiffusionTensorPrincipleDirectionImageFilter.h
  Algorithms/itkCartesianToPolarVectorImageFilter.h
  Algorithms/itkPolarToCartesianVectorImageFilter.h
)

SET( TOOL_FILES
)

IF(WIN32)
ENDIF(WIN32)

#MITK_MULTIPLEX_PICTYPE( Algorithms/mitkImageRegistrationMethod-TYPE.cpp )
