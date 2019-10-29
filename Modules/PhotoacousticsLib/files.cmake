SET(H_FILES
  include/mitkPAPropertyCalculator.h
  include/mitkPAVector.h
  include/mitkPATissueGeneratorParameters.h
  include/mitkPAInSilicoTissueVolume.h
  include/mitkPATissueGenerator.h
  include/mitkPAVesselTree.h
  include/mitkPAVessel.h
  include/mitkPAVesselMeanderStrategy.h
  include/mitkPANoiseGenerator.h
  include/mitkPAVolume.h
  include/mitkPAComposedVolume.h
  include/mitkPASlicedVolumeGenerator.h
  include/mitkPAProbe.h
  include/mitkPALightSource.h
  include/mitkPAIOUtil.h
  include/mitkPAMonteCarloThreadHandler.h
  include/mitkPASimulationBatchGenerator.h
  include/mitkPAFluenceYOffsetPair.h
  include/mitkPAVolumeManipulator.h
  include/mitkPAVesselProperties.h
  include/mitkPASimulationBatchGeneratorParameters.h
  include/mitkPAExceptions.h
  include/mitkPASpectralUnmixingFilterBase.h
  include/mitkPALinearSpectralUnmixingFilter.h
  include/mitkPASpectralUnmixingSO2.h
  include/mitkPASpectralUnmixingFilterLagrange.h
  include/mitkPASpectralUnmixingFilterSimplex.h
  include/mitkPASpectralUnmixingFilterVigra.h
)

set(CPP_FILES
  Domain/Vessel/mitkPAVesselTree.cpp
  Domain/Vessel/mitkPAVessel.cpp
  Domain/Vessel/mitkPAVesselMeanderStrategy.cpp
  Domain/Vessel/mitkPAVesselProperties.cpp
  Domain/Volume/mitkPAInSilicoTissueVolume.cpp
  Domain/Volume/mitkPAVolume.cpp
  Domain/Volume/mitkPAComposedVolume.cpp
  Domain/Volume/mitkPAFluenceYOffsetPair.cpp
  Generator/mitkPATissueGenerator.cpp
  Generator/mitkPANoiseGenerator.cpp
  Generator/mitkPASlicedVolumeGenerator.cpp
  Generator/mitkPASimulationBatchGenerator.cpp
  Generator/mitkPASimulationBatchGeneratorParameters.cpp
  IO/mitkPAIOUtil.cpp
  Utils/mitkPAPropertyCalculator.cpp
  Utils/mitkPAVector.cpp
  Utils/mitkPATissueGeneratorParameters.cpp
  Utils/mitkPAVolumeManipulator.cpp
  Utils/ProbeDesign/mitkPAProbe.cpp
  Utils/ProbeDesign/mitkPALightSource.cpp
  Utils/Thread/mitkPAMonteCarloThreadHandler.cpp
  SUFilter/mitkPASpectralUnmixingFilterBase.cpp
  SUFilter/mitkPALinearSpectralUnmixingFilter.cpp
  SUFilter/mitkPASpectralUnmixingSO2.cpp
  SUFilter/mitkPASpectralUnmixingFilterSimplex.cpp
  SUFilter/mitkPASpectralUnmixingFilterVigra.cpp
  SUFilter/mitkPASpectralUnmixingFilterLagrange.cpp
  Utils/mitkPAVesselDrawer.cpp
)

set(RESOURCE_FILES
  spectralLIB.dat
)
