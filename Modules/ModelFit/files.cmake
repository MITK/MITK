file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Common/mitkExtractTimeGrid.cpp
  Common/mitkTimeGridHelper.cpp
  Common/mitkMaskedDynamicImageStatisticsGenerator.cpp
  Common/mitkModelFitConstants.cpp
  Common/mitkModelFitParameter.cpp
  Common/mitkModelFitCmdAppsHelper.cpp
  Common/mitkParameterFitImageGeneratorBase.cpp
  Common/mitkPixelBasedParameterFitImageGenerator.cpp
  Common/mitkROIBasedParameterFitImageGenerator.cpp
  Common/mitkModelFitInfo.cpp
  Common/mitkModelFitStaticParameterMap.cpp
  Common/mitkModelGenerator.cpp
  Common/mitkModelFitResultHelper.cpp
  Common/mitkScalarListLookupTable.cpp
  Common/mitkScalarListLookupTableProperty.cpp
  Common/mitkScalarListLookupTablePropertySerializer.cpp
  Common/mitkIModelFitProvider.cpp
  Common/mitkModelFitParameterValueExtraction.cpp
  Common/mitkBinaryImageToLabelSetImageFilter.cpp
  Common/mitkFormulaParser.cpp
  Common/mitkFresnel.cpp
  Common/mitkModelFitPlotDataHelper.cpp
  Common/mitkModelSignalImageGenerator.cpp
  Common/mitkModelFitResultRelationRule.cpp
  Functors/mitkSimpleFunctorBase.cpp
  Functors/mitkSimpleFunctorPolicy.cpp
  Functors/mitkChiSquareFitCostFunction.cpp
  Functors/mitkReducedChiSquareFitCostFunction.cpp
  Functors/mitkConstraintCheckerBase.cpp
  Functors/mitkSimpleBarrierConstraintChecker.cpp
  Functors/mitkSquaredDifferencesFitCostFunction.cpp
  Functors/mitkSumOfSquaredDifferencesFitCostFunction.cpp
  Functors/mitkMVConstrainedCostFunctionDecorator.cpp
  Functors/mitkMVModelFitCostFunction.cpp
  Functors/mitkNormalizedSumOfSquaredDifferencesFitCostFunction.cpp
  Functors/mitkSVModelFitCostFunction.cpp
  Functors/mitkModelFitFunctorBase.cpp
  Functors/mitkLevenbergMarquardtModelFitFunctor.cpp
  Functors/mitkDummyModelFitFunctor.cpp
  Functors/mitkModelFitInfoSignalGenerationFunctor.cpp
  Functors/mitkIndexedValueFunctorPolicy.cpp
  Functors/mitkModelDataGenerationFunctor.cpp
  Models/mitkModelBase.cpp
  Models/mitkModelFactoryBase.cpp
  Models/mitkModelParameterizerBase.cpp
  Models/mitkLinearModel.cpp
  Models/mitkLinearModelFactory.cpp
  Models/mitkInitialParameterizationDelegateBase.cpp
  Models/mitkImageBasedParameterizationDelegate.cpp
  Models/mitkGenericParamModel.cpp
  Models/mitkGenericParamModelFactory.cpp
  Models/mitkGenericParamModelParameterizer.cpp
  Models/mitkValueBasedParameterizationDelegate.cpp
  Models/mitkExponentialDecayModel.cpp
  Models/mitkExponentialDecayModelFactory.cpp
  Models/mitkExponentialDecayModelParameterizer.cpp
  Models/mitkExponentialSaturationModel.cpp
  Models/mitkExponentialSaturationModelFactory.cpp
  Models/mitkExponentialSaturationModelParameterizer.cpp
  Models/mitkExpDecayOffsetModel.cpp
  Models/mitkExpDecayOffsetModelFactory.cpp
  Models/mitkExpDecayOffsetModelParameterizer.cpp
  Models/mitkTwoStepLinearModel.cpp
  Models/mitkTwoStepLinearModelFactory.cpp
  Models/mitkTwoStepLinearModelParameterizer.cpp
  Models/mitkThreeStepLinearModel.cpp
  Models/mitkThreeStepLinearModelFactory.cpp
  Models/mitkThreeStepLinearModelParameterizer.cpp
  TestingHelper/mitkTestModel.cpp
  TestingHelper/mitkTestModelFactory.cpp
)

set(TPP_FILES
    include/itkMultiOutputNaryFunctorImageFilter.tpp
    include/itkMaskedStatisticsImageFilter.hxx
    include/itkMaskedNaryStatisticsImageFilter.hxx
	include/mitkModelFitProviderBase.tpp
)

set(HXX_FILES
)


set(MOC_H_FILES
)
