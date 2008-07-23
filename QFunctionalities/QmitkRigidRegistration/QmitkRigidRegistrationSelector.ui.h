/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include "mitkMetricParameters.h"
#include "mitkOptimizerParameters.h"
#include "mitkTransformParameters.h"
#include "qvalidator.h"
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include "mitkMatrixConvert.h"
#include "itkVersorTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkSimilarity3DTransform.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "QmitkLoadPresetDialog.h"


/// this method starts the registration process
void QmitkRigidRegistrationSelector::CalculateTransformation()
{
  if (m_FixedNode != NULL && m_MovingNode != NULL)
  {
    emit AddNewTransformationToUndoList();
    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    m_ImageGeometry = m_MovingNode->GetData()->GetGeometry()->Clone();
    m_MovingGeometry = m_MovingNode->GetData()->GetGeometry();
    m_GeometryWorldToItkPhysicalTransform = mitk::Geometry3D::TransformType::New();
    GetWorldToItkPhysicalTransform(m_MovingGeometry, m_GeometryWorldToItkPhysicalTransform.GetPointer());
    m_GeometryItkPhysicalToWorldTransform = mitk::Geometry3D::TransformType::New();
    m_GeometryWorldToItkPhysicalTransform->GetInverse(m_GeometryItkPhysicalToWorldTransform);
    mitk::ImageRegistrationMethod::Pointer registration = mitk::ImageRegistrationMethod::New();
    itk::ReceptorMemberCommand<QmitkRigidRegistrationSelector>::Pointer command = itk::ReceptorMemberCommand<QmitkRigidRegistrationSelector>::New();
    command->SetCallbackFunction(this, &QmitkRigidRegistrationSelector::SetOptimizerValue);
    int observer = m_Observer->AddObserver( itk::AnyEvent(), command );
    registration->SetObserver(m_Observer);
    /// set the selected Interpolator
    registration->SetInterpolator(m_InterpolatorBox->currentItem());
    registration->SetReferenceImage(fimage);
    registration->SetInput(mimage);
    this->setTransformParameters();
    registration->SetTransformParameters(m_TransformParameters);
    this->setMetricParameters();
    registration->SetMetricParameters(m_MetricParameters);
    this->setOptimizerParameters();
    registration->SetOptimizerParameters(m_OptimizerParameters);
    registration->Update();
    m_Observer->RemoveObserver(observer);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistrationSelector::SetFixedNode( mitk::DataTreeNode * fixedNode )
{
  m_FixedNode = fixedNode;
  this->TransformSelected(m_TransformBox->currentItem());
}

void QmitkRigidRegistrationSelector::SetFixedDimension( int dimension )
{
  m_FixedDimension = dimension;
}

void QmitkRigidRegistrationSelector::SetMovingNode( mitk::DataTreeNode * movingNode )
{
  m_MovingNode = movingNode;
  this->TransformSelected(m_TransformBox->currentItem());
}

void QmitkRigidRegistrationSelector::SetMovingDimension(int dimension )
{
  m_MovingDimension = dimension;
}


/// this method initializes all variables and all line edits get their appropriate validator for int or float
void QmitkRigidRegistrationSelector::init()
{
  m_OptimizerParameters = NULL;
  m_TransformParameters = NULL;
  m_MetricParameters = NULL;
  m_FixedNode = NULL;
  m_FixedDimension = 0;
  m_MovingNode = NULL;
  m_MovingDimension = 0;

  m_Preset = new mitk::RigidRegistrationPreset();
  m_Preset->LoadPreset();

  /// for optimizer
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_NumberOfStepsExhaustive->setValidator(validatorLineEditInput);
  m_IterationsGradientDescent->setValidator(validatorLineEditInput);
  m_IterationsQuaternionRigidTransformGradientDescent->setValidator(validatorLineEditInput);
  m_EpsilonOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_InitialRadiusOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_IterationsOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_IterationsPowell->setValidator(validatorLineEditInput);
  m_IterationsFRPR->setValidator(validatorLineEditInput);
  m_IterationsRegularStepGradientDescent->setValidator(validatorLineEditInput);
  m_IterationsVersorTransform->setValidator(validatorLineEditInput);
  m_IterationsAmoeba->setValidator(validatorLineEditInput);
  m_IterationsLBFGS->setValidator(validatorLineEditInput);
  m_MinimumNumberOfIterationsSPSA->setValidator(validatorLineEditInput);
  m_NumberOfPerturbationsSPSA->setValidator(validatorLineEditInput);
  m_IterationsSPSA->setValidator(validatorLineEditInput);
  m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->setValidator(validatorLineEditInput);
  m_NumberOfHistogramBinsCorrelationCoefficientHistogram->setValidator(validatorLineEditInput);
  m_NumberOfHistogramBinsMeanSquaresHistogram->setValidator(validatorLineEditInput);
  m_NumberOfHistogramBinsMutualInformationHistogram->setValidator(validatorLineEditInput);
  m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->setValidator(validatorLineEditInput);
  m_NumberOfSpatialSamplesMattesMutualInformation->setValidator(validatorLineEditInput);
  m_NumberOfHistogramBinsMattesMutualInformation->setValidator(validatorLineEditInput);
  m_LambdaMeanReciprocalSquareDifference->setValidator(validatorLineEditInput);
  m_NumberOfSpatialSamplesMutualInformation->setValidator(validatorLineEditInput);

  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_StepLengthExhaustive->setValidator(validatorLineEditInputFloat);
  m_LearningRateGradientDescent->setValidator(validatorLineEditInputFloat);
  m_LearningRateQuaternionRigidTransformGradientDescent->setValidator(validatorLineEditInputFloat);
  m_ShrinkFactorOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_GrowthFactorOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_EpsilonOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_InitialRadiusOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_IterationsOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_StepLengthPowell->setValidator(validatorLineEditInputFloat);
  m_StepTolerancePowell->setValidator(validatorLineEditInputFloat);
  m_ValueTolerancePowell->setValidator(validatorLineEditInputFloat);
  m_StepLengthFRPR->setValidator(validatorLineEditInputFloat);
  m_GradientMagnitudeToleranceRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  m_MinimumStepLengthRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  m_MaximumStepLengthRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  QValidator* validatorLineEditInputFloat0to1 = new QDoubleValidator(0.000001, 0.999999, 8, this);
  m_RelaxationFactorRegularStepGradientDescent->setValidator(validatorLineEditInputFloat0to1);
  m_GradientMagnitudeToleranceVersorTransform->setValidator(validatorLineEditInputFloat);
  m_MinimumStepLengthVersorTransform->setValidator(validatorLineEditInputFloat);
  m_MaximumStepLengthVersorTransform->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba1->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba2->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba3->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba4->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba5->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba6->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba7->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba8->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba9->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba10->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba11->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba12->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba13->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba14->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba15->setValidator(validatorLineEditInputFloat);
  m_SimplexDeltaAmoeba16->setValidator(validatorLineEditInputFloat);
  m_ParametersConvergenceToleranceAmoeba->setValidator(validatorLineEditInputFloat);
  m_FunctionConvergenceToleranceAmoeba->setValidator(validatorLineEditInputFloat);
  m_GradientConvergenceToleranceLBFGS->setValidator(validatorLineEditInputFloat);
  m_LineSearchAccuracyLBFGS->setValidator(validatorLineEditInputFloat);
  m_DefaultStepLengthLBFGS->setValidator(validatorLineEditInputFloat);
  m_aSPSA->setValidator(validatorLineEditInputFloat);
  m_ASPSA->setValidator(validatorLineEditInputFloat);
  m_AlphaSPSA->setValidator(validatorLineEditInputFloat);
  m_cSPSA->setValidator(validatorLineEditInputFloat);
  m_GammaSPSA->setValidator(validatorLineEditInputFloat);
  m_ToleranceSPSA->setValidator(validatorLineEditInputFloat);
  m_StateOfConvergenceDecayRateSPSA->setValidator(validatorLineEditInputFloat);
  m_FixedImageStandardDeviationMutualInformation->setValidator(validatorLineEditInputFloat);
  m_MovingImageStandardDeviationMutualInformation->setValidator(validatorLineEditInputFloat);
  m_FixedSmootherVarianceMutualInformation->setValidator(validatorLineEditInput);
  m_MovingSmootherVarianceMutualInformation->setValidator(validatorLineEditInput);
  m_GradientMagnitudeToleranceVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);
  m_MinimumStepLengthVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);
  m_MaximumStepLengthVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);
  m_IterationsVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);

  // for Transformations
  m_ScalesTranslationTransformTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesTranslationTransformTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesTranslationTransformTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleTransformScaleX->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleTransformScaleY->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleTransformScaleZ->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleLogarithmicTransformScaleX->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleLogarithmicTransformScaleY->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleLogarithmicTransformScaleZ->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale4->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale5->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale6->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale7->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale8->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale9->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale10->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale11->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale12->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScale13->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesAffineTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale4->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale5->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale6->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale7->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale8->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale9->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale10->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale11->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale12->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScale13->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale5->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale6->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale7->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale8->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScale9->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesEuler3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesEuler3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesEuler3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesEuler3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesEuler3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesEuler3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredEuler3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredEuler3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredEuler3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredEuler3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredEuler3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredEuler3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScale4->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesQuaternionRigidTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesVersorRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale7->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale8->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale9->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale10->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale11->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale12->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale13->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale14->setValidator(validatorLineEditInputFloat);
  m_ScalesScaleSkewVersorRigid3DTransformScale15->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_ScalesSimilarity3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid2DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid2DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesRigid2DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_AngleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_RotationScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_CenterXScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_CenterYScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_TranslationXScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_TranslationYScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_RotationScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_TranslationXScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_TranslationYScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_ScalingScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_RotationScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_TranslationXScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_TranslationYScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredSimilarity2DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredSimilarity2DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredSimilarity2DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredSimilarity2DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredSimilarity2DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_ScalesCenteredSimilarity2DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);

  m_Observer = mitk::RigidRegistrationObserver::New();
  m_TransformFrame->setEnabled(true);
  m_MetricFrame->setEnabled(true);
  m_OptimizerFrame->setEnabled(true);
  m_InterpolatorFrame->setEnabled(true);
  m_TransformFrame->hide();
  m_MetricFrame->hide();
  m_OptimizerFrame->hide();
  m_InterpolatorFrame->hide();

  /// hide all views
  this->hideAllTransformFrames();
  this->hideAllOptimizerFrames();
  this->hideAllMetricFrames();
  /// and show the selected views
  this->TransformSelected(m_TransformBox->currentItem());
  this->MetricSelected(m_MetricBox->currentItem());
  this->OptimizerSelected(m_OptimizerBox->currentItem());
}

/// method to make all TransformFrames invisible
void QmitkRigidRegistrationSelector::hideAllTransformFrames()
{
  m_TranslationTransformFrame->hide();
  m_ScaleTransformFrame->hide();
  m_ScaleLogarithmicTransformFrame->hide();
  m_AffineTransformFrame->hide();
  m_FixedCenterOfRotationAffineTransformFrame->hide();
  m_Rigid3DTransformFrame->hide();
  m_Euler3DTransformFrame->hide();
  m_CenteredEuler3DTransformFrame->hide();
  m_QuaternionRigidTransformFrame->hide();
  m_VersorFrame->hide();
  m_VersorRigid3DTransformFrame->hide();
  m_ScaleSkewVersorRigid3DTransformFrame->hide();
  m_Similarity3DTransformFrame->hide();
  m_Rigid2DTransformFrame->hide();
  m_CenteredRigid2DTransformFrame->hide();
  m_Euler2DTransformFrame->hide();
  m_Similarity2DTransformFrame->hide();
  m_CenteredSimilarity2DTransformFrame->hide();
}

/// method to make all OptimizerFrames invisible
void QmitkRigidRegistrationSelector::hideAllOptimizerFrames()
{
  m_ExhaustiveFrame->hide();
  m_GradientDescentFrame->hide();
  m_QuaternionRigidTransformGradientDescentFrame->hide();
  m_LBFGSBFrame->hide();
  m_OnePlusOneEvolutionaryFrame->hide();
  m_PowellFrame->hide();
  m_FRPRFrame->hide();
  m_RegularStepGradientDescentFrame->hide();
  m_VersorTransformFrame->hide();
  m_AmoebaFrame->hide();
  m_ConjugateGradientFrame->hide();
  m_LBFGSFrame->hide();
  m_SPSAFrame->hide();
  m_VersorRigid3DFrame->hide();
}

/// method to make all MetricFrames invisible
void QmitkRigidRegistrationSelector::hideAllMetricFrames()
{
  m_MeanSquaresFrame->hide();
  m_NormalizedCorrelationFrame->hide();
  m_GradientDifferenceFrame->hide();
  m_KullbackLeiblerCompareHistogramFrame->hide();
  m_CorrelationCoefficientHistogramFrame->hide();
  m_MeanSquaresHistogramFrame->hide();
  m_MutualInformationHistogramFrame->hide();
  m_NormalizedMutualInformationHistogramFrame->hide();
  m_MattesMutualInformationFrame->hide();
  m_MeanReciprocalSquareDifferenceFrame->hide();
  m_MutualInformationFrame->hide();
  m_MatchCardinalityFrame->hide();
  m_KappaStatisticFrame->hide();
}

//// this method will be called with every progress step of the optimizer, the geometry of the moving image will be changed accordingly
//// to the transform parameters, which will be delivered by mitkRigidRgistrationObserver.cpp
void QmitkRigidRegistrationSelector::SetOptimizerValue( const itk::EventObject & )
{
  //mitk::TransformParameters* transformParameters = mitk::TransformParameters::GetInstance();
  double value = m_Observer->GetCurrentOptimizerValue();
  itk::Array<double> translateVector = m_Observer->GetCurrentTranslation();
  vtkMatrix4x4* vtkmatrix = vtkMatrix4x4::New();
  vtkmatrix->Identity();
  vtkTransform* vtktransform = vtkTransform::New();
  if (m_MovingNode != NULL)
  {
    if (m_TransformBox->currentItem() == mitk::TransformParameters::TRANSLATIONTRANSFORM)
    {
      if (translateVector.size() == 2)
      {
        vtktransform->Translate(translateVector[0], translateVector[1], 0);
      }
      else if (translateVector.size() == 3)
      {
        vtktransform->Translate(translateVector[0], translateVector[1], translateVector[2]);
      }
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALETRANSFORM)
    {
      for( unsigned int i=0; i<translateVector.size(); i++)
      {
        vtkmatrix->SetElement(i, i, translateVector[i]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
    {
      for( unsigned int i=0; i<translateVector.size(); i++)
      {
        vtkmatrix->SetElement(i, i, translateVector[i]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::AFFINETRANSFORM)
    {
      int m = 0;
      for (int i = 0; i < m_FixedDimension; i++)
      {
        for (int j = 0; j < m_FixedDimension; j++)
        {
          vtkmatrix->SetElement(i, j, translateVector[m]);
          m++;
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      if (m_FixedDimension == 2)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[4]);
        vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[5]);
      }
      else if (m_FixedDimension == 3)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[9]);
        vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[10]);
        vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterY() + translateVector[11]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      int m = 0;
      for (int i = 0; i < m_FixedDimension; i++)
      {
        for (int j = 0; j < m_FixedDimension; j++)
        {
          vtkmatrix->SetElement(i, j, translateVector[m]);
          m++;
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      if (m_FixedDimension == 2)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[4]);
        vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[5]);
      }
      else if (m_FixedDimension == 3)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[9]);
        vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[10]);
        vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterY() + translateVector[11]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID3DTRANSFORM)
    {
      int m = 0;
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, translateVector[m]);
          m++;
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[9]);
      vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[10]);
      vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterY() + translateVector[11]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER3DTRANSFORM)
    {
      mitk::ScalarType angleX = translateVector[0] * 45.0 / atan(1.0);
      mitk::ScalarType angleY = translateVector[1] * 45.0 / atan(1.0);
      mitk::ScalarType angleZ = translateVector[2] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Translate(-m_TransformParameters->GetTransformCenterX(), -m_TransformParameters->GetTransformCenterY(), -m_TransformParameters->GetTransformCenterZ());
      vtktransform->RotateX(angleX);
      vtktransform->RotateY(angleY);
      vtktransform->RotateZ(angleZ);
      vtktransform->Translate(m_TransformParameters->GetTransformCenterX(), m_TransformParameters->GetTransformCenterY(), m_TransformParameters->GetTransformCenterZ());
      vtktransform->Translate(translateVector[3], translateVector[4], translateVector[5]);
      vtktransform->PreMultiply();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
    {
      mitk::ScalarType angleX = translateVector[0] * 45.0 / atan(1.0);
      mitk::ScalarType angleY = translateVector[1] * 45.0 / atan(1.0);
      mitk::ScalarType angleZ = translateVector[2] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Translate(-m_TransformParameters->GetTransformCenterX(), -m_TransformParameters->GetTransformCenterY(), -m_TransformParameters->GetTransformCenterZ());
      vtktransform->RotateX(angleX);
      vtktransform->RotateY(angleY);
      vtktransform->RotateZ(angleZ);
      vtktransform->Translate(m_TransformParameters->GetTransformCenterX(), m_TransformParameters->GetTransformCenterY(), m_TransformParameters->GetTransformCenterZ());
      vtktransform->Translate(translateVector[3], translateVector[4], translateVector[5]);
      vtktransform->PreMultiply();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
    {
      itk::QuaternionRigidTransform<double>::Pointer quaternionTransform = itk::QuaternionRigidTransform<double>::New();
      quaternionTransform->SetParameters(translateVector);
      itk::Matrix<double, 3, 3> Matrix = quaternionTransform->GetMatrix();
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, Matrix[i][j]);
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[4]);
      vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[5]);
      vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterZ() + translateVector[6]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORTRANSFORM)
    {
      itk::VersorTransform<double>::Pointer versorTransform = itk::VersorTransform<double>::New();
      versorTransform->SetParameters(translateVector);
      itk::Matrix<double, 3, 3> Matrix = versorTransform->GetMatrix();
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, Matrix[i][j]);
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX());
      vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY());
      vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterZ());
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      itk::VersorRigid3DTransform<double>::Pointer versorTransform = itk::VersorRigid3DTransform<double>::New();
      versorTransform->SetParameters(translateVector);
      itk::Matrix<double, 3, 3> Matrix = versorTransform->GetMatrix();
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, Matrix[i][j]);
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[3]);
      vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[4]);
      vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterZ() + translateVector[5]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      itk::ScaleSkewVersor3DTransform<double>::Pointer versorTransform = itk::ScaleSkewVersor3DTransform<double>::New();
      versorTransform->SetParameters(translateVector);
      itk::Matrix<double, 3, 3> Matrix = versorTransform->GetMatrix();
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, Matrix[i][j]);
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[3]);
      vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[4]);
      vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterZ() + translateVector[5]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
    {
      itk::Similarity3DTransform<double>::Pointer similarityTransform = itk::Similarity3DTransform<double>::New();
      similarityTransform->SetParameters(translateVector);
      itk::Matrix<double, 3, 3> Matrix = similarityTransform->GetMatrix();
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, Matrix[i][j]);
        }
      }
      float center[4];
      float translation[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      vtkmatrix->MultiplyPoint(center, translation);
      vtkmatrix->SetElement(0, 3, -translation[0] + m_TransformParameters->GetTransformCenterX() + translateVector[4]);
      vtkmatrix->SetElement(1, 3, -translation[1] + m_TransformParameters->GetTransformCenterY() + translateVector[5]);
      vtkmatrix->SetElement(2, 3, -translation[2] + m_TransformParameters->GetTransformCenterZ() + translateVector[6]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID2DTRANSFORM)
    {
      mitk::ScalarType angle = translateVector[0] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->RotateZ(angle);
      vtktransform->Translate(translateVector[1], translateVector[2], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {      
      mitk::ScalarType angle = translateVector[0] * 45.0 / atan(1.0);;
      vtktransform->PostMultiply();
      vtktransform->Translate(-translateVector[1], -translateVector[2], 0);
      vtktransform->RotateZ(angle);
      vtktransform->Translate(translateVector[1], translateVector[2], 0);
      vtktransform->Translate(translateVector[3], translateVector[4], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER2DTRANSFORM)
    {
      mitk::ScalarType angle = translateVector[0] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->RotateZ(angle);
      vtktransform->Translate(translateVector[1], translateVector[2], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
    {
      mitk::ScalarType angle = translateVector[1] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Scale(translateVector[0], translateVector[0], 1);
      vtktransform->RotateZ(angle);
      vtktransform->Translate(translateVector[2], translateVector[3], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      mitk::ScalarType angle = translateVector[1] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Translate(-translateVector[2], -translateVector[3], 0);
      vtktransform->Scale(translateVector[0], translateVector[0], 1);
      vtktransform->RotateZ(angle);
      vtktransform->Translate(translateVector[2], translateVector[3], 0);
      vtktransform->Translate(translateVector[4], translateVector[5], 0);
      vtktransform->PreMultiply();
    }
    vtktransform->GetInverse(vtkmatrix);
    m_MovingGeometry->GetIndexToWorldTransform()->SetIdentity();
    m_MovingGeometry->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    m_MovingGeometry->Compose(m_GeometryWorldToItkPhysicalTransform, 1);
    m_MovingGeometry->Compose(m_ImageGeometry->GetIndexToWorldTransform(), 1);
    m_MovingGeometry->Compose(m_GeometryItkPhysicalToWorldTransform, 0);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  emit OptimizerChanged(value);
}

/// this method is called whenever the combobox with the selectable transforms changes
/// responsible for showing the selected transformparameters
void QmitkRigidRegistrationSelector::TransformSelected( int transform )
{
  this->SetSimplexDeltaVisible();
  this->hideAllTransformFrames();
  emit TransformChanged();
  if (m_FixedNode == NULL || m_MovingNode == NULL)
  {
    return;
  }
  else if (m_FixedDimension != m_MovingDimension)
  {
    return;
  }
  else if (transform == mitk::TransformParameters::TRANSLATIONTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_ScalesTranslationTransformTranslationZ->hide();
      textLabel4_4_2->hide();
    }
    else if (m_FixedDimension == 3)
    {
      m_ScalesTranslationTransformTranslationZ->show();
      textLabel4_4_2->show();
    }
    m_TranslationTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::SCALETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_ScalesScaleTransformScaleZ->hide();
      textLabel3_5->hide();
    }
    else if (m_FixedDimension == 3)
    {
      m_ScalesScaleTransformScaleZ->show();
      textLabel3_5->show();
    }
    m_ScaleTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_ScalesScaleLogarithmicTransformScaleZ->hide();
      textLabel3_5_3->hide();
    }
    else if (m_FixedDimension == 3)
    {
      m_ScalesScaleLogarithmicTransformScaleZ->show();
      textLabel3_5_3->show();
    }
    m_ScaleLogarithmicTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::AFFINETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_ScalesAffineTransformScale5->hide();
      m_ScalesAffineTransformScale6->hide();
      m_ScalesAffineTransformScale7->hide();
      m_ScalesAffineTransformScale8->hide();
      m_ScalesAffineTransformScale9->hide();
      m_ScalesAffineTransformScale10->hide();
      m_ScalesAffineTransformScale11->hide();
      m_ScalesAffineTransformScale12->hide();
      m_ScalesAffineTransformScale13->hide();
      m_ScalesAffineTransformScaleTranslationZ->hide();
      textLabel2_7->hide();
      textLabel3_6->hide();
      textLabel4_4->hide();
      textLabel5_4->hide();
      textLabel6_4->hide();
      textLabel7_4->hide();
      textLabel8_4->hide();
      textLabel9_4->hide();
      textLabel10_3->hide();
      textLabel13_2->hide();
    }
    else if (m_FixedDimension == 3)
    {
      m_ScalesAffineTransformScale5->show();
      m_ScalesAffineTransformScale6->show();
      m_ScalesAffineTransformScale7->show();
      m_ScalesAffineTransformScale8->show();
      m_ScalesAffineTransformScale9->show();
      m_ScalesAffineTransformScale10->show();
      m_ScalesAffineTransformScale11->show();
      m_ScalesAffineTransformScale12->show();
      m_ScalesAffineTransformScale13->show();
      m_ScalesAffineTransformScaleTranslationZ->show();
      textLabel2_7->show();
      textLabel3_6->show();
      textLabel4_4->show();
      textLabel5_4->show();
      textLabel6_4->show();
      textLabel7_4->show();
      textLabel8_4->show();
      textLabel9_4->show();
      textLabel10_3->show();
      textLabel13_2->show();
    }
    m_AffineTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_ScalesFixedCenterOfRotationAffineTransformScale5->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale6->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale7->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale8->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale9->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale10->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale11->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale12->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScale13->hide();
      m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->hide();
      textLabel2_7_2_2->hide();
      textLabel3_6_2_2->hide();
      textLabel4_4_3_2->hide();
      textLabel5_4_2_2->hide();
      textLabel6_4_2_2->hide();
      textLabel7_4_2_2->hide();
      textLabel8_4_2_2->hide();
      textLabel9_4_2_2->hide();
      textLabel10_3_2_2->hide();
      textLabel13_2_2_2->hide();
    }
    else if (m_FixedDimension == 3)
    {
      m_ScalesFixedCenterOfRotationAffineTransformScale5->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale6->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale7->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale8->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale9->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale10->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale11->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale12->show();
      m_ScalesFixedCenterOfRotationAffineTransformScale13->show();
      m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->show();
      textLabel2_7_2_2->show();
      textLabel3_6_2_2->show();
      textLabel4_4_3_2->show();
      textLabel5_4_2_2->show();
      textLabel6_4_2_2->show();
      textLabel7_4_2_2->show();
      textLabel8_4_2_2->show();
      textLabel9_4_2_2->show();
      textLabel10_3_2_2->show();
      textLabel13_2_2_2->show();
    }
    m_FixedCenterOfRotationAffineTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_Rigid3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::EULER3DTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_Euler3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_CenteredEuler3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_QuaternionRigidTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::VERSORTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_VersorFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_VersorRigid3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_ScaleSkewVersorRigid3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    if (m_FixedDimension == 3)
    {
      m_Similarity3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::RIGID2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Rigid2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_CenteredRigid2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Euler2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Similarity2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_CenteredSimilarity2DTransformFrame->show();
    }
  }
}

/// this method is called whenever the combobox with the selectable optimizers changes
/// responsible for showing the selected optimizerparameters
void QmitkRigidRegistrationSelector::OptimizerSelected( int optimizer )
{
  this->SetSimplexDeltaVisible();
  this->hideAllOptimizerFrames();
  if (optimizer == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
  {
    m_ExhaustiveFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
  {
    m_GradientDescentFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
  {
    m_QuaternionRigidTransformGradientDescentFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::LBFGSBOPTIMIZER)
  {
    m_LBFGSBFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
  {
    m_OnePlusOneEvolutionaryFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::POWELLOPTIMIZER)
  {
    m_PowellFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::FRPROPTIMIZER)
  {
    m_FRPRFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
  {
    m_RegularStepGradientDescentFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
  {
    m_VersorTransformFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
  {
    m_AmoebaFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
  {
    m_ConjugateGradientFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::LBFGSOPTIMIZER)
  {
    m_LBFGSFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::SPSAOPTIMIZER)
  {
    m_SPSAFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
  {
    m_VersorRigid3DFrame->show();
  }
}

/// this method is called whenever the combobox with the selectable metrics changes
/// responsible for showing the selected metricparameters
void QmitkRigidRegistrationSelector::MetricSelected( int metric )
{
  this->hideAllMetricFrames();
  if (metric == mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC)
  {
    m_MeanSquaresFrame->show();
  }
  else if (metric == mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC)
  {
    m_NormalizedCorrelationFrame->show();
  }
  else if (metric == mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_GradientDifferenceFrame->show();
  }
  else if (metric == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_KullbackLeiblerCompareHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_CorrelationCoefficientHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MeanSquaresHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MutualInformationHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_NormalizedMutualInformationHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_MattesMutualInformationFrame->show();
  }
  else if (metric == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_MeanReciprocalSquareDifferenceFrame->show();
  }
  else if (metric == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_MutualInformationFrame->show();
  }
  else if (metric == mitk::MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC)
  {
    m_MatchCardinalityFrame->show();
  }
  else if (metric == mitk::MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
  {
    m_KappaStatisticFrame->show();
  }
}

/// this method writes the transform parameters from the selected transform into mitkTransformParameters class
/// so that the desired transform can be build up with these parameters
void QmitkRigidRegistrationSelector::setTransformParameters()
{
  itk::Array<double> m_Scales;
  m_TransformParameters = mitk::TransformParameters::New();
  m_TransformParameters->SetTransform(m_TransformBox->currentItem());
  if (m_TransformBox->currentItem() == mitk::TransformParameters::TRANSLATIONTRANSFORM)
  {
    if (m_UseOptimizerScalesTranslation->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_ScalesTranslationTransformTranslationX->text().toDouble();
      m_Scales[1] = m_ScalesTranslationTransformTranslationY->text().toDouble();
      m_Scales[2] = m_ScalesTranslationTransformTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALETRANSFORM)
  {
    if (m_UseOptimizerScalesScale->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_ScalesScaleTransformScaleX->text().toDouble();
      m_Scales[1] = m_ScalesScaleTransformScaleY->text().toDouble();
      m_Scales[2] = m_ScalesScaleTransformScaleZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    if (m_UseOptimizerScalesScaleLogarithmic->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_ScalesScaleLogarithmicTransformScaleX->text().toDouble();
      m_Scales[1] = m_ScalesScaleLogarithmicTransformScaleY->text().toDouble();
      m_Scales[2] = m_ScalesScaleLogarithmicTransformScaleZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::AFFINETRANSFORM)
  {
    if (m_UseOptimizerScalesAffine->isChecked())
    {
      m_Scales.SetSize(16);
      m_Scales[0] = m_ScalesAffineTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesAffineTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesAffineTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesAffineTransformScale4->text().toDouble();
      m_Scales[4] = m_ScalesAffineTransformScale5->text().toDouble();
      m_Scales[5] = m_ScalesAffineTransformScale6->text().toDouble();
      m_Scales[6] = m_ScalesAffineTransformScale7->text().toDouble();
      m_Scales[7] = m_ScalesAffineTransformScale8->text().toDouble();
      m_Scales[8] = m_ScalesAffineTransformScale9->text().toDouble();
      m_Scales[9] = m_ScalesAffineTransformScale10->text().toDouble();
      m_Scales[10] = m_ScalesAffineTransformScale11->text().toDouble();
      m_Scales[11] = m_ScalesAffineTransformScale12->text().toDouble();
      m_Scales[12] = m_ScalesAffineTransformScale13->text().toDouble();
      m_Scales[13] = m_ScalesAffineTransformScaleTranslationX->text().toDouble();
      m_Scales[14] = m_ScalesAffineTransformScaleTranslationY->text().toDouble();
      m_Scales[15] = m_ScalesAffineTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerAffine->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerAffine->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsAffine->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    if (m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked())
    {
      m_Scales.SetSize(16);
      m_Scales[0] = m_ScalesFixedCenterOfRotationAffineTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesFixedCenterOfRotationAffineTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesFixedCenterOfRotationAffineTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesFixedCenterOfRotationAffineTransformScale4->text().toDouble();
      m_Scales[4] = m_ScalesFixedCenterOfRotationAffineTransformScale5->text().toDouble();
      m_Scales[5] = m_ScalesFixedCenterOfRotationAffineTransformScale6->text().toDouble();
      m_Scales[6] = m_ScalesFixedCenterOfRotationAffineTransformScale7->text().toDouble();
      m_Scales[7] = m_ScalesFixedCenterOfRotationAffineTransformScale8->text().toDouble();
      m_Scales[8] = m_ScalesFixedCenterOfRotationAffineTransformScale9->text().toDouble();
      m_Scales[9] = m_ScalesFixedCenterOfRotationAffineTransformScale10->text().toDouble();
      m_Scales[10] = m_ScalesFixedCenterOfRotationAffineTransformScale11->text().toDouble();
      m_Scales[11] = m_ScalesFixedCenterOfRotationAffineTransformScale12->text().toDouble();
      m_Scales[12] = m_ScalesFixedCenterOfRotationAffineTransformScale13->text().toDouble();
      m_Scales[13] = m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->text().toDouble();
      m_Scales[14] = m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->text().toDouble();
      m_Scales[15] = m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerFixedCenterOfRotationAffine->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerFixedCenterOfRotationAffine->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsFixedCenterOfRotationAffine->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    if (m_UseOptimizerScalesRigid3D->isChecked())
    {
      m_Scales.SetSize(12);
      m_Scales[0] = m_ScalesRigid3DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesRigid3DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesRigid3DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesRigid3DTransformScale4->text().toDouble();
      m_Scales[4] = m_ScalesRigid3DTransformScale5->text().toDouble();
      m_Scales[5] = m_ScalesRigid3DTransformScale6->text().toDouble();
      m_Scales[6] = m_ScalesRigid3DTransformScale7->text().toDouble();
      m_Scales[7] = m_ScalesRigid3DTransformScale8->text().toDouble();
      m_Scales[8] = m_ScalesRigid3DTransformScale9->text().toDouble();
      m_Scales[9] = m_ScalesRigid3DTransformScaleTranslationX->text().toDouble();
      m_Scales[10] = m_ScalesRigid3DTransformScaleTranslationY->text().toDouble();
      m_Scales[11] = m_ScalesRigid3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerRigid3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerRigid3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsRigid3D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER3DTRANSFORM)
  {
    if (m_UseOptimizerScalesEuler3D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_ScalesEuler3DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesEuler3DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesEuler3DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesEuler3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_ScalesEuler3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_ScalesEuler3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerEuler3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerEuler3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsEuler3D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
  {
    if (m_UseOptimizerScalesCenteredEuler3D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_ScalesCenteredEuler3DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesCenteredEuler3DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesCenteredEuler3DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesCenteredEuler3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_ScalesCenteredEuler3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_ScalesCenteredEuler3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerCenteredEuler3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerCenteredEuler3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsCenteredEuler3D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
  {
    if (m_UseOptimizerScalesQuaternionRigid->isChecked())
    {
      m_Scales.SetSize(7);
      m_Scales[0] = m_ScalesQuaternionRigidTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesQuaternionRigidTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesQuaternionRigidTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesQuaternionRigidTransformScale4->text().toDouble();
      m_Scales[4] = m_ScalesQuaternionRigidTransformScaleTranslationX->text().toDouble();
      m_Scales[5] = m_ScalesQuaternionRigidTransformScaleTranslationY->text().toDouble();
      m_Scales[6] = m_ScalesQuaternionRigidTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerQuaternionRigid->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerQuaternionRigid->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsQuaternionRigid->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORTRANSFORM)
  {
    if (m_UseOptimizerScalesVersor->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_ScalesVersorTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesVersorTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesVersorTransformScale3->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerVersor->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerVersor->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsVersor->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
  {
    if (m_UseOptimizerScalesVersorRigid3D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_ScalesVersorRigid3DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesVersorRigid3DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesVersorRigid3DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesVersorRigid3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_ScalesVersorRigid3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_ScalesVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerVersorRigid3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerVersorRigid3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsVersorRigid3D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    if (m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked())
    {
      m_Scales.SetSize(15);
      m_Scales[0] = m_ScalesScaleSkewVersorRigid3DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesScaleSkewVersorRigid3DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesScaleSkewVersorRigid3DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      m_Scales[6] = m_ScalesScaleSkewVersorRigid3DTransformScale7->text().toDouble();
      m_Scales[7] = m_ScalesScaleSkewVersorRigid3DTransformScale8->text().toDouble();
      m_Scales[8] = m_ScalesScaleSkewVersorRigid3DTransformScale9->text().toDouble();
      m_Scales[9] = m_ScalesScaleSkewVersorRigid3DTransformScale10->text().toDouble();
      m_Scales[10] = m_ScalesScaleSkewVersorRigid3DTransformScale11->text().toDouble();
      m_Scales[11] = m_ScalesScaleSkewVersorRigid3DTransformScale12->text().toDouble();
      m_Scales[12] = m_ScalesScaleSkewVersorRigid3DTransformScale13->text().toDouble();
      m_Scales[13] = m_ScalesScaleSkewVersorRigid3DTransformScale14->text().toDouble();
      m_Scales[14] = m_ScalesScaleSkewVersorRigid3DTransformScale15->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerScaleSkewVersorRigid3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerScaleSkewVersorRigid3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsScaleSkewVersorRigid3D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    if (m_UseOptimizerScalesSimilarity3D->isChecked())
    {
      m_Scales.SetSize(7);
      m_Scales[0] = m_ScalesSimilarity3DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesSimilarity3DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesSimilarity3DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesSimilarity3DTransformScale4->text().toDouble();
      m_Scales[4] = m_ScalesSimilarity3DTransformScaleTranslationX->text().toDouble();
      m_Scales[5] = m_ScalesSimilarity3DTransformScaleTranslationY->text().toDouble();
      m_Scales[6] = m_ScalesSimilarity3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerSimilarity3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerSimilarity3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsSimilarity3D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID2DTRANSFORM)
  {
    if (m_UseOptimizerScalesRigid2D->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_ScalesRigid2DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesRigid2DTransformScaleTranslationX->text().toDouble();
      m_Scales[2] = m_ScalesRigid2DTransformScaleTranslationY->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerRigid2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerRigid2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsRigid2D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    if (m_UseOptimizerScalesCenteredRigid2D->isChecked())
    {
      m_Scales.SetSize(5);
      m_Scales[0] = m_RotationScaleCenteredRigid2D->text().toDouble();
      m_Scales[1] = m_CenterXScaleCenteredRigid2D->text().toDouble();
      m_Scales[2] = m_CenterYScaleCenteredRigid2D->text().toDouble();
      m_Scales[3] = m_TranslationXScaleCenteredRigid2D->text().toDouble();
      m_Scales[4] = m_TranslationYScaleCenteredRigid2D->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    m_TransformParameters->SetAngle(m_AngleCenteredRigid2D->text().toFloat());
    if (m_CenterForInitializerCenteredRigid2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerCenteredRigid2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsCenteredRigid2D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    if (m_UseOptimizerScalesEuler2D->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_RotationScaleEuler2D->text().toDouble();
      m_Scales[1] = m_TranslationXScaleEuler2D->text().toDouble();
      m_Scales[2] = m_TranslationYScaleEuler2D->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_CenterForInitializerEuler2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerEuler2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsEuler2D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    if (m_UseOptimizerScalesSimilarity2D->isChecked())
    {
      m_Scales.SetSize(4);
      m_Scales[0] = m_ScalingScaleSimilarity2D->text().toDouble();
      m_Scales[1] = m_RotationScaleSimilarity2D->text().toDouble();
      m_Scales[2] = m_TranslationXScaleSimilarity2D->text().toDouble();
      m_Scales[3] = m_TranslationYScaleSimilarity2D->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    m_TransformParameters->SetScale(m_InitialScaleSimilarity2D->text().toFloat());
    m_TransformParameters->SetAngle(m_AngleSimilarity2D->text().toFloat());
    if (m_CenterForInitializerSimilarity2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerSimilarity2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsSimilarity2D->isChecked());
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    if (m_UseOptimizerScalesCenteredSimilarity2D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_ScalesCenteredSimilarity2DTransformScale1->text().toDouble();
      m_Scales[1] = m_ScalesCenteredSimilarity2DTransformScale2->text().toDouble();
      m_Scales[2] = m_ScalesCenteredSimilarity2DTransformScale3->text().toDouble();
      m_Scales[3] = m_ScalesCenteredSimilarity2DTransformScale4->text().toDouble();
      m_Scales[4] = m_ScalesCenteredSimilarity2DTransformScaleTranslationX->text().toDouble();
      m_Scales[5] = m_ScalesCenteredSimilarity2DTransformScaleTranslationY->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    m_TransformParameters->SetScale(m_InitialScaleCenteredSimilarity2D->text().toFloat());
    m_TransformParameters->SetAngle(m_AngleCenteredSimilarity2D->text().toFloat());
    if (m_CenterForInitializerCenteredSimilarity2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_CenterForInitializerCenteredSimilarity2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_MomentsCenteredSimilarity2D->isChecked());
    }
  }
}

/// this method writes the optimizer parameters from the selected optimizer into mitkOptimizerParameters class
/// so that the desired optimizer can be build up with these parameters
void QmitkRigidRegistrationSelector::setOptimizerParameters()
{
  m_OptimizerParameters = mitk::OptimizerParameters::New();
  m_OptimizerParameters->SetOptimizer(m_OptimizerBox->currentItem());
  m_OptimizerParameters->SetDimension(m_FixedDimension);
  m_OptimizerParameters->SetMaximize(m_Maximize->isOn());
  if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
  {
    m_OptimizerParameters->SetStepLengthExhaustive(m_StepLengthExhaustive->text().toFloat());
    m_OptimizerParameters->SetNumberOfStepsExhaustive(m_NumberOfStepsExhaustive->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
  {
    m_OptimizerParameters->SetLearningRateGradientDescent(m_LearningRateGradientDescent->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsGradientDescent(m_IterationsGradientDescent->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
  {
    m_OptimizerParameters->SetLearningRateQuaternionRigidTransformGradientDescent(m_LearningRateQuaternionRigidTransformGradientDescent->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsQuaternionRigidTransformGradientDescent(m_IterationsQuaternionRigidTransformGradientDescent->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::LBFGSBOPTIMIZER)
  {
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
  {
    m_OptimizerParameters->SetShrinkFactorOnePlusOneEvolutionary(m_ShrinkFactorOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetGrowthFactorOnePlusOneEvolutionary(m_GrowthFactorOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetEpsilonOnePlusOneEvolutionary(m_EpsilonOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetInitialRadiusOnePlusOneEvolutionary(m_InitialRadiusOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsOnePlusOneEvolutionary(m_IterationsOnePlusOneEvolutionary->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::POWELLOPTIMIZER)
  {
    m_OptimizerParameters->SetStepLengthPowell(m_StepLengthPowell->text().toFloat());
    m_OptimizerParameters->SetStepTolerancePowell(m_StepTolerancePowell->text().toFloat());
    m_OptimizerParameters->SetValueTolerancePowell(m_ValueTolerancePowell->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsPowell(m_IterationsPowell->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::FRPROPTIMIZER)
  {
    m_OptimizerParameters->SetFletchReevesFRPR(m_FletchReevesFRPR->isOn());
    m_OptimizerParameters->SetPolakRibiereFRPR(m_PolakRibiereFRPR->isOn());
    m_OptimizerParameters->SetStepLengthFRPR(m_StepLengthFRPR->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsFRPR(m_IterationsFRPR->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientMagnitudeToleranceRegularStepGradientDescent(m_GradientMagnitudeToleranceRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetMinimumStepLengthRegularStepGradientDescent(m_MinimumStepLengthRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetMaximumStepLengthRegularStepGradientDescent(m_MaximumStepLengthRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetRelaxationFactorRegularStepGradientDescent(m_RelaxationFactorRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsRegularStepGradientDescent(m_IterationsRegularStepGradientDescent->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientMagnitudeToleranceVersorTransform(m_GradientMagnitudeToleranceVersorTransform->text().toFloat());
    m_OptimizerParameters->SetMinimumStepLengthVersorTransform(m_MinimumStepLengthVersorTransform->text().toFloat());
    m_OptimizerParameters->SetMaximumStepLengthVersorTransform(m_MaximumStepLengthVersorTransform->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsVersorTransform(m_IterationsVersorTransform->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
  {
    itk::Array<double> simplexDelta;
    simplexDelta.SetSize(16);
    simplexDelta[0] = m_SimplexDeltaAmoeba1->text().toDouble();
    simplexDelta[1] = m_SimplexDeltaAmoeba2->text().toDouble();
    simplexDelta[2] = m_SimplexDeltaAmoeba3->text().toDouble();
    simplexDelta[3] = m_SimplexDeltaAmoeba4->text().toDouble();
    simplexDelta[4] = m_SimplexDeltaAmoeba5->text().toDouble();
    simplexDelta[5] = m_SimplexDeltaAmoeba6->text().toDouble();
    simplexDelta[6] = m_SimplexDeltaAmoeba7->text().toDouble();
    simplexDelta[7] = m_SimplexDeltaAmoeba8->text().toDouble();
    simplexDelta[8] = m_SimplexDeltaAmoeba9->text().toDouble();
    simplexDelta[9] = m_SimplexDeltaAmoeba10->text().toDouble();
    simplexDelta[10] = m_SimplexDeltaAmoeba11->text().toDouble();
    simplexDelta[11] = m_SimplexDeltaAmoeba12->text().toDouble();
    simplexDelta[12] = m_SimplexDeltaAmoeba13->text().toDouble();
    simplexDelta[13] = m_SimplexDeltaAmoeba14->text().toDouble();
    simplexDelta[14] = m_SimplexDeltaAmoeba15->text().toDouble();
    simplexDelta[15] = m_SimplexDeltaAmoeba16->text().toDouble();
    m_OptimizerParameters->SetSimplexDeltaAmoeba(simplexDelta);
    m_OptimizerParameters->SetParametersConvergenceToleranceAmoeba(m_ParametersConvergenceToleranceAmoeba->text().toFloat());
    m_OptimizerParameters->SetFunctionConvergenceToleranceAmoeba(m_FunctionConvergenceToleranceAmoeba->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsAmoeba(m_IterationsAmoeba->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
  {
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::LBFGSOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientConvergenceToleranceLBFGS(m_GradientConvergenceToleranceLBFGS->text().toFloat());
    m_OptimizerParameters->SetLineSearchAccuracyLBFGS(m_LineSearchAccuracyLBFGS->text().toFloat());
    m_OptimizerParameters->SetDefaultStepLengthLBFGS(m_DefaultStepLengthLBFGS->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsLBFGS(m_IterationsLBFGS->text().toInt());
    m_OptimizerParameters->SetTraceOnLBFGS(m_TraceOnLBFGS->isOn());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::SPSAOPTIMIZER)
  {
    m_OptimizerParameters->SetaSPSA(m_aSPSA->text().toFloat());
    m_OptimizerParameters->SetASPSA(m_ASPSA->text().toFloat());
    m_OptimizerParameters->SetAlphaSPSA(m_AlphaSPSA->text().toFloat());
    m_OptimizerParameters->SetcSPSA(m_cSPSA->text().toFloat());
    m_OptimizerParameters->SetGammaSPSA(m_GammaSPSA->text().toFloat());
    m_OptimizerParameters->SetToleranceSPSA(m_ToleranceSPSA->text().toFloat());
    m_OptimizerParameters->SetStateOfConvergenceDecayRateSPSA(m_StateOfConvergenceDecayRateSPSA->text().toFloat());
    m_OptimizerParameters->SetMinimumNumberOfIterationsSPSA(m_MinimumNumberOfIterationsSPSA->text().toInt());
    m_OptimizerParameters->SetNumberOfPerturbationsSPSA(m_NumberOfPerturbationsSPSA->text().toInt());
    m_OptimizerParameters->SetNumberOfIterationsSPSA(m_IterationsSPSA->text().toInt());
  }
  else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientMagnitudeToleranceVersorRigid3DTransform(m_GradientMagnitudeToleranceVersorRigid3DTransform->text().toFloat());
    m_OptimizerParameters->SetMinimumStepLengthVersorRigid3DTransform(m_MinimumStepLengthVersorRigid3DTransform->text().toFloat());
    m_OptimizerParameters->SetMaximumStepLengthVersorRigid3DTransform(m_MaximumStepLengthVersorRigid3DTransform->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsVersorRigid3DTransform(m_IterationsVersorRigid3DTransform->text().toInt());
  }
}

/// this method writes the metric parameters from the selected metric into mitkMetricParameters class
/// so that the desired metric can be build up with these parameters
void QmitkRigidRegistrationSelector::setMetricParameters()
{
  m_MetricParameters = mitk::MetricParameters::New();
  m_MetricParameters->SetMetric(m_MetricBox->currentItem());
  m_MetricParameters->SetComputeGradient(m_ComputeGradient->isOn());
  if (m_MetricBox->currentItem() == mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsKullbackLeiblerCompareHistogram(m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsCorrelationCoefficientHistogram(m_NumberOfHistogramBinsCorrelationCoefficientHistogram->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsMeanSquaresHistogram(m_NumberOfHistogramBinsMeanSquaresHistogram->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsMutualInformationHistogram(m_NumberOfHistogramBinsMutualInformationHistogram->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsNormalizedMutualInformationHistogram(m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetUseSamplesMattesMutualInformation(m_UseSamplingMattesMutualInformation->isOn());
    m_MetricParameters->SetSpatialSamplesMattesMutualInformation(m_NumberOfSpatialSamplesMattesMutualInformation->text().toInt());
    m_MetricParameters->SetNumberOfHistogramBinsMattesMutualInformation(m_NumberOfHistogramBinsMattesMutualInformation->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetLambdaMeanReciprocalSquareDifference(m_LambdaMeanReciprocalSquareDifference->text().toInt());
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    mitk::Image* fixedImage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    unsigned int nPixel = 1;
    for (int i = 0; i < m_FixedDimension; i++)
    {
      nPixel *= fixedImage->GetDimension(i);
    }
    m_MetricParameters->SetSpatialSamplesMutualInformation(nPixel * m_NumberOfSpatialSamplesMutualInformation->text().toInt()/100);
    m_MetricParameters->SetFixedImageStandardDeviationMutualInformation(m_FixedImageStandardDeviationMutualInformation->text().toFloat());
    m_MetricParameters->SetMovingImageStandardDeviationMutualInformation(m_MovingImageStandardDeviationMutualInformation->text().toFloat());
    if (m_UseNormalizerAndSmoother)
    {
      m_MetricParameters->SetUseNormalizerAndSmootherMutualInformation(m_UseNormalizerAndSmoother->isChecked());
      m_MetricParameters->SetFixedSmootherVarianceMutualInformation(m_FixedSmootherVarianceMutualInformation->text().toFloat());
      m_MetricParameters->SetMovingSmootherVarianceMutualInformation(m_MovingSmootherVarianceMutualInformation->text().toFloat());
    }
    else
    {
      m_MetricParameters->SetUseNormalizerAndSmootherMutualInformation(m_UseNormalizerAndSmoother->isChecked());
    }
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_MetricBox->currentItem() == mitk::MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
  {
  }
}

/// this method is needed because the Amoeba optimizer needs a simplex delta parameter, which defines the search range for each transform parameter
/// every transform has its own set of parameters, so this method looks for every time showing the correct number of simplex delta input fields for the
/// Amoeba optimizer
void QmitkRigidRegistrationSelector::SetSimplexDeltaVisible()
{
  m_SimplexDeltaAmoebaLabel1->hide();
  m_SimplexDeltaAmoebaLabel2->hide();
  m_SimplexDeltaAmoebaLabel3->hide();
  m_SimplexDeltaAmoebaLabel4->hide();
  m_SimplexDeltaAmoebaLabel5->hide();
  m_SimplexDeltaAmoebaLabel6->hide();
  m_SimplexDeltaAmoebaLabel7->hide();
  m_SimplexDeltaAmoebaLabel8->hide();
  m_SimplexDeltaAmoebaLabel9->hide();
  m_SimplexDeltaAmoebaLabel10->hide();
  m_SimplexDeltaAmoebaLabel11->hide();
  m_SimplexDeltaAmoebaLabel12->hide();
  m_SimplexDeltaAmoebaLabel13->hide();
  m_SimplexDeltaAmoebaLabel14->hide();
  m_SimplexDeltaAmoebaLabel15->hide();
  m_SimplexDeltaAmoebaLabel16->hide();
  m_SimplexDeltaAmoeba1->hide();
  m_SimplexDeltaAmoeba2->hide();
  m_SimplexDeltaAmoeba3->hide();
  m_SimplexDeltaAmoeba4->hide();
  m_SimplexDeltaAmoeba5->hide();
  m_SimplexDeltaAmoeba6->hide();
  m_SimplexDeltaAmoeba7->hide();
  m_SimplexDeltaAmoeba8->hide();
  m_SimplexDeltaAmoeba9->hide();
  m_SimplexDeltaAmoeba10->hide();
  m_SimplexDeltaAmoeba11->hide();
  m_SimplexDeltaAmoeba12->hide();
  m_SimplexDeltaAmoeba13->hide();
  m_SimplexDeltaAmoeba14->hide();
  m_SimplexDeltaAmoeba15->hide();
  m_SimplexDeltaAmoeba16->hide();
  if (m_FixedNode == NULL || m_MovingNode == NULL)
  {
    return;
  }
  else if (m_FixedDimension != m_MovingDimension)
  {
    return;
  }
  if (m_TransformBox->currentItem() == mitk::TransformParameters::TRANSLATIONTRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::SCALETRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_SimplexDeltaAmoebaLabel1->show();
      m_SimplexDeltaAmoeba1->show();
      m_SimplexDeltaAmoebaLabel2->show();
      m_SimplexDeltaAmoeba2->show();
    }
    else if (m_FixedDimension == 3)
    {
      m_SimplexDeltaAmoebaLabel1->show();
      m_SimplexDeltaAmoeba1->show();
      m_SimplexDeltaAmoebaLabel2->show();
      m_SimplexDeltaAmoeba2->show();
      m_SimplexDeltaAmoebaLabel3->show();
      m_SimplexDeltaAmoeba3->show();
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::AFFINETRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_SimplexDeltaAmoebaLabel1->show();
      m_SimplexDeltaAmoeba1->show();
      m_SimplexDeltaAmoebaLabel2->show();
      m_SimplexDeltaAmoeba2->show();
      m_SimplexDeltaAmoebaLabel3->show();
      m_SimplexDeltaAmoeba3->show();
      m_SimplexDeltaAmoebaLabel4->show();
      m_SimplexDeltaAmoeba4->show();
      m_SimplexDeltaAmoebaLabel5->show();
      m_SimplexDeltaAmoeba5->show();
      m_SimplexDeltaAmoebaLabel6->show();
      m_SimplexDeltaAmoeba6->show();
    }
    else if (m_FixedDimension == 3)
    {
      m_SimplexDeltaAmoebaLabel1->show();
      m_SimplexDeltaAmoeba1->show();
      m_SimplexDeltaAmoebaLabel2->show();
      m_SimplexDeltaAmoeba2->show();
      m_SimplexDeltaAmoebaLabel3->show();
      m_SimplexDeltaAmoeba3->show();
      m_SimplexDeltaAmoebaLabel4->show();
      m_SimplexDeltaAmoeba4->show();
      m_SimplexDeltaAmoebaLabel5->show();
      m_SimplexDeltaAmoeba5->show();
      m_SimplexDeltaAmoebaLabel6->show();
      m_SimplexDeltaAmoeba6->show();
      m_SimplexDeltaAmoebaLabel7->show();
      m_SimplexDeltaAmoeba7->show();
      m_SimplexDeltaAmoebaLabel8->show();
      m_SimplexDeltaAmoeba8->show();
      m_SimplexDeltaAmoebaLabel9->show();
      m_SimplexDeltaAmoeba9->show();
      m_SimplexDeltaAmoebaLabel10->show();
      m_SimplexDeltaAmoeba10->show();
      m_SimplexDeltaAmoebaLabel11->show();
      m_SimplexDeltaAmoeba11->show();
      m_SimplexDeltaAmoebaLabel12->show();
      m_SimplexDeltaAmoeba12->show();
      m_SimplexDeltaAmoebaLabel13->show();
      m_SimplexDeltaAmoeba13->show();
      m_SimplexDeltaAmoebaLabel14->show();
      m_SimplexDeltaAmoeba14->show();
      m_SimplexDeltaAmoebaLabel15->show();
      m_SimplexDeltaAmoeba15->show();
      m_SimplexDeltaAmoebaLabel16->show();
      m_SimplexDeltaAmoeba16->show();
    }
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
    m_SimplexDeltaAmoebaLabel4->show();
    m_SimplexDeltaAmoeba4->show();
    m_SimplexDeltaAmoebaLabel5->show();
    m_SimplexDeltaAmoeba5->show();
    m_SimplexDeltaAmoebaLabel6->show();
    m_SimplexDeltaAmoeba6->show();
    m_SimplexDeltaAmoebaLabel7->show();
    m_SimplexDeltaAmoeba7->show();
    m_SimplexDeltaAmoebaLabel8->show();
    m_SimplexDeltaAmoeba8->show();
    m_SimplexDeltaAmoebaLabel9->show();
    m_SimplexDeltaAmoeba9->show();
    m_SimplexDeltaAmoebaLabel10->show();
    m_SimplexDeltaAmoeba10->show();
    m_SimplexDeltaAmoebaLabel11->show();
    m_SimplexDeltaAmoeba11->show();
    m_SimplexDeltaAmoebaLabel12->show();
    m_SimplexDeltaAmoeba12->show();
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER3DTRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM
    || m_TransformBox->currentItem() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
    m_SimplexDeltaAmoebaLabel4->show();
    m_SimplexDeltaAmoeba4->show();
    m_SimplexDeltaAmoebaLabel5->show();
    m_SimplexDeltaAmoeba5->show();
    m_SimplexDeltaAmoebaLabel6->show();
    m_SimplexDeltaAmoeba6->show();
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
    m_SimplexDeltaAmoebaLabel4->show();
    m_SimplexDeltaAmoeba4->show();
    m_SimplexDeltaAmoebaLabel5->show();
    m_SimplexDeltaAmoeba5->show();
    m_SimplexDeltaAmoebaLabel6->show();
    m_SimplexDeltaAmoeba6->show();
    m_SimplexDeltaAmoebaLabel7->show();
    m_SimplexDeltaAmoeba7->show();
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORTRANSFORM || m_TransformBox->currentItem() == mitk::TransformParameters::RIGID2DTRANSFORM
    || m_TransformBox->currentItem() == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
  }
  
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
    m_SimplexDeltaAmoebaLabel4->show();
    m_SimplexDeltaAmoeba4->show();
    m_SimplexDeltaAmoebaLabel5->show();
    m_SimplexDeltaAmoeba5->show();
    m_SimplexDeltaAmoebaLabel6->show();
    m_SimplexDeltaAmoeba6->show();
    m_SimplexDeltaAmoebaLabel7->show();
    m_SimplexDeltaAmoeba7->show();
    m_SimplexDeltaAmoebaLabel8->show();
    m_SimplexDeltaAmoeba8->show();
    m_SimplexDeltaAmoebaLabel9->show();
    m_SimplexDeltaAmoeba9->show();
    m_SimplexDeltaAmoebaLabel10->show();
    m_SimplexDeltaAmoeba10->show();
    m_SimplexDeltaAmoebaLabel11->show();
    m_SimplexDeltaAmoeba11->show();
    m_SimplexDeltaAmoebaLabel12->show();
    m_SimplexDeltaAmoeba12->show();
    m_SimplexDeltaAmoebaLabel13->show();
    m_SimplexDeltaAmoeba13->show();
    m_SimplexDeltaAmoebaLabel14->show();
    m_SimplexDeltaAmoeba14->show();
    m_SimplexDeltaAmoebaLabel15->show();
    m_SimplexDeltaAmoeba15->show();
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
    m_SimplexDeltaAmoebaLabel4->show();
    m_SimplexDeltaAmoeba4->show();
    m_SimplexDeltaAmoebaLabel5->show();
    m_SimplexDeltaAmoeba5->show();
  }
  else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    m_SimplexDeltaAmoebaLabel1->show();
    m_SimplexDeltaAmoeba1->show();
    m_SimplexDeltaAmoebaLabel2->show();
    m_SimplexDeltaAmoeba2->show();
    m_SimplexDeltaAmoebaLabel3->show();
    m_SimplexDeltaAmoeba3->show();
    m_SimplexDeltaAmoebaLabel4->show();
    m_SimplexDeltaAmoeba4->show();
  }
}

void QmitkRigidRegistrationSelector::LoadRigidRegistrationParameter()
{
  std::map<std::string, itk::Array<double> > existingPresets = m_Preset->getTransformValuesPresets();
  std::map<std::string, itk::Array<double> >::iterator iter;
  std::list<std::string> presets;
  for( iter = existingPresets.begin(); iter != existingPresets.end(); iter++ ) 
  {
    presets.push_back( (*iter).first );
  }
  presets.sort();
  // ask about the name to load a preset
  QmitkLoadPresetDialog dialog( this, "Load Preset", presets ); // needs a QWidget as parent
  int dialogReturnValue = dialog.exec();
  if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

  itk::Array<double> transformValues = m_Preset->getTransformValues(dialog.GetPresetName());
  m_TransformGroup->setChecked(true);
  m_TransformBox->setCurrentItem((int)transformValues[0]);
  m_TransformWidgetStack->raiseWidget((int)transformValues[0]);
  this->TransformSelected((int)transformValues[0]);
  if (transformValues[0] == mitk::TransformParameters::TRANSLATIONTRANSFORM)
  {
    m_UseOptimizerScalesTranslation->setChecked(transformValues[1]);
    m_ScalesTranslationTransformTranslationX->setText(QString::number(transformValues[2]));
    m_ScalesTranslationTransformTranslationY->setText(QString::number(transformValues[3]));
    m_ScalesTranslationTransformTranslationZ->setText(QString::number(transformValues[4]));
  }
  else if (transformValues[0] == mitk::TransformParameters::SCALETRANSFORM)
  {
    m_UseOptimizerScalesScale->setChecked(transformValues[1]);
    m_ScalesScaleTransformScaleX->setText(QString::number(transformValues[2]));
    m_ScalesScaleTransformScaleY->setText(QString::number(transformValues[3]));
    m_ScalesScaleTransformScaleZ->setText(QString::number(transformValues[4]));
  }
  else if (transformValues[0] == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    m_UseOptimizerScalesScaleLogarithmic->setChecked(transformValues[1]);
    m_ScalesScaleLogarithmicTransformScaleX->setText(QString::number(transformValues[2]));
    m_ScalesScaleLogarithmicTransformScaleY->setText(QString::number(transformValues[3]));
    m_ScalesScaleLogarithmicTransformScaleZ->setText(QString::number(transformValues[4]));
  }
  else if (transformValues[0] == mitk::TransformParameters::AFFINETRANSFORM)
  {
    m_UseOptimizerScalesAffine->setChecked(transformValues[1]);
    m_ScalesAffineTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesAffineTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesAffineTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesAffineTransformScale4->setText(QString::number(transformValues[5]));
    m_ScalesAffineTransformScale5->setText(QString::number(transformValues[6]));
    m_ScalesAffineTransformScale6->setText(QString::number(transformValues[7]));
    m_ScalesAffineTransformScale7->setText(QString::number(transformValues[8]));
    m_ScalesAffineTransformScale8->setText(QString::number(transformValues[9]));
    m_ScalesAffineTransformScale9->setText(QString::number(transformValues[10]));
    m_ScalesAffineTransformScale10->setText(QString::number(transformValues[11]));
    m_ScalesAffineTransformScale11->setText(QString::number(transformValues[12]));
    m_ScalesAffineTransformScale12->setText(QString::number(transformValues[13]));
    m_ScalesAffineTransformScale13->setText(QString::number(transformValues[14]));
    m_ScalesAffineTransformScaleTranslationX->setText(QString::number(transformValues[15]));
    m_ScalesAffineTransformScaleTranslationY->setText(QString::number(transformValues[16]));
    m_ScalesAffineTransformScaleTranslationZ->setText(QString::number(transformValues[17]));
    m_CenterForInitializerAffine->setChecked(transformValues[18]);
    m_MomentsAffine->setChecked(transformValues[19]);
    m_GeometryAffine->setChecked(!transformValues[19]);
  }
  else if (transformValues[0] == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    m_UseOptimizerScalesFixedCenterOfRotationAffine->setChecked(transformValues[1]);
    m_ScalesFixedCenterOfRotationAffineTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesFixedCenterOfRotationAffineTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesFixedCenterOfRotationAffineTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesFixedCenterOfRotationAffineTransformScale4->setText(QString::number(transformValues[5]));
    m_ScalesFixedCenterOfRotationAffineTransformScale5->setText(QString::number(transformValues[6]));
    m_ScalesFixedCenterOfRotationAffineTransformScale6->setText(QString::number(transformValues[7]));
    m_ScalesFixedCenterOfRotationAffineTransformScale7->setText(QString::number(transformValues[8]));
    m_ScalesFixedCenterOfRotationAffineTransformScale8->setText(QString::number(transformValues[9]));
    m_ScalesFixedCenterOfRotationAffineTransformScale9->setText(QString::number(transformValues[10]));
    m_ScalesFixedCenterOfRotationAffineTransformScale10->setText(QString::number(transformValues[11]));
    m_ScalesFixedCenterOfRotationAffineTransformScale11->setText(QString::number(transformValues[12]));
    m_ScalesFixedCenterOfRotationAffineTransformScale12->setText(QString::number(transformValues[13]));
    m_ScalesFixedCenterOfRotationAffineTransformScale13->setText(QString::number(transformValues[14]));
    m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->setText(QString::number(transformValues[15]));
    m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->setText(QString::number(transformValues[16]));
    m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->setText(QString::number(transformValues[17]));
    m_CenterForInitializerFixedCenterOfRotationAffine->setChecked(transformValues[18]);
    m_MomentsFixedCenterOfRotationAffine->setChecked(transformValues[19]);
    m_GeometryFixedCenterOfRotationAffine->setChecked(!transformValues[19]);
  }
  else if (transformValues[0] == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    m_UseOptimizerScalesRigid3D->setChecked(transformValues[1]);
    m_ScalesRigid3DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesRigid3DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesRigid3DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesRigid3DTransformScale4->setText(QString::number(transformValues[5]));
    m_ScalesRigid3DTransformScale5->setText(QString::number(transformValues[6]));
    m_ScalesRigid3DTransformScale6->setText(QString::number(transformValues[7]));
    m_ScalesRigid3DTransformScale7->setText(QString::number(transformValues[8]));
    m_ScalesRigid3DTransformScale8->setText(QString::number(transformValues[9]));
    m_ScalesRigid3DTransformScale9->setText(QString::number(transformValues[10]));
    m_ScalesRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[11]));
    m_ScalesRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[12]));
    m_ScalesRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[13]));
    m_CenterForInitializerRigid3D->setChecked(transformValues[14]);
    m_MomentsRigid3D->setChecked(transformValues[15]);
    m_GeometryRigid3D->setChecked(!transformValues[15]);
  }
  else if (transformValues[0] == mitk::TransformParameters::EULER3DTRANSFORM)
  {
    m_UseOptimizerScalesEuler3D->setChecked(transformValues[1]);
    m_ScalesEuler3DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesEuler3DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesEuler3DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesEuler3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_ScalesEuler3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_ScalesEuler3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_CenterForInitializerEuler3D->setChecked(transformValues[8]);
    m_MomentsEuler3D->setChecked(transformValues[9]);
    m_GeometryEuler3D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
  {
    m_UseOptimizerScalesCenteredEuler3D->setChecked(transformValues[1]);
    m_ScalesCenteredEuler3DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesCenteredEuler3DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesCenteredEuler3DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesCenteredEuler3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_ScalesCenteredEuler3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_ScalesCenteredEuler3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_CenterForInitializerCenteredEuler3D->setChecked(transformValues[8]);
    m_MomentsCenteredEuler3D->setChecked(transformValues[9]);
    m_GeometryCenteredEuler3D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
  {
    m_UseOptimizerScalesQuaternionRigid->setChecked(transformValues[1]);
    m_ScalesQuaternionRigidTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesQuaternionRigidTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesQuaternionRigidTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesQuaternionRigidTransformScale4->setText(QString::number(transformValues[5]));
    m_ScalesQuaternionRigidTransformScaleTranslationX->setText(QString::number(transformValues[6]));
    m_ScalesQuaternionRigidTransformScaleTranslationY->setText(QString::number(transformValues[7]));
    m_ScalesQuaternionRigidTransformScaleTranslationZ->setText(QString::number(transformValues[8]));
    m_CenterForInitializerQuaternionRigid->setChecked(transformValues[9]);
    m_MomentsQuaternionRigid->setChecked(transformValues[10]);
    m_GeometryQuaternionRigid->setChecked(!transformValues[10]);
  }
  else if (transformValues[0] == mitk::TransformParameters::VERSORTRANSFORM)
  {
    m_UseOptimizerScalesVersor->setChecked(transformValues[1]);
    m_ScalesVersorTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesVersorTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesVersorTransformScale3->setText(QString::number(transformValues[4]));
    m_CenterForInitializerVersor->setChecked(transformValues[5]);
    m_MomentsVersor->setChecked(transformValues[6]);
    m_GeometryVersor->setChecked(!transformValues[6]);
  }
  else if (transformValues[0] == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
  {
    m_UseOptimizerScalesVersorRigid3D->setChecked(transformValues[1]);
    m_ScalesVersorRigid3DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesVersorRigid3DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesVersorRigid3DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesVersorRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_ScalesVersorRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_ScalesVersorRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_CenterForInitializerVersorRigid3D->setChecked(transformValues[8]);
    m_MomentsVersorRigid3D->setChecked(transformValues[9]);
    m_GeometryVersorRigid3D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    m_UseOptimizerScalesFixedCenterOfRotationAffine->setChecked(transformValues[1]);
    m_ScalesScaleSkewVersorRigid3DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesScaleSkewVersorRigid3DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesScaleSkewVersorRigid3DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_ScalesScaleSkewVersorRigid3DTransformScale7->setText(QString::number(transformValues[8]));
    m_ScalesScaleSkewVersorRigid3DTransformScale8->setText(QString::number(transformValues[9]));
    m_ScalesScaleSkewVersorRigid3DTransformScale9->setText(QString::number(transformValues[10]));
    m_ScalesScaleSkewVersorRigid3DTransformScale10->setText(QString::number(transformValues[11]));
    m_ScalesScaleSkewVersorRigid3DTransformScale11->setText(QString::number(transformValues[12]));
    m_ScalesScaleSkewVersorRigid3DTransformScale12->setText(QString::number(transformValues[13]));
    m_ScalesScaleSkewVersorRigid3DTransformScale13->setText(QString::number(transformValues[14]));
    m_ScalesScaleSkewVersorRigid3DTransformScale14->setText(QString::number(transformValues[15]));
    m_ScalesScaleSkewVersorRigid3DTransformScale15->setText(QString::number(transformValues[16]));
    m_CenterForInitializerScaleSkewVersorRigid3D->setChecked(transformValues[17]);
    m_MomentsScaleSkewVersorRigid3D->setChecked(transformValues[18]);
    m_GeometryScaleSkewVersorRigid3D->setChecked(!transformValues[18]);
  }
  else if (transformValues[0] == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    m_UseOptimizerScalesSimilarity3D->setChecked(transformValues[1]);
    m_ScalesSimilarity3DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesSimilarity3DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesSimilarity3DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesSimilarity3DTransformScale4->setText(QString::number(transformValues[5]));
    m_ScalesSimilarity3DTransformScaleTranslationX->setText(QString::number(transformValues[6]));
    m_ScalesSimilarity3DTransformScaleTranslationY->setText(QString::number(transformValues[7]));
    m_ScalesSimilarity3DTransformScaleTranslationZ->setText(QString::number(transformValues[8]));
    m_CenterForInitializerSimilarity3D->setChecked(transformValues[9]);
    m_MomentsSimilarity3D->setChecked(transformValues[10]);
    m_GeometrySimilarity3D->setChecked(!transformValues[10]);
  }
  else if (transformValues[0] == mitk::TransformParameters::RIGID2DTRANSFORM)
  {
    m_UseOptimizerScalesRigid2D->setChecked(transformValues[1]);
    m_ScalesRigid2DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesRigid2DTransformScaleTranslationX->setText(QString::number(transformValues[3]));
    m_ScalesRigid2DTransformScaleTranslationY->setText(QString::number(transformValues[4]));
    m_CenterForInitializerRigid2D->setChecked(transformValues[5]);
    m_MomentsRigid2D->setChecked(transformValues[6]);
    m_GeometryRigid2D->setChecked(!transformValues[6]);
  }
  else if (transformValues[0] == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    m_UseOptimizerScalesCenteredRigid2D->setChecked(transformValues[1]);
    m_RotationScaleCenteredRigid2D->setText(QString::number(transformValues[2]));
    m_CenterXScaleCenteredRigid2D->setText(QString::number(transformValues[3]));
    m_CenterYScaleCenteredRigid2D->setText(QString::number(transformValues[4]));
    m_TranslationXScaleCenteredRigid2D->setText(QString::number(transformValues[5]));
    m_TranslationYScaleCenteredRigid2D->setText(QString::number(transformValues[6]));
    m_AngleCenteredRigid2D->setText(QString::number(transformValues[7]));
    m_CenterForInitializerCenteredRigid2D->setChecked(transformValues[8]);
    m_MomentsCenteredRigid2D->setChecked(transformValues[9]);
    m_GeometryCenteredRigid2D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    m_UseOptimizerScalesEuler2D->setChecked(transformValues[1]);
    m_RotationScaleEuler2D->setText(QString::number(transformValues[2]));
    m_TranslationXScaleEuler2D->setText(QString::number(transformValues[3]));
    m_TranslationYScaleEuler2D->setText(QString::number(transformValues[4]));
    m_CenterForInitializerEuler2D->setChecked(transformValues[5]);
    m_MomentsEuler2D->setChecked(transformValues[6]);
    m_GeometryEuler2D->setChecked(!transformValues[6]);
  }
  else if (transformValues[0] == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    m_UseOptimizerScalesSimilarity2D->setChecked(transformValues[1]);
    m_ScalingScaleSimilarity2D->setText(QString::number(transformValues[2]));
    m_RotationScaleSimilarity2D->setText(QString::number(transformValues[3]));
    m_TranslationXScaleSimilarity2D->setText(QString::number(transformValues[4]));
    m_TranslationYScaleSimilarity2D->setText(QString::number(transformValues[5]));
    m_InitialScaleSimilarity2D->setText(QString::number(transformValues[6]));
    m_AngleSimilarity2D->setText(QString::number(transformValues[7]));
    m_CenterForInitializerSimilarity2D->setChecked(transformValues[8]);
    m_MomentsSimilarity2D->setChecked(transformValues[9]);
    m_GeometrySimilarity2D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    m_UseOptimizerScalesCenteredSimilarity2D->setChecked(transformValues[1]);
    m_ScalesCenteredSimilarity2DTransformScale1->setText(QString::number(transformValues[2]));
    m_ScalesCenteredSimilarity2DTransformScale2->setText(QString::number(transformValues[3]));
    m_ScalesCenteredSimilarity2DTransformScale3->setText(QString::number(transformValues[4]));
    m_ScalesCenteredSimilarity2DTransformScale4->setText(QString::number(transformValues[5]));
    m_ScalesCenteredSimilarity2DTransformScaleTranslationX->setText(QString::number(transformValues[6]));
    m_ScalesCenteredSimilarity2DTransformScaleTranslationY->setText(QString::number(transformValues[7]));
    m_InitialScaleCenteredSimilarity2D->setText(QString::number(transformValues[8]));
    m_AngleCenteredSimilarity2D->setText(QString::number(transformValues[9]));
    m_CenterForInitializerCenteredSimilarity2D->setChecked(transformValues[10]);
    m_MomentsCenteredSimilarity2D->setChecked(transformValues[11]);
    m_GeometryCenteredSimilarity2D->setChecked(!transformValues[11]);
  }

  itk::Array<double> metricValues = m_Preset->getMetricValues(dialog.GetPresetName());
  m_MetricGroup->setChecked(true);
  m_MetricBox->setCurrentItem((int)metricValues[0]);
  m_MetricWidgetStack->raiseWidget((int)metricValues[0]);
  this->MetricSelected((int)metricValues[0]);
  m_ComputeGradient->setChecked(metricValues[1]);
  if (metricValues[0] == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_NumberOfHistogramBinsCorrelationCoefficientHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_NumberOfHistogramBinsMeanSquaresHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_NumberOfHistogramBinsMutualInformationHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_UseSamplingMattesMutualInformation->setChecked(metricValues[2]);
    m_NumberOfSpatialSamplesMattesMutualInformation->setText(QString::number(metricValues[3]));
    m_NumberOfHistogramBinsMattesMutualInformation->setText(QString::number(metricValues[4]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_LambdaMeanReciprocalSquareDifference->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_NumberOfSpatialSamplesMutualInformation->setText(QString::number(metricValues[2]));
    m_FixedImageStandardDeviationMutualInformation->setText(QString::number(metricValues[3]));
    m_MovingImageStandardDeviationMutualInformation->setText(QString::number(metricValues[4]));
    m_UseNormalizerAndSmoother->setChecked(metricValues[5]);
    m_FixedSmootherVarianceMutualInformation->setText(QString::number(metricValues[6]));
    m_MovingSmootherVarianceMutualInformation->setText(QString::number(metricValues[7]));
  }

  itk::Array<double> optimizerValues = m_Preset->getOptimizerValues(dialog.GetPresetName());
  m_OptimizerGroup->setChecked(true);
  m_OptimizerBox->setCurrentItem((int)optimizerValues[0]);
  m_OptimizerWidgetStack->raiseWidget((int)optimizerValues[0]);
  this->OptimizerSelected((int)optimizerValues[0]);
  m_Maximize->setChecked(optimizerValues[1]);
  if (optimizerValues[0] == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
  {
    m_StepLengthExhaustive->setText(QString::number(optimizerValues[2]));
    m_NumberOfStepsExhaustive->setText(QString::number(optimizerValues[3]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
  {
    m_LearningRateGradientDescent->setText(QString::number(optimizerValues[2]));
    m_IterationsGradientDescent->setText(QString::number(optimizerValues[3]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
  {
    m_LearningRateQuaternionRigidTransformGradientDescent->setText(QString::number(optimizerValues[2]));
    m_IterationsQuaternionRigidTransformGradientDescent->setText(QString::number(optimizerValues[3]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
  {
    m_ShrinkFactorOnePlusOneEvolutionary->setText(QString::number(optimizerValues[2]));
    m_GrowthFactorOnePlusOneEvolutionary->setText(QString::number(optimizerValues[3]));
    m_EpsilonOnePlusOneEvolutionary->setText(QString::number(optimizerValues[4]));
    m_InitialRadiusOnePlusOneEvolutionary->setText(QString::number(optimizerValues[5]));
    m_IterationsOnePlusOneEvolutionary->setText(QString::number(optimizerValues[6]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::POWELLOPTIMIZER)
  {
    m_StepLengthPowell->setText(QString::number(optimizerValues[2]));
    m_StepTolerancePowell->setText(QString::number(optimizerValues[3]));
    m_ValueTolerancePowell->setText(QString::number(optimizerValues[4]));
    m_IterationsPowell->setText(QString::number(optimizerValues[5]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::FRPROPTIMIZER)
  {
    m_FletchReevesFRPR->setChecked(optimizerValues[2]);
    m_PolakRibiereFRPR->setChecked(!optimizerValues[2]);
    m_StepLengthFRPR->setText(QString::number(optimizerValues[3]));
    m_IterationsFRPR->setText(QString::number(optimizerValues[4]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
  {
    m_GradientMagnitudeToleranceRegularStepGradientDescent->setText(QString::number(optimizerValues[2]));
    m_MinimumStepLengthRegularStepGradientDescent->setText(QString::number(optimizerValues[3]));
    m_MaximumStepLengthRegularStepGradientDescent->setText(QString::number(optimizerValues[4]));
    m_RelaxationFactorRegularStepGradientDescent->setText(QString::number(optimizerValues[5]));
    m_IterationsRegularStepGradientDescent->setText(QString::number(optimizerValues[6]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
  {
    m_GradientMagnitudeToleranceVersorTransform->setText(QString::number(optimizerValues[2]));
    m_MinimumStepLengthVersorTransform->setText(QString::number(optimizerValues[3]));
    m_MaximumStepLengthVersorTransform->setText(QString::number(optimizerValues[4]));
    m_IterationsVersorTransform->setText(QString::number(optimizerValues[5]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
  {
    m_SimplexDeltaAmoeba1->setText(QString::number(optimizerValues[2]));
    m_SimplexDeltaAmoeba2->setText(QString::number(optimizerValues[3]));
    m_SimplexDeltaAmoeba3->setText(QString::number(optimizerValues[4]));
    m_SimplexDeltaAmoeba4->setText(QString::number(optimizerValues[5]));
    m_SimplexDeltaAmoeba5->setText(QString::number(optimizerValues[6]));
    m_SimplexDeltaAmoeba6->setText(QString::number(optimizerValues[7]));
    m_SimplexDeltaAmoeba7->setText(QString::number(optimizerValues[8]));
    m_SimplexDeltaAmoeba8->setText(QString::number(optimizerValues[9]));
    m_SimplexDeltaAmoeba9->setText(QString::number(optimizerValues[10]));
    m_SimplexDeltaAmoeba10->setText(QString::number(optimizerValues[11]));
    m_SimplexDeltaAmoeba11->setText(QString::number(optimizerValues[12]));
    m_SimplexDeltaAmoeba12->setText(QString::number(optimizerValues[13]));
    m_SimplexDeltaAmoeba13->setText(QString::number(optimizerValues[14]));
    m_SimplexDeltaAmoeba14->setText(QString::number(optimizerValues[15]));
    m_SimplexDeltaAmoeba15->setText(QString::number(optimizerValues[16]));
    m_SimplexDeltaAmoeba16->setText(QString::number(optimizerValues[17]));
    m_ParametersConvergenceToleranceAmoeba->setText(QString::number(optimizerValues[18]));
    m_FunctionConvergenceToleranceAmoeba->setText(QString::number(optimizerValues[19]));
    m_IterationsAmoeba->setText(QString::number(optimizerValues[20]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::LBFGSOPTIMIZER)
  {
    m_GradientConvergenceToleranceLBFGS->setText(QString::number(optimizerValues[2]));
    m_LineSearchAccuracyLBFGS->setText(QString::number(optimizerValues[3]));
    m_DefaultStepLengthLBFGS->setText(QString::number(optimizerValues[4]));
    m_IterationsLBFGS->setText(QString::number(optimizerValues[5]));
    m_TraceOnLBFGS->setChecked(optimizerValues[6]);
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::SPSAOPTIMIZER)
  {
    m_aSPSA->setText(QString::number(optimizerValues[2]));
    m_ASPSA->setText(QString::number(optimizerValues[3]));
    m_AlphaSPSA->setText(QString::number(optimizerValues[4]));
    m_cSPSA->setText(QString::number(optimizerValues[5]));
    m_GammaSPSA->setText(QString::number(optimizerValues[6]));
    m_ToleranceSPSA->setText(QString::number(optimizerValues[7]));
    m_StateOfConvergenceDecayRateSPSA->setText(QString::number(optimizerValues[8]));
    m_MinimumNumberOfIterationsSPSA->setText(QString::number(optimizerValues[9]));
    m_NumberOfPerturbationsSPSA->setText(QString::number(optimizerValues[10]));
    m_IterationsSPSA->setText(QString::number(optimizerValues[11]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
  {
    m_GradientMagnitudeToleranceVersorRigid3DTransform->setText(QString::number(optimizerValues[2]));
    m_MinimumStepLengthVersorRigid3DTransform->setText(QString::number(optimizerValues[3]));
    m_MaximumStepLengthVersorRigid3DTransform->setText(QString::number(optimizerValues[4]));
    m_IterationsVersorRigid3DTransform->setText(QString::number(optimizerValues[5]));
  }

  itk::Array<double> interpolatorValues = m_Preset->getInterpolatorValues(dialog.GetPresetName());
  m_InterpolatorGroup->setChecked(true);
  m_InterpolatorBox->setCurrentItem((int)interpolatorValues[0]);
}

void QmitkRigidRegistrationSelector::SaveRigidRegistrationParameter()
{
  bool ok;
  QString text = QInputDialog::getText(
          "Save Parameter Preset", "Enter name for preset:", QLineEdit::Normal,
          QString::null, &ok, this );
  if ( ok )
  {
    std::map<std::string, itk::Array<double> > existingPresets = m_Preset->getTransformValuesPresets();
    std::map<std::string, itk::Array<double> >::iterator iter = existingPresets.find(std::string((const char*)text));
    if (iter != existingPresets.end())
    {
      QMessageBox::critical( this, "Preset definition",
      "Presetname already exists.");
      return;
    }
    if (text.isEmpty())
    {
      QMessageBox::critical( this, "Preset definition",
      "Presetname has to be set.\n"
      "You have to enter a Presetname." );
      return;
    }
    itk::Array<double> transformValues;
    transformValues.SetSize(25);
    transformValues.fill(0);
    transformValues[0] = m_TransformBox->currentItem();
    if (m_TransformBox->currentItem() == mitk::TransformParameters::TRANSLATIONTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesTranslation->isChecked();
      transformValues[2] = m_ScalesTranslationTransformTranslationX->text().toDouble();
      transformValues[3] = m_ScalesTranslationTransformTranslationY->text().toDouble();
      transformValues[4] = m_ScalesTranslationTransformTranslationZ->text().toDouble();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALETRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesScale->isChecked();
      transformValues[2] = m_ScalesScaleTransformScaleX->text().toDouble();
      transformValues[3] = m_ScalesScaleTransformScaleY->text().toDouble();
      transformValues[4] = m_ScalesScaleTransformScaleZ->text().toDouble();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesScaleLogarithmic->isChecked();
      transformValues[2] = m_ScalesScaleLogarithmicTransformScaleX->text().toDouble();
      transformValues[3] = m_ScalesScaleLogarithmicTransformScaleY->text().toDouble();
      transformValues[4] = m_ScalesScaleLogarithmicTransformScaleZ->text().toDouble();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::AFFINETRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesAffine->isChecked();
      transformValues[2] = m_ScalesAffineTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesAffineTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesAffineTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesAffineTransformScale4->text().toDouble();
      transformValues[6] = m_ScalesAffineTransformScale5->text().toDouble();
      transformValues[7] = m_ScalesAffineTransformScale6->text().toDouble();
      transformValues[8] = m_ScalesAffineTransformScale7->text().toDouble();
      transformValues[9] = m_ScalesAffineTransformScale8->text().toDouble();
      transformValues[10] = m_ScalesAffineTransformScale9->text().toDouble();
      transformValues[11] = m_ScalesAffineTransformScale10->text().toDouble();
      transformValues[12] = m_ScalesAffineTransformScale11->text().toDouble();
      transformValues[13] = m_ScalesAffineTransformScale12->text().toDouble();
      transformValues[14] = m_ScalesAffineTransformScale13->text().toDouble();
      transformValues[15] = m_ScalesAffineTransformScaleTranslationX->text().toDouble();
      transformValues[16] = m_ScalesAffineTransformScaleTranslationY->text().toDouble();
      transformValues[17] = m_ScalesAffineTransformScaleTranslationZ->text().toDouble();
      transformValues[18] = m_CenterForInitializerAffine->isChecked();
      transformValues[19] = m_MomentsAffine->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked();
      transformValues[2] = m_ScalesFixedCenterOfRotationAffineTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesFixedCenterOfRotationAffineTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesFixedCenterOfRotationAffineTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesFixedCenterOfRotationAffineTransformScale4->text().toDouble();
      transformValues[6] = m_ScalesFixedCenterOfRotationAffineTransformScale5->text().toDouble();
      transformValues[7] = m_ScalesFixedCenterOfRotationAffineTransformScale6->text().toDouble();
      transformValues[8] = m_ScalesFixedCenterOfRotationAffineTransformScale7->text().toDouble();
      transformValues[9] = m_ScalesFixedCenterOfRotationAffineTransformScale8->text().toDouble();
      transformValues[10] = m_ScalesFixedCenterOfRotationAffineTransformScale9->text().toDouble();
      transformValues[11] = m_ScalesFixedCenterOfRotationAffineTransformScale10->text().toDouble();
      transformValues[12] = m_ScalesFixedCenterOfRotationAffineTransformScale11->text().toDouble();
      transformValues[13] = m_ScalesFixedCenterOfRotationAffineTransformScale12->text().toDouble();
      transformValues[14] = m_ScalesFixedCenterOfRotationAffineTransformScale13->text().toDouble();
      transformValues[15] = m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->text().toDouble();
      transformValues[16] = m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->text().toDouble();
      transformValues[17] = m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->text().toDouble();
      transformValues[18] = m_CenterForInitializerFixedCenterOfRotationAffine->isChecked();
      transformValues[19] = m_MomentsFixedCenterOfRotationAffine->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID3DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesRigid3D->isChecked();
      transformValues[2] = m_ScalesRigid3DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesRigid3DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesRigid3DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesRigid3DTransformScale4->text().toDouble();
      transformValues[6] = m_ScalesRigid3DTransformScale5->text().toDouble();
      transformValues[7] = m_ScalesRigid3DTransformScale6->text().toDouble();
      transformValues[8] = m_ScalesRigid3DTransformScale7->text().toDouble();
      transformValues[9] = m_ScalesRigid3DTransformScale8->text().toDouble();
      transformValues[10] = m_ScalesRigid3DTransformScale9->text().toDouble();
      transformValues[11] = m_ScalesRigid3DTransformScaleTranslationX->text().toDouble();
      transformValues[12] = m_ScalesRigid3DTransformScaleTranslationY->text().toDouble();
      transformValues[13] = m_ScalesRigid3DTransformScaleTranslationZ->text().toDouble();
      transformValues[14] = m_CenterForInitializerRigid3D->isChecked();
      transformValues[15] = m_MomentsRigid3D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER3DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesEuler3D->isChecked();
      transformValues[2] = m_ScalesEuler3DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesEuler3DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesEuler3DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesEuler3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_ScalesEuler3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_ScalesEuler3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_CenterForInitializerEuler3D->isChecked();
      transformValues[9] = m_MomentsEuler3D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesCenteredEuler3D->isChecked();
      transformValues[2] = m_ScalesCenteredEuler3DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesCenteredEuler3DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesCenteredEuler3DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesCenteredEuler3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_ScalesCenteredEuler3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_ScalesCenteredEuler3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_CenterForInitializerCenteredEuler3D->isChecked();
      transformValues[9] = m_MomentsCenteredEuler3D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesQuaternionRigid->isChecked();
      transformValues[2] = m_ScalesQuaternionRigidTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesQuaternionRigidTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesQuaternionRigidTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesQuaternionRigidTransformScale4->text().toDouble();
      transformValues[6] = m_ScalesQuaternionRigidTransformScaleTranslationX->text().toDouble();
      transformValues[7] = m_ScalesQuaternionRigidTransformScaleTranslationY->text().toDouble();
      transformValues[8] = m_ScalesQuaternionRigidTransformScaleTranslationZ->text().toDouble();
      transformValues[9] = m_CenterForInitializerQuaternionRigid->isChecked();
      transformValues[10] = m_MomentsQuaternionRigid->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesVersor->isChecked();
      transformValues[2] = m_ScalesVersorTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesVersorTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesVersorTransformScale3->text().toDouble();
      transformValues[5] = m_CenterForInitializerVersor->isChecked();
      transformValues[6] = m_MomentsVersor->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesVersorRigid3D->isChecked();
      transformValues[2] = m_ScalesVersorRigid3DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesVersorRigid3DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesVersorRigid3DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesVersorRigid3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_ScalesVersorRigid3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_ScalesVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_CenterForInitializerVersorRigid3D->isChecked();
      transformValues[9] = m_MomentsVersorRigid3D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked();
      transformValues[2] = m_ScalesScaleSkewVersorRigid3DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesScaleSkewVersorRigid3DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesScaleSkewVersorRigid3DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_ScalesScaleSkewVersorRigid3DTransformScale7->text().toDouble();
      transformValues[9] = m_ScalesScaleSkewVersorRigid3DTransformScale8->text().toDouble();
      transformValues[10] = m_ScalesScaleSkewVersorRigid3DTransformScale9->text().toDouble();
      transformValues[11] = m_ScalesScaleSkewVersorRigid3DTransformScale10->text().toDouble();
      transformValues[12] = m_ScalesScaleSkewVersorRigid3DTransformScale11->text().toDouble();
      transformValues[13] = m_ScalesScaleSkewVersorRigid3DTransformScale12->text().toDouble();
      transformValues[14] = m_ScalesScaleSkewVersorRigid3DTransformScale13->text().toDouble();
      transformValues[15] = m_ScalesScaleSkewVersorRigid3DTransformScale14->text().toDouble();
      transformValues[16] = m_ScalesScaleSkewVersorRigid3DTransformScale15->text().toDouble();
      transformValues[17] = m_CenterForInitializerScaleSkewVersorRigid3D->isChecked();
      transformValues[18] = m_MomentsScaleSkewVersorRigid3D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesSimilarity3D->isChecked();
      transformValues[2] = m_ScalesSimilarity3DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesSimilarity3DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesSimilarity3DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesSimilarity3DTransformScale4->text().toDouble();
      transformValues[6] = m_ScalesSimilarity3DTransformScaleTranslationX->text().toDouble();
      transformValues[7] = m_ScalesSimilarity3DTransformScaleTranslationY->text().toDouble();
      transformValues[8] = m_ScalesSimilarity3DTransformScaleTranslationZ->text().toDouble();
      transformValues[9] = m_CenterForInitializerSimilarity3D->isChecked();
      transformValues[10] = m_MomentsSimilarity3D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::RIGID2DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesRigid2D->isChecked();
      transformValues[2] = m_ScalesRigid2DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesRigid2DTransformScaleTranslationX->text().toDouble();
      transformValues[4] = m_ScalesRigid2DTransformScaleTranslationY->text().toDouble();
      transformValues[5] = m_CenterForInitializerRigid2D->isChecked();
      transformValues[6] = m_MomentsRigid2D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesCenteredRigid2D->isChecked();
      transformValues[2] = m_RotationScaleCenteredRigid2D->text().toDouble();
      transformValues[3] = m_CenterXScaleCenteredRigid2D->text().toDouble();
      transformValues[4] = m_CenterYScaleCenteredRigid2D->text().toDouble();
      transformValues[5] = m_TranslationXScaleCenteredRigid2D->text().toDouble();
      transformValues[6] = m_TranslationYScaleCenteredRigid2D->text().toDouble();
      transformValues[7] = m_AngleCenteredRigid2D->text().toFloat();
      transformValues[8] = m_CenterForInitializerCenteredRigid2D->isChecked();
      transformValues[9] = m_MomentsCenteredRigid2D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::EULER2DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesEuler2D->isChecked();
      transformValues[2] = m_RotationScaleEuler2D->text().toDouble();
      transformValues[3] = m_TranslationXScaleEuler2D->text().toDouble();
      transformValues[4] = m_TranslationYScaleEuler2D->text().toDouble();
      transformValues[5] = m_CenterForInitializerEuler2D->isChecked();
      transformValues[6] = m_MomentsEuler2D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesSimilarity2D->isChecked();
      transformValues[2] = m_ScalingScaleSimilarity2D->text().toDouble();
      transformValues[3] = m_RotationScaleSimilarity2D->text().toDouble();
      transformValues[4] = m_TranslationXScaleSimilarity2D->text().toDouble();
      transformValues[5] = m_TranslationYScaleSimilarity2D->text().toDouble();
      transformValues[6] = m_InitialScaleSimilarity2D->text().toFloat();
      transformValues[7] = m_AngleSimilarity2D->text().toFloat();
      transformValues[8] = m_CenterForInitializerSimilarity2D->isChecked();
      transformValues[9] = m_MomentsSimilarity2D->isChecked();
    }
    else if (m_TransformBox->currentItem() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      transformValues[1] = m_UseOptimizerScalesCenteredSimilarity2D->isChecked();
      transformValues[2] = m_ScalesCenteredSimilarity2DTransformScale1->text().toDouble();
      transformValues[3] = m_ScalesCenteredSimilarity2DTransformScale2->text().toDouble();
      transformValues[4] = m_ScalesCenteredSimilarity2DTransformScale3->text().toDouble();
      transformValues[5] = m_ScalesCenteredSimilarity2DTransformScale4->text().toDouble();
      transformValues[6] = m_ScalesCenteredSimilarity2DTransformScaleTranslationX->text().toDouble();
      transformValues[7] = m_ScalesCenteredSimilarity2DTransformScaleTranslationY->text().toDouble();
      transformValues[8] = m_InitialScaleCenteredSimilarity2D->text().toFloat();
      transformValues[9] = m_AngleCenteredSimilarity2D->text().toFloat();
      transformValues[10] = m_CenterForInitializerCenteredSimilarity2D->isChecked();
      transformValues[11] = m_MomentsCenteredSimilarity2D->isChecked();
    }
    std::map<std::string, itk::Array<double> > transformMap = m_Preset->getTransformValuesPresets();
    transformMap[std::string((const char*)text)] = transformValues;

    itk::Array<double> metricValues;
    metricValues.SetSize(25);
    metricValues.fill(0);
    metricValues[0] = m_MetricBox->currentItem();
    metricValues[1] = m_ComputeGradient->isOn();
    if (m_MetricBox->currentItem() == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_NumberOfHistogramBinsCorrelationCoefficientHistogram->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_NumberOfHistogramBinsMeanSquaresHistogram->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_NumberOfHistogramBinsMutualInformationHistogram->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_UseSamplingMattesMutualInformation->isOn();
      metricValues[3] = m_NumberOfSpatialSamplesMattesMutualInformation->text().toInt();
      metricValues[4] = m_NumberOfHistogramBinsMattesMutualInformation->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_LambdaMeanReciprocalSquareDifference->text().toInt();
    }
    else if (m_MetricBox->currentItem() == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_NumberOfSpatialSamplesMutualInformation->text().toInt();
      metricValues[3] = m_FixedImageStandardDeviationMutualInformation->text().toFloat();
      metricValues[4] = m_MovingImageStandardDeviationMutualInformation->text().toFloat();
      metricValues[5] = m_UseNormalizerAndSmoother->isChecked();
      metricValues[6] = m_FixedSmootherVarianceMutualInformation->text().toFloat();
      metricValues[7] = m_MovingSmootherVarianceMutualInformation->text().toFloat();
    }

    std::map<std::string, itk::Array<double> > metricMap = m_Preset->getMetricValuesPresets();
    metricMap[std::string((const char*)text)] = metricValues;

    itk::Array<double> optimizerValues;
    optimizerValues.SetSize(25);
    optimizerValues.fill(0);
    optimizerValues[0] = m_OptimizerBox->currentItem();
    optimizerValues[1] = m_Maximize->isOn();
    if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
    {
      optimizerValues[2] = m_StepLengthExhaustive->text().toFloat();
      optimizerValues[3] = m_NumberOfStepsExhaustive->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
    {
      optimizerValues[2] = m_LearningRateGradientDescent->text().toFloat();
      optimizerValues[3] = m_IterationsGradientDescent->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
    {
      optimizerValues[2] = m_LearningRateQuaternionRigidTransformGradientDescent->text().toFloat();
      optimizerValues[3] = m_IterationsQuaternionRigidTransformGradientDescent->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
    {
      optimizerValues[2] = m_ShrinkFactorOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[3] = m_GrowthFactorOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[4] = m_EpsilonOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[5] = m_InitialRadiusOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[6] = m_IterationsOnePlusOneEvolutionary->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::POWELLOPTIMIZER)
    {
      optimizerValues[2] = m_StepLengthPowell->text().toFloat();
      optimizerValues[3] = m_StepTolerancePowell->text().toFloat();
      optimizerValues[4] = m_ValueTolerancePowell->text().toFloat();
      optimizerValues[5] = m_IterationsPowell->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::FRPROPTIMIZER)
    {
      optimizerValues[2] = m_FletchReevesFRPR->isOn();
      optimizerValues[3] = m_StepLengthFRPR->text().toFloat();
      optimizerValues[4] = m_IterationsFRPR->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
    {
      optimizerValues[2] = m_GradientMagnitudeToleranceRegularStepGradientDescent->text().toFloat();
      optimizerValues[3] = m_MinimumStepLengthRegularStepGradientDescent->text().toFloat();
      optimizerValues[4] = m_MaximumStepLengthRegularStepGradientDescent->text().toFloat();
      optimizerValues[5] = m_RelaxationFactorRegularStepGradientDescent->text().toFloat();
      optimizerValues[6] = m_IterationsRegularStepGradientDescent->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
    {
      optimizerValues[2] = m_GradientMagnitudeToleranceVersorTransform->text().toFloat();
      optimizerValues[3] = m_MinimumStepLengthVersorTransform->text().toFloat();
      optimizerValues[4] = m_MaximumStepLengthVersorTransform->text().toFloat();
      optimizerValues[5] = m_IterationsVersorTransform->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
    {
      optimizerValues[2] = m_SimplexDeltaAmoeba1->text().toDouble();
      optimizerValues[3] = m_SimplexDeltaAmoeba2->text().toDouble();
      optimizerValues[4] = m_SimplexDeltaAmoeba3->text().toDouble();
      optimizerValues[5] = m_SimplexDeltaAmoeba4->text().toDouble();
      optimizerValues[6] = m_SimplexDeltaAmoeba5->text().toDouble();
      optimizerValues[7] = m_SimplexDeltaAmoeba6->text().toDouble();
      optimizerValues[8] = m_SimplexDeltaAmoeba7->text().toDouble();
      optimizerValues[9] = m_SimplexDeltaAmoeba8->text().toDouble();
      optimizerValues[10] = m_SimplexDeltaAmoeba9->text().toDouble();
      optimizerValues[11] = m_SimplexDeltaAmoeba10->text().toDouble();
      optimizerValues[12] = m_SimplexDeltaAmoeba11->text().toDouble();
      optimizerValues[13] = m_SimplexDeltaAmoeba12->text().toDouble();
      optimizerValues[14] = m_SimplexDeltaAmoeba13->text().toDouble();
      optimizerValues[15] = m_SimplexDeltaAmoeba14->text().toDouble();
      optimizerValues[16] = m_SimplexDeltaAmoeba15->text().toDouble();
      optimizerValues[17] = m_SimplexDeltaAmoeba16->text().toDouble();
      optimizerValues[18] = m_ParametersConvergenceToleranceAmoeba->text().toFloat();
      optimizerValues[19] = m_FunctionConvergenceToleranceAmoeba->text().toFloat();
      optimizerValues[20] = m_IterationsAmoeba->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::LBFGSOPTIMIZER)
    {
      optimizerValues[2] = m_GradientConvergenceToleranceLBFGS->text().toFloat();
      optimizerValues[3] = m_LineSearchAccuracyLBFGS->text().toFloat();
      optimizerValues[4] = m_DefaultStepLengthLBFGS->text().toFloat();
      optimizerValues[5] = m_IterationsLBFGS->text().toInt();
      optimizerValues[6] = m_TraceOnLBFGS->isOn();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::SPSAOPTIMIZER)
    {
      optimizerValues[2] = m_aSPSA->text().toFloat();
      optimizerValues[3] = m_ASPSA->text().toFloat();
      optimizerValues[4] = m_AlphaSPSA->text().toFloat();
      optimizerValues[5] = m_cSPSA->text().toFloat();
      optimizerValues[6] = m_GammaSPSA->text().toFloat();
      optimizerValues[7] = m_ToleranceSPSA->text().toFloat();
      optimizerValues[8] = m_StateOfConvergenceDecayRateSPSA->text().toFloat();
      optimizerValues[9] = m_MinimumNumberOfIterationsSPSA->text().toInt();
      optimizerValues[10] = m_NumberOfPerturbationsSPSA->text().toInt();
      optimizerValues[11] = m_IterationsSPSA->text().toInt();
    }
    else if (m_OptimizerBox->currentItem() == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
    {
      optimizerValues[2] = m_GradientMagnitudeToleranceVersorRigid3DTransform->text().toFloat();
      optimizerValues[3] = m_MinimumStepLengthVersorRigid3DTransform->text().toFloat();
      optimizerValues[4] = m_MaximumStepLengthVersorRigid3DTransform->text().toFloat();
      optimizerValues[5] = m_IterationsVersorRigid3DTransform->text().toInt();
    }

    std::map<std::string, itk::Array<double> > optimizerMap = m_Preset->getOptimizerValuesPresets();
    optimizerMap[std::string((const char*)text)] = optimizerValues;

    itk::Array<double> interpolatorValues;
    interpolatorValues.SetSize(25);
    interpolatorValues.fill(0);
    interpolatorValues[0] = m_InterpolatorBox->currentItem();

    std::map<std::string, itk::Array<double> > interpolatorMap = m_Preset->getInterpolatorValuesPresets();
    interpolatorMap[std::string((const char*)text)] = interpolatorValues;

    m_Preset->newPresets(transformMap, metricMap, optimizerMap, interpolatorMap);
  }
  else 
  {
      // user pressed Cancel
  }
}
