set(CPP_FILES

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

  # DataStructures
  IODataStructures/mitkFiberTrackingObjectFactory.cpp

  # Rendering
  Rendering/mitkFiberBundleXMapper2D.cpp
  Rendering/mitkFiberBundleXMapper3D.cpp
  Rendering/mitkFiberBundleXThreadMonitorMapper3D.cpp
  #Rendering/mitkPlanarFigureMapper3D.cpp

  # Interactions
  Interactions/mitkFiberBundleInteractor.cpp

  # Algorithms
  Algorithms/mitkTractAnalyzer.cpp

  # Tractography
  Algorithms/GibbsTracking/mitkParticleGrid.cpp
  Algorithms/GibbsTracking/mitkMetropolisHastingsSampler.cpp
  Algorithms/GibbsTracking/mitkEnergyComputer.cpp
  Algorithms/GibbsTracking/mitkGibbsEnergyComputer.cpp
  Algorithms/GibbsTracking/mitkFiberBuilder.cpp
)

set(H_FILES
  # Rendering
  Rendering/mitkFiberBundleXMapper3D.h
  Rendering/mitkFiberBundleXMapper2D.h
  Rendering/mitkFiberBundleXThreadMonitorMapper3D.h
  #Rendering/mitkPlanarFigureMapper3D.h

  # DataStructures -> FiberBundleX
  IODataStructures/FiberBundleX/mitkFiberBundleX.h
  IODataStructures/FiberBundleX/mitkFiberBundleXWriter.h
  IODataStructures/FiberBundleX/mitkFiberBundleXReader.h
  IODataStructures/FiberBundleX/mitkFiberBundleXIOFactory.h
  IODataStructures/FiberBundleX/mitkFiberBundleXWriterFactory.h
  IODataStructures/FiberBundleX/mitkFiberBundleXSerializer.h
  IODataStructures/FiberBundleX/mitkFiberBundleXThreadMonitor.h

  # Algorithms
  Algorithms/itkTractDensityImageFilter.h
  Algorithms/itkTractsToFiberEndingsImageFilter.h
  Algorithms/itkTractsToRgbaImageFilter.h
  Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.h
  Algorithms/itkFibersFromPlanarFiguresFilter.h
  Algorithms/itkTractsToDWIImageFilter.h
  Algorithms/itkTractsToVectorImageFilter.h

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
  SignalModels/mitkStickModel.h
  SignalModels/mitkDiffusionNoiseModel.h
  SignalModels/mitkRicianNoiseModel.h
  SignalModels/mitkKspaceArtifact.h
  SignalModels/mitkGibbsRingingArtifact.h
  SignalModels/mitkT2SmearingArtifact.h
)