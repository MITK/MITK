set(CPP_FILES

  mitkFiberTrackingModuleActivator.cpp

  ## IO datastructures
  IODataStructures/FiberBundleX/mitkFiberBundleX.cpp
  IODataStructures/FiberBundleX/mitkTrackvis.cpp
  IODataStructures/PlanarFigureComposite/mitkPlanarFigureComposite.cpp

  # Interactions
  Interactions/mitkFiberBundleInteractor.cpp

  # Tractography
  Algorithms/GibbsTracking/mitkParticleGrid.cpp
  Algorithms/GibbsTracking/mitkMetropolisHastingsSampler.cpp
  Algorithms/GibbsTracking/mitkEnergyComputer.cpp
  Algorithms/GibbsTracking/mitkGibbsEnergyComputer.cpp
  Algorithms/GibbsTracking/mitkFiberBuilder.cpp
  Algorithms/GibbsTracking/mitkSphereInterpolator.cpp
)

set(H_FILES
  # DataStructures -> FiberBundleX
  IODataStructures/FiberBundleX/mitkFiberBundleX.h
  IODataStructures/FiberBundleX/mitkTrackvis.h
  IODataStructures/mitkFiberfoxParameters.h

  # Algorithms
  Algorithms/itkTractDensityImageFilter.h
  Algorithms/itkTractsToFiberEndingsImageFilter.h
  Algorithms/itkTractsToRgbaImageFilter.h
  # moved to DiffusionCore
  #Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.h
  Algorithms/itkFibersFromPlanarFiguresFilter.h
  Algorithms/itkTractsToDWIImageFilter.h
  Algorithms/itkTractsToVectorImageFilter.h
  Algorithms/itkKspaceImageFilter.h
  Algorithms/itkDftImageFilter.h
  Algorithms/itkAddArtifactsToDwiImageFilter.h
  Algorithms/itkFieldmapGeneratorFilter.h
  Algorithms/itkEvaluateDirectionImagesFilter.h
  Algorithms/itkEvaluateTractogramDirectionsFilter.h

  # (old) Tractography
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

  # Signal Models
  SignalModels/mitkDiffusionSignalModel.h
  SignalModels/mitkTensorModel.h
  SignalModels/mitkBallModel.h
  SignalModels/mitkDotModel.h
  SignalModels/mitkAstroStickModel.h
  SignalModels/mitkStickModel.h
  SignalModels/mitkRawShModel.h
  SignalModels/mitkDiffusionNoiseModel.h
  SignalModels/mitkRicianNoiseModel.h
  SignalModels/mitkChiSquareNoiseModel.h
)

set(RESOURCE_FILES
  # Binary directory resources
  FiberTrackingLUTBaryCoords.bin
  FiberTrackingLUTIndices.bin

  # Shaders
  Shaders/mitkShaderFiberClipping.xml
)
