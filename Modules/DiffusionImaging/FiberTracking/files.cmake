set(CPP_FILES

  mitkFiberTrackingModuleActivator.cpp

  ## IO datastructures
  IODataStructures/FiberBundle/mitkFiberBundle.cpp
  IODataStructures/FiberBundle/mitkTrackvis.cpp
  IODataStructures/PlanarFigureComposite/mitkPlanarFigureComposite.cpp

  # Interactions

  # Tractography
  Algorithms/GibbsTracking/mitkParticleGrid.cpp
  Algorithms/GibbsTracking/mitkMetropolisHastingsSampler.cpp
  Algorithms/GibbsTracking/mitkEnergyComputer.cpp
  Algorithms/GibbsTracking/mitkGibbsEnergyComputer.cpp
  Algorithms/GibbsTracking/mitkFiberBuilder.cpp
  Algorithms/GibbsTracking/mitkSphereInterpolator.cpp
)

set(H_FILES
  # DataStructures -> FiberBundle
  IODataStructures/FiberBundle/mitkFiberBundle.h
  IODataStructures/FiberBundle/mitkTrackvis.h
  IODataStructures/mitkFiberfoxParameters.h

  # Algorithms
  Algorithms/itkTractDensityImageFilter.h
  Algorithms/itkTractsToFiberEndingsImageFilter.h
  Algorithms/itkTractsToRgbaImageFilter.h
  Algorithms/itkTractsToVectorImageFilter.h
  Algorithms/itkEvaluateDirectionImagesFilter.h
  Algorithms/itkEvaluateTractogramDirectionsFilter.h
  Algorithms/itkFiberCurvatureFilter.h

  # Tractography
  Algorithms/itkGibbsTrackingFilter.h
  Algorithms/itkStochasticTractographyFilter.h
  Algorithms/itkStreamlineTrackingFilter.h
  Algorithms/GibbsTracking/mitkParticle.h
  Algorithms/GibbsTracking/mitkParticleGrid.h
  Algorithms/GibbsTracking/mitkMetropolisHastingsSampler.h
  Algorithms/GibbsTracking/mitkSimpSamp.h
  Algorithms/GibbsTracking/mitkEnergyComputer.h
  Algorithms/GibbsTracking/mitkGibbsEnergyComputer.h
  Algorithms/GibbsTracking/mitkSphereInterpolator.h
  Algorithms/GibbsTracking/mitkFiberBuilder.h
  Algorithms/MLTracking/mitkTrackingForestHandler.h
  Algorithms/MLTracking/itkMLBSTrackingFilter.h

  # Fiberfox
  Fiberfox/itkFibersFromPlanarFiguresFilter.h
  Fiberfox/itkTractsToDWIImageFilter.h
  Fiberfox/itkKspaceImageFilter.h
  Fiberfox/itkDftImageFilter.h
  Fiberfox/itkFieldmapGeneratorFilter.h

  Fiberfox/SignalModels/mitkDiffusionSignalModel.h
  Fiberfox/SignalModels/mitkTensorModel.h
  Fiberfox/SignalModels/mitkBallModel.h
  Fiberfox/SignalModels/mitkDotModel.h
  Fiberfox/SignalModels/mitkAstroStickModel.h
  Fiberfox/SignalModels/mitkStickModel.h
  Fiberfox/SignalModels/mitkRawShModel.h
  Fiberfox/SignalModels/mitkDiffusionNoiseModel.h
  Fiberfox/SignalModels/mitkRicianNoiseModel.h
  Fiberfox/SignalModels/mitkChiSquareNoiseModel.h

  Fiberfox/Sequences/mitkAcquisitionType.h
  Fiberfox/Sequences/mitkSingleShotEpi.h
  Fiberfox/Sequences/mitkCartesianReadout.h
)

set(RESOURCE_FILES
  # Binary directory resources
  FiberTrackingLUTBaryCoords.bin
  FiberTrackingLUTIndices.bin

  # Shaders
  Shaders/mitkShaderFiberClipping.xml
)
