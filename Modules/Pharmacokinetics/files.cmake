file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Common/mitkAterialInputFunctionGenerator.cpp
  Common/mitkAIFParametrizerHelper.cpp
  Common/mitkConcentrationCurveGenerator.cpp
  Common/mitkDescriptionParameterImageGeneratorBase.cpp
  Common/mitkPixelBasedDescriptionParameterImageGenerator.cpp
  DescriptionParameters/mitkCurveDescriptionParameterBase.cpp
  DescriptionParameters/mitkAreaUnderTheCurveDescriptionParameter.cpp
  DescriptionParameters/mitkAreaUnderFirstMomentDescriptionParameter.cpp
  DescriptionParameters/mitkMeanResidenceTimeDescriptionParameter.cpp
  DescriptionParameters/mitkTimeToPeakCurveDescriptionParameter.cpp
  DescriptionParameters/mitkMaximumCurveDescriptionParameter.cpp
  Functors/mitkCurveParameterFunctor.cpp
  Models/mitkAIFBasedModelBase.cpp
  Models/mitkDescriptivePharmacokineticBrixModel.cpp
  Models/mitkDescriptivePharmacokineticBrixModelFactory.cpp
  Models/mitkDescriptivePharmacokineticBrixModelValueBasedParameterizer.cpp
  Models/mitkDescriptivePharmacokineticBrixModelParameterizer.cpp
  Models/mitkThreeStepLinearModel.cpp
  Models/mitkThreeStepLinearModelFactory.cpp
  Models/mitkThreeStepLinearModelParameterizer.cpp
  Models/mitkTwoCompartmentExchangeModel.cpp
  Models/mitkTwoCompartmentExchangeModelFactory.cpp
  Models/mitkTwoCompartmentExchangeModelParameterizer.cpp
  Models/mitkNumericTwoCompartmentExchangeModel.cpp
  Models/mitkNumericTwoCompartmentExchangeModelFactory.cpp
  Models/mitkNumericTwoCompartmentExchangeModelParameterizer.cpp
  Models/mitkExtendedToftsModel.cpp
  Models/mitkExtendedToftsModelFactory.cpp
  Models/mitkExtendedToftsModelParameterizer.cpp
  Models/mitkStandardToftsModel.cpp
  Models/mitkStandardToftsModelFactory.cpp
  Models/mitkStandardToftsModelParameterizer.cpp
  Models/mitkOneTissueCompartmentModel.cpp
  Models/mitkOneTissueCompartmentModelFactory.cpp
  Models/mitkOneTissueCompartmentModelParameterizer.cpp
  Models/mitkExtendedOneTissueCompartmentModel.cpp
  Models/mitkExtendedOneTissueCompartmentModelFactory.cpp
  Models/mitkExtendedOneTissueCompartmentModelParameterizer.cpp
  Models/mitkTwoTissueCompartmentModel.cpp
  Models/mitkTwoTissueCompartmentModelFactory.cpp
  Models/mitkTwoTissueCompartmentModelParameterizer.cpp
  Models/mitkTwoTissueCompartmentFDGModel.cpp
  Models/mitkTwoTissueCompartmentFDGModelFactory.cpp
  Models/mitkTwoTissueCompartmentFDGModelParameterizer.cpp
  Models/mitkNumericTwoTissueCompartmentModel.cpp
  Models/mitkNumericTwoTissueCompartmentModelFactory.cpp
  Models/mitkNumericTwoTissueCompartmentModelParameterizer.cpp  
  SimulationFramework/mitkImageGenerationHelper.cpp
)

set(HXX_FILES
mitkDICOMSegmentationConstants.h

)


set(MOC_H_FILES
)



