/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkMetricParameters.h"
#include "mitkOptimizerParameters.h"
#include "mitkTransformParameters.h"
#include "mitkImageTimeSelector.h"
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
#include <itkArray.h>
#include "mitkRigidRegistrationPreset.h"
#include "mitkRigidRegistrationTestPreset.h"
#include "mitkProgressBar.h"

#include "QmitkRigidRegistrationSelectorView.h"
#include "mitkPyramidalRegistrationMethod.h"

QmitkRigidRegistrationSelectorView::QmitkRigidRegistrationSelectorView(QWidget* parent, Qt::WindowFlags f ) : QWidget( parent, f ),
m_FixedNode(NULL), m_FixedMaskNode(NULL), m_MovingNode(NULL), m_MovingMaskNode(NULL), m_OptimizerParameters(NULL), m_TransformParameters(NULL), m_MetricParameters(NULL),
m_FixedDimension(0), m_MovingDimension(0), m_StopOptimization(false), m_GeometryItkPhysicalToWorldTransform(NULL),
m_GeometryWorldToItkPhysicalTransform(NULL), m_MovingGeometry(NULL), m_ImageGeometry(NULL)
{
  m_Controls.setupUi(parent);
  m_Controls.m_PyramidGroup->hide();
  m_Controls.m_PreprocessingGroup->hide();


  //// create connections
  connect( m_Controls.m_TransformGroup, SIGNAL(clicked(bool)), m_Controls.m_TransformFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_TransformBox, SIGNAL(activated(int)), m_Controls.m_TransformWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_TransformBox, SIGNAL(activated(int)), this, SLOT(TransformSelected(int)));
  connect( m_Controls.m_MetricGroup, SIGNAL(clicked(bool)), m_Controls.m_MetricFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_MetricBox, SIGNAL(activated(int)), m_Controls.m_MetricWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_MetricBox, SIGNAL(activated(int)), this, SLOT(MetricSelected(int)));
  connect( m_Controls.m_OptimizerGroup, SIGNAL(clicked(bool)), m_Controls.m_OptimizerFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_OptimizerBox, SIGNAL(activated(int)), m_Controls.m_OptimizerWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_OptimizerBox, SIGNAL(activated(int)), this, SLOT(OptimizerSelected(int)));
  connect( m_Controls.m_InterpolatorGroup, SIGNAL(toggled(bool)), m_Controls.m_InterpolatorFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_PyramidGroup, SIGNAL(clicked(bool)), m_Controls.m_PyramidFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_PreprocessingGroup, SIGNAL(clicked(bool)), m_Controls.m_PreprocessingFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_UseSamplingMattesMutualInformation, SIGNAL(clicked(bool)), m_Controls.textLabel1, SLOT(setEnabled(bool)));
  connect( m_Controls.m_UseSamplingMattesMutualInformation, SIGNAL(clicked(bool)), m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation, SLOT(setEnabled(bool)));


  m_Preset = new mitk::RigidRegistrationPreset();
  m_Preset->LoadPreset();

  m_TestPreset = new mitk::RigidRegistrationTestPreset();
  m_TestPreset->LoadPreset();

  /// for optimizer
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfStepsExhaustive->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsGradientDescent->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->setValidator(validatorLineEditInput);
  m_Controls.m_EpsilonOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_Controls.m_InitialRadiusOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsPowell->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsFRPR->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsRegularStepGradientDescent->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsVersorTransform->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsAmoeba->setValidator(validatorLineEditInput);
  m_Controls.m_MaximumEvaluationsLBFGS->setValidator(validatorLineEditInput);
  m_Controls.m_MinimumNumberOfIterationsSPSA->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfPerturbationsSPSA->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsSPSA->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsCorrelationCoefficientHistogram->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->setValidator(validatorLineEditInput);
  m_Controls.m_LambdaMeanReciprocalSquareDifference->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfSpatialSamplesMutualInformation->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsLBFGSB->setValidator(validatorLineEditInput);


  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_StepLengthExhaustive->setValidator(validatorLineEditInputFloat);
  m_Controls.m_LearningRateGradientDescent->setValidator(validatorLineEditInputFloat);
  m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_GrowthFactorOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_EpsilonOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_InitialRadiusOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_IterationsOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_StepLengthPowell->setValidator(validatorLineEditInputFloat);
  m_Controls.m_StepTolerancePowell->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ValueTolerancePowell->setValidator(validatorLineEditInputFloat);
  m_Controls.m_StepLengthFRPR->setValidator(validatorLineEditInputFloat);
  m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MinimumStepLengthRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MaximumStepLengthRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  QValidator* validatorLineEditInputFloat0to1 = new QDoubleValidator(0.000001, 0.999999, 8, this);
  m_Controls.m_RelaxationFactorRegularStepGradientDescent->setValidator(validatorLineEditInputFloat0to1);
  m_Controls.m_GradientMagnitudeToleranceVersorTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MinimumStepLengthVersorTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MaximumStepLengthVersorTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba10->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba11->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba12->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba13->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba14->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba15->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ParametersConvergenceToleranceAmoeba->setValidator(validatorLineEditInputFloat);
  m_Controls.m_FunctionConvergenceToleranceAmoeba->setValidator(validatorLineEditInputFloat);
  m_Controls.m_GradientMagnitudeToleranceLBFGS->setValidator(validatorLineEditInputFloat);
  m_Controls.m_LineSearchAccuracyLBFGS->setValidator(validatorLineEditInputFloat);
  m_Controls.m_DefaultStepLengthLBFGS->setValidator(validatorLineEditInputFloat);
  m_Controls.m_aSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ASPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_AlphaSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_cSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_GammaSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ToleranceSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_StateOfConvergenceDecayRateSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_FixedImageStandardDeviationMutualInformation->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MovingImageStandardDeviationMutualInformation->setValidator(validatorLineEditInputFloat);
  m_Controls.m_FixedSmootherVarianceMutualInformation->setValidator(validatorLineEditInput);
  m_Controls.m_MovingSmootherVarianceMutualInformation->setValidator(validatorLineEditInput);
  m_Controls.m_GradientConvergenceToleranceLBFGSB->setValidator(validatorLineEditInputFloat);
  m_Controls.m_DefaultStepLengthLBFGSB->setValidator(validatorLineEditInputFloat);
  m_Controls.m_LineSearchAccuracyLBFGSB->setValidator(validatorLineEditInputFloat);


  // for Transformations
  m_Controls.m_ScalesTranslationTransformTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesTranslationTransformTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesTranslationTransformTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleTransformScaleX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleTransformScaleY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleTransformScaleZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid2DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid2DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid2DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_AngleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_RotationScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_CenterXScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_CenterYScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationXScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationYScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_RotationScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationXScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationYScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalingScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_RotationScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationXScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationYScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);

  m_Observer = mitk::RigidRegistrationObserver::New();
  m_Controls.m_TransformFrame->setEnabled(true);
  m_Controls.m_MetricFrame->setEnabled(true);
  m_Controls.m_OptimizerFrame->setEnabled(true);
  m_Controls.m_InterpolatorFrame->setEnabled(true);

  m_Controls.m_TransformFrame->hide();
  m_Controls.m_MetricFrame->hide();
  m_Controls.m_OptimizerFrame->hide();
  m_Controls.m_InterpolatorFrame->hide();
  m_Controls.m_PyramidFrame->hide();
  m_Controls.m_PreprocessingFrame->hide();
  m_Controls.m_TransformWidgetStack->setCurrentIndex(0);
  m_Controls.m_MetricWidgetStack->setCurrentIndex(0);
  m_Controls.m_OptimizerWidgetStack->setCurrentIndex(0);


  /// and show the selected views
  m_Controls.m_TransformBox->setCurrentIndex(0);
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
  m_Controls.m_MetricBox->setCurrentIndex(0);
  this->MetricSelected(m_Controls.m_MetricBox->currentIndex());
  m_Controls.m_OptimizerBox->setCurrentIndex(0);
  this->OptimizerSelected(m_Controls.m_OptimizerBox->currentIndex());
  
  this->DoLoadRigidRegistrationPreset("AffineMutualInformationGradientDescent", false);
}

QmitkRigidRegistrationSelectorView::~QmitkRigidRegistrationSelectorView()
{
}

/// this method starts the registration process
void QmitkRigidRegistrationSelectorView::CalculateTransformation(unsigned int timestep)
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    emit AddNewTransformationToUndoList();

    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    mitk::Image::Pointer mmimage = NULL;
    mitk::Image::Pointer fmimage = NULL;
    if (m_MovingMaskNode.IsNotNull() && m_FixedMaskNode.IsNotNull())
    {
      mmimage = dynamic_cast<mitk::Image*>(m_MovingMaskNode->GetData());
      fmimage = dynamic_cast<mitk::Image*>(m_FixedMaskNode->GetData());
    }

    mitk::ImageTimeSelector::Pointer its = mitk::ImageTimeSelector::New();

    if(fimage->GetDimension()>3)
    {
      its->SetInput(fimage);
      its->SetTimeNr(timestep);
      its->Update();
      fimage = its->GetOutput();
    }

    if(mimage->GetDimension()>3)
    {
      its->SetInput(mimage);
      its->SetTimeNr(timestep);
      its->Update();
      mimage = its->GetOutput();
    }

    // Initial moving image geometry
    m_ImageGeometry = m_MovingNode->GetData()->GetGeometry()->Clone();
    std::cout << "Moving Image Geometry (IndexToWorldTransform)"  << std::endl;
    std::cout << m_ImageGeometry->GetIndexToWorldTransform()->GetMatrix();
    mitk::Geometry3D::TransformType::InputPointType center = m_ImageGeometry->GetIndexToWorldTransform()->GetCenter();
    std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    mitk::Geometry3D::TransformType::OutputVectorType offset = m_ImageGeometry->GetIndexToWorldTransform()->GetOffset();
    std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    std::cout << std::endl;

    // Fixed image geometry
    //     mitk::AffineGeometryFrame3D::Pointer m_FixedGeometryCopy = m_FixedNode->GetData()->GetGeometry()->Clone();
    //     std::cout << "Fixed Image Geometry (IndexToWorldTransform)"  << std::endl;
    //     std::cout << m_FixedGeometryCopy->GetIndexToWorldTransform()->GetMatrix();
    //     center = m_FixedGeometryCopy->GetIndexToWorldTransform()->GetCenter();
    //     std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    //     offset = m_FixedGeometryCopy->GetIndexToWorldTransform()->GetOffset();
    //     std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    //     std::cout << std::endl;

    // Calculate the World to ITK-Physical transform for the moving image
    m_MovingGeometry = m_MovingNode->GetData()->GetGeometry();

    unsigned long size;
    size = m_MovingNodeChildren->Size();
    mitk::DataTreeNode::Pointer childNode;
    for (unsigned long i = 0; i < size; ++i)
    {
      m_ChildNodes.insert(std::pair<mitk::DataTreeNode::Pointer, mitk::Geometry3D*>(m_MovingNodeChildren->GetElement(i), m_MovingNodeChildren->GetElement(i)->GetData()->GetGeometry()));
      m_ChildNodes2.insert(std::pair<mitk::DataTreeNode::Pointer, mitk::AffineGeometryFrame3D::Pointer>(m_MovingNodeChildren->GetElement(i), m_MovingNodeChildren->GetElement(i)->GetData()->GetGeometry()->Clone()));
    }

    m_GeometryWorldToItkPhysicalTransform = mitk::Geometry3D::TransformType::New();
    GetWorldToItkPhysicalTransform(m_MovingGeometry, m_GeometryWorldToItkPhysicalTransform.GetPointer());

    //     std::cout << "Moving Image: World to ITK-physical transform" << std::endl;
    //     std::cout << m_GeometryWorldToItkPhysicalTransform->GetMatrix();
    //     center = m_GeometryWorldToItkPhysicalTransform->GetCenter();
    //     std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    //     offset = m_GeometryWorldToItkPhysicalTransform->GetOffset();
    //     std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    //     std::cout << std::endl;

    // Calculate the ITK-Physical to World transform for the fixed image
    m_GeometryItkPhysicalToWorldTransform = mitk::Geometry3D::TransformType::New();
    mitk::Geometry3D::TransformType::Pointer fixedWorld2Phys = mitk::Geometry3D::TransformType::New();
    GetWorldToItkPhysicalTransform(m_FixedNode->GetData()->GetGeometry(), fixedWorld2Phys.GetPointer());
    fixedWorld2Phys->GetInverse(m_GeometryItkPhysicalToWorldTransform);

    //     std::cout << "Fixed Image: ITK-physical to World transform" << std::endl;
    //     std::cout << m_GeometryItkPhysicalToWorldTransform->GetMatrix();
    //     center = m_GeometryItkPhysicalToWorldTransform->GetCenter();
    //     std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    //     offset = m_GeometryItkPhysicalToWorldTransform->GetOffset();
    //     std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    //     std::cout << std::endl;

    // init callback
    itk::ReceptorMemberCommand<QmitkRigidRegistrationSelectorView>::Pointer command = itk::ReceptorMemberCommand<QmitkRigidRegistrationSelectorView>::New();
    command->SetCallbackFunction(this, &QmitkRigidRegistrationSelectorView::SetOptimizerValue);
    int observer = m_Observer->AddObserver( itk::AnyEvent(), command );






    // Actually not necessary, but depending on itk some initial transform might be necessary
    // which is passed to the registration method in transform parameters.
    this->setTransformParameters();  

    std::vector<std::string> presets;

    if( m_Controls.m_PyramidGroup->isChecked() ){
      mitk::PyramidalRegistrationMethod::Pointer registration = mitk::PyramidalRegistrationMethod::New();

      // Set Presets
      std::string pre = m_Controls.m_Presets->text().toStdString();      
      Tokenize(pre, presets, ",");
      registration->SetPresets(presets);

      // We need this information about the type of transform later in the SetOptimizerValue method
      // to see which type of transform we have to update in the moving image geometry.
      mitk::RigidRegistrationTestPreset* preset = registration->GetPreset();
      itk::Array<double> transformValues = preset->getTransformValues(presets[0]);  
      m_TransformParameters->SetTransform(transformValues[0]);

      // Set Schedules
      std::string movingSchedule = m_Controls.m_MovingSchedule->text().toStdString();
      std::string fixedSchedule = m_Controls.m_FixedSchedule->text().toStdString();
      itk::Array2D<unsigned int> mSchedule = ParseSchedule(movingSchedule);
      itk::Array2D<unsigned int> fSchedule = ParseSchedule(fixedSchedule);      

      if(m_Controls.m_MatchHistograms->isChecked())
      {
        registration->SetMatchHistograms(true);
      } else{
        registration->SetMatchHistograms(false);
      }


      registration->SetMovingSchedule(mSchedule);
      registration->SetFixedSchedule(fSchedule);
      registration->SetObserver(m_Observer);
      registration->SetInterpolator(m_Controls.m_InterpolatorBox->currentIndex());
      registration->SetReferenceImage(fimage);
      registration->SetInput(mimage);
      if (mmimage.IsNotNull() && fmimage.IsNotNull())
      {
        registration->SetMovingMask(mmimage);
        registration->SetFixedMask(fmimage);
      }

      // m_Transform is needed for the initial transform
      registration->SetTransformParameters(m_TransformParameters); 


      double time(0.0);
      double tstart(0.0);
      tstart = clock();
      try
      {
        registration->Update();
      }
      catch (itk::ExceptionObject e)
      {
        LOG_INFO << "Caught exception: "<<e.GetDescription();
        QMessageBox::information( this, "Registration exception", e.GetDescription());
        mitk::ProgressBar::GetInstance()->Progress(20);
      }

      time += clock() - tstart;
      time = time / CLOCKS_PER_SEC;

      //printOut of the Time
      LOG_INFO << "Registration Time: " << time;


    }
    else{
      // init registration method
      mitk::ImageRegistrationMethod::Pointer registration = mitk::ImageRegistrationMethod::New();
      this->setMetricParameters();
      this->setOptimizerParameters();  

      registration->SetObserver(m_Observer);
      registration->SetInterpolator(m_Controls.m_InterpolatorBox->currentIndex());
      registration->SetReferenceImage(fimage);
      registration->SetInput(mimage);    
      if (mmimage.IsNotNull() && fmimage.IsNotNull())
      {
        registration->SetMovingMask(mmimage);
        registration->SetFixedMask(fmimage);
      }
      registration->SetTransformParameters(m_TransformParameters);
      registration->SetMetricParameters(m_MetricParameters);
      registration->SetOptimizerParameters(m_OptimizerParameters);




      double time(0.0);
      double tstart(0.0);
      tstart = clock();

      try
      {
        registration->Update();
      }
      catch (itk::ExceptionObject e)
      {
        LOG_INFO << "Caught exception: "<<e.GetDescription();
        QMessageBox::information( this, "Registration exception", e.GetDescription());
        mitk::ProgressBar::GetInstance()->Progress(20);
      }

      time += clock() - tstart;
      time = time / CLOCKS_PER_SEC;

      //printOut of the Time
      LOG_INFO << "Registration Time: " << time;
    }

    m_Observer->RemoveObserver(observer);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }  
}

itk::Array2D<unsigned int> QmitkRigidRegistrationSelectorView::ParseSchedule(std::string s)
{

  // READ OUT THE SCHEDULE: First separate every level of the schedule 
  std::vector<std::string> tokens;
  std::string delimiter = ";";
  Tokenize(s, tokens, delimiter);

  std::vector< std::vector<std::string> > levels;
  std::vector<std::string> level;
  delimiter = ",";
  // Then separate every dimension         
  for(unsigned int i=0; i<tokens.size(); i++)
  {          
    Tokenize(tokens[i], level, delimiter);
    levels.push_back(level);
    level.clear();
  }

  // Build up the schedule
  itk::Array2D<unsigned int> schedule;   

  schedule.SetSize(levels.size(), levels[0].size());
  for(unsigned int i=0; i<levels.size(); i++)
  {
    for(unsigned int j=0; j<levels[i].size(); j++)
    {
      schedule[i][j] = atoi( levels[i].at(j).c_str() );
      cout << "Schedule: " << schedule[i][j] << endl;
    }
  }   

  return schedule;
}

void QmitkRigidRegistrationSelectorView::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{

  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);      
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {        
    tokens.push_back(str.substr(lastPos, pos - lastPos));       
    lastPos = str.find_first_not_of(delimiters, pos);       
    pos = str.find_first_of(delimiters, lastPos);
  }
}


void QmitkRigidRegistrationSelectorView::SetFixedNode( mitk::DataTreeNode * fixedNode )
{
  m_FixedNode = fixedNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());  
}

void QmitkRigidRegistrationSelectorView::SetFixedDimension( int dimension )
{
  m_FixedDimension = dimension;
}

void QmitkRigidRegistrationSelectorView::SetMovingNode( mitk::DataTreeNode * movingNode )
{
  m_MovingNode = movingNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetMovingDimension(int dimension )
{
  m_MovingDimension = dimension;
}


/// method to make all TransformFrames invisible
void QmitkRigidRegistrationSelectorView::hideAllTransformFrames()
{
  m_Controls.m_TranslationTransformFrame->hide();
  m_Controls.m_ScaleTransformFrame->hide();
  m_Controls.m_ScaleLogarithmicTransformFrame->hide();
  m_Controls.m_AffineTransformFrame->hide();
  m_Controls.m_FixedCenterOfRotationAffineTransformFrame->hide();
  m_Controls.m_Rigid3DTransformFrame->hide();
  m_Controls.m_Euler3DTransformFrame->hide();
  m_Controls.m_CenteredEuler3DTransformFrame->hide();
  m_Controls.m_QuaternionRigidTransformFrame->hide();
  m_Controls.m_VersorFrame->hide();
  m_Controls.m_VersorRigid3DTransformFrame->hide();
  m_Controls.m_ScaleSkewVersorRigid3DTransformFrame->hide();
  m_Controls.m_Similarity3DTransformFrame->hide();
  m_Controls.m_Rigid2DTransformFrame->hide();
  m_Controls.m_CenteredRigid2DTransformFrame->hide();
  m_Controls.m_Euler2DTransformFrame->hide();
  m_Controls.m_Similarity2DTransformFrame->hide();
  m_Controls.m_CenteredSimilarity2DTransformFrame->hide();  
}

/// method to make all OptimizerFrames invisible
void QmitkRigidRegistrationSelectorView::hideAllOptimizerFrames()
{
  m_Controls.m_ExhaustiveFrame->hide();
  m_Controls.m_GradientDescentFrame->hide();
  m_Controls.m_QuaternionRigidTransformGradientDescentFrame->hide();
  m_Controls.m_LBFGSBFrame->hide();
  m_Controls.m_OnePlusOneEvolutionaryFrame->hide();
  m_Controls.m_PowellFrame->hide();
  m_Controls.m_FRPRFrame->hide();
  m_Controls.m_RegularStepGradientDescentFrame->hide();
  m_Controls.m_VersorTransformFrame->hide();
  m_Controls.m_AmoebaFrame->hide();
  m_Controls.m_ConjugateGradientFrame->hide();
  m_Controls.m_LBFGSFrame->hide();
  m_Controls.m_SPSAFrame->hide();
  m_Controls.m_VersorRigid3DFrame->hide();
}

/// method to make all MetricFrames invisible
void QmitkRigidRegistrationSelectorView::hideAllMetricFrames()
{
  m_Controls.m_MeanSquaresFrame->hide();
  m_Controls.m_NormalizedCorrelationFrame->hide();
  m_Controls.m_GradientDifferenceFrame->hide();
  m_Controls.m_KullbackLeiblerCompareHistogramFrame->hide();
  m_Controls.m_CorrelationCoefficientHistogramFrame->hide();
  m_Controls.m_MeanSquaresHistogramFrame->hide();
  m_Controls.m_MutualInformationHistogramFrame->hide();
  m_Controls.m_NormalizedMutualInformationHistogramFrame->hide();
  m_Controls.m_MattesMutualInformationFrame->hide();
  m_Controls.m_MeanReciprocalSquareDifferenceFrame->hide();
  m_Controls.m_MutualInformationFrame->hide();
  m_Controls.m_MatchCardinalityFrame->hide();
  m_Controls.m_KappaStatisticFrame->hide();
}

// this is a callback function that retrieves the current transformation
// parameters after every step of progress in the optimizer.
// depending on the choosen transformation, we construct a vtktransform
// that will be applied to the geometry of the moving image.
// the values are delivered by mitkRigidRgistrationObserver.cpp
void QmitkRigidRegistrationSelectorView::SetOptimizerValue( const itk::EventObject & )
{
  if (m_StopOptimization)
  {
    m_Observer->SetStopOptimization(true);
    m_StopOptimization = false;
  }



  // retreive optimizer value for the current transformation
  double value = m_Observer->GetCurrentOptimizerValue();

  // retreive current parameterset of the transformation
  itk::Array<double> transformParams = m_Observer->GetCurrentTranslation();

  // init an empty affine transformation that will be filled with
  // the corresponding transformation parameters in the following
  vtkMatrix4x4* vtkmatrix = vtkMatrix4x4::New();
  vtkmatrix->Identity();

  // init a transform that will be initialized with the vtkmatrix later
  vtkTransform* vtktransform = vtkTransform::New();

  if (m_MovingNode.IsNotNull())
  {
    if (m_TransformParameters->GetTransform() == mitk::TransformParameters::TRANSLATIONTRANSFORM)
    {
      if (transformParams.size() == 2)
      {
        vtktransform->Translate(transformParams[0], transformParams[1], 0);
      }
      else if (transformParams.size() == 3)
      {
        vtktransform->Translate(transformParams[0], transformParams[1], transformParams[2]);
        std::cout<<"Translation is: "<<transformParams[0] << transformParams[1] << transformParams[2] << std::endl;
      }
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::SCALETRANSFORM)
    {
      for( unsigned int i=0; i<transformParams.size(); i++)
      {
        vtkmatrix->SetElement(i, i, transformParams[i]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
    {
      for( unsigned int i=0; i<transformParams.size(); i++)
      {
        vtkmatrix->SetElement(i, i, transformParams[i]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::AFFINETRANSFORM)
    {

      // -  the 9 rotation-coefficients are copied to the
      //    directly to the top left part of the matrix
      int m = 0;
      for (int i = 0; i < m_FixedDimension; i++)
      {
        for (int j = 0; j < m_FixedDimension; j++)
        {
          vtkmatrix->SetElement(i, j, transformParams[m]);
          m++;
        }
      }

      // -  the 3 translation-coefficients are corrected to take
      //    into account the center of the transformation
      float center[4];
      center[0] = m_TransformParameters->GetTransformCenterX();
      center[1] = m_TransformParameters->GetTransformCenterY();
      center[2] = m_TransformParameters->GetTransformCenterZ();
      center[3] = 1;
      std::cout<< "rotation center: " << center[0] << " " << center[1] << " " << center [2] << std::endl;

      float translation[4];
      vtkmatrix->MultiplyPoint(center, translation);
      if (m_FixedDimension == 2)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[4]);
        vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[5]);
      }
      else if (m_FixedDimension == 3)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[9]);
        vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[10]);
        vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[11]);
      }

      // set the transform matrix to init the transform
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      int m = 0;
      for (int i = 0; i < m_FixedDimension; i++)
      {
        for (int j = 0; j < m_FixedDimension; j++)
        {
          vtkmatrix->SetElement(i, j, transformParams[m]);
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
        vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[4]);
        vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[5]);
      }
      else if (m_FixedDimension == 3)
      {
        vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[9]);
        vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[10]);
        vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[11]);
      }
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::RIGID3DTRANSFORM)
    {
      int m = 0;
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vtkmatrix->SetElement(i, j, transformParams[m]);
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
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[9]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[10]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[11]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::EULER3DTRANSFORM)
    {
      mitk::ScalarType angleX = transformParams[0] * 45.0 / atan(1.0);
      mitk::ScalarType angleY = transformParams[1] * 45.0 / atan(1.0);
      mitk::ScalarType angleZ = transformParams[2] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Translate(-m_TransformParameters->GetTransformCenterX(), -m_TransformParameters->GetTransformCenterY(), -m_TransformParameters->GetTransformCenterZ());
      vtktransform->RotateX(angleX);
      vtktransform->RotateY(angleY);
      vtktransform->RotateZ(angleZ);
      vtktransform->Translate(m_TransformParameters->GetTransformCenterX(), m_TransformParameters->GetTransformCenterY(), m_TransformParameters->GetTransformCenterZ());
      vtktransform->Translate(transformParams[3], transformParams[4], transformParams[5]);
      vtktransform->PreMultiply();
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
    {
      mitk::ScalarType angleX = transformParams[0] * 45.0 / atan(1.0);
      mitk::ScalarType angleY = transformParams[1] * 45.0 / atan(1.0);
      mitk::ScalarType angleZ = transformParams[2] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Translate(-m_TransformParameters->GetTransformCenterX(), -m_TransformParameters->GetTransformCenterY(), -m_TransformParameters->GetTransformCenterZ());
      vtktransform->RotateX(angleX);
      vtktransform->RotateY(angleY);
      vtktransform->RotateZ(angleZ);
      vtktransform->Translate(m_TransformParameters->GetTransformCenterX(), m_TransformParameters->GetTransformCenterY(), m_TransformParameters->GetTransformCenterZ());
      vtktransform->Translate(transformParams[3], transformParams[4], transformParams[5]);
      vtktransform->PreMultiply();
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
    {
      itk::QuaternionRigidTransform<double>::Pointer quaternionTransform = itk::QuaternionRigidTransform<double>::New();
      quaternionTransform->SetParameters(transformParams);
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
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[4]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[5]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[6]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::VERSORTRANSFORM)
    {
      itk::VersorTransform<double>::Pointer versorTransform = itk::VersorTransform<double>::New();
      versorTransform->SetParameters(transformParams);
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
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      itk::VersorRigid3DTransform<double>::Pointer versorTransform = itk::VersorRigid3DTransform<double>::New();
      versorTransform->SetParameters(transformParams);
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
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[3]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[4]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[5]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      itk::ScaleSkewVersor3DTransform<double>::Pointer versorTransform = itk::ScaleSkewVersor3DTransform<double>::New();
      versorTransform->SetParameters(transformParams);
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
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[3]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[4]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[5]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
    {
      itk::Similarity3DTransform<double>::Pointer similarityTransform = itk::Similarity3DTransform<double>::New();
      similarityTransform->SetParameters(transformParams);
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
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[4]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[5]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[6]);
      vtktransform->SetMatrix(vtkmatrix);
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::RIGID2DTRANSFORM)
    {
      mitk::ScalarType angle = transformParams[0] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->RotateZ(angle);
      vtktransform->Translate(transformParams[1], transformParams[2], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {      
      mitk::ScalarType angle = transformParams[0] * 45.0 / atan(1.0);;
      vtktransform->PostMultiply();
      vtktransform->Translate(-transformParams[1], -transformParams[2], 0);
      vtktransform->RotateZ(angle);
      vtktransform->Translate(transformParams[1], transformParams[2], 0);
      vtktransform->Translate(transformParams[3], transformParams[4], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::EULER2DTRANSFORM)
    {
      mitk::ScalarType angle = transformParams[0] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->RotateZ(angle);
      vtktransform->Translate(transformParams[1], transformParams[2], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
    {
      mitk::ScalarType angle = transformParams[1] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Scale(transformParams[0], transformParams[0], 1);
      vtktransform->RotateZ(angle);
      vtktransform->Translate(transformParams[2], transformParams[3], 0);
      vtktransform->PreMultiply();
    }
    else if (m_TransformParameters->GetTransform() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      mitk::ScalarType angle = transformParams[1] * 45.0 / atan(1.0);
      vtktransform->PostMultiply();
      vtktransform->Translate(-transformParams[2], -transformParams[3], 0);
      vtktransform->Scale(transformParams[0], transformParams[0], 1);
      vtktransform->RotateZ(angle);
      vtktransform->Translate(transformParams[2], transformParams[3], 0);
      vtktransform->Translate(transformParams[4], transformParams[5], 0);
      vtktransform->PreMultiply();
    }

    // the retrieved transform goes from fixed to moving space.
    // invert the transform in order to go from moving to fixed space.
    vtkMatrix4x4* vtkmatrix_inv = vtkMatrix4x4::New();
    vtktransform->GetInverse(vtkmatrix_inv);

    // now adapt the moving geometry accordingly
    m_MovingGeometry->GetIndexToWorldTransform()->SetIdentity();

    // the next view lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)

    // set moving image geometry to registration result
    m_MovingGeometry->SetIndexToWorldTransformByVtkMatrix(vtkmatrix_inv);

    /*std::cout << std::endl;
    std::cout << m_MovingGeometry->GetIndexToWorldTransform()->GetMatrix();
    mitk::Geometry3D::TransformType::OutputVectorType offset = m_MovingGeometry->GetIndexToWorldTransform()->GetOffset();
    std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;*/


#if !defined(ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE)
    // the next few lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)
    // go to itk physical space before applying the registration result
    m_MovingGeometry->Compose(m_GeometryWorldToItkPhysicalTransform, 1);

    // right in the beginning, transform by initial moving image geometry
    m_MovingGeometry->Compose(m_ImageGeometry->GetIndexToWorldTransform(), 1);

    // in the end, go back to world space
    m_MovingGeometry->Compose(m_GeometryItkPhysicalToWorldTransform, 0);  

#else     
    m_MovingGeometry->Compose(m_ImageGeometry->GetIndexToWorldTransform(), 1);
#endif


    /*std::cout << std::endl << m_MovingGeometry->GetIndexToWorldTransform()->GetMatrix();
    offset = m_MovingGeometry->GetIndexToWorldTransform()->GetOffset();
    std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl << std::endl;*/


    // now adapt all children geometries accordingly if children exist
    std::map<mitk::DataTreeNode::Pointer, mitk::Geometry3D*>::iterator iter;
    std::map<mitk::DataTreeNode::Pointer, mitk::AffineGeometryFrame3D::Pointer>::iterator iter2;
    mitk::DataTreeNode::Pointer childNode;
    for( iter = m_ChildNodes.begin(); iter != m_ChildNodes.end(); iter++ ) 
    {
      childNode = (*iter).first;
      if (childNode.IsNotNull())
      {
        mitk::Geometry3D* childGeometry;
        mitk::AffineGeometryFrame3D::Pointer childImageGeometry;
        // Calculate the World to ITK-Physical transform for the moving mask
        childGeometry = (*iter).second;
        iter2 = m_ChildNodes2.find(childNode);
        childImageGeometry = (*iter2).second;

        childGeometry->GetIndexToWorldTransform()->SetIdentity();

        // the next view lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)

        // set moving mask geometry to registration result
        childGeometry->SetIndexToWorldTransformByVtkMatrix(vtkmatrix_inv);
   
#if !defined(ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE)
        // the next few lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)
        // go to itk physical space before applying the registration result
        childGeometry->Compose(m_GeometryWorldToItkPhysicalTransform, 1);

        // right in the beginning, transform by initial moving image geometry
        childGeometry->Compose(childImageGeometry->GetIndexToWorldTransform(), 1);

        // in the end, go back to world space
        childGeometry->Compose(m_GeometryItkPhysicalToWorldTransform, 0);  

#else     
        childGeometry->Compose(childImageGeometry->GetIndexToWorldTransform(), 1);
#endif
      }
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  emit OptimizerChanged(value);
}

/// this method is called whenever the combobox with the selectable transforms changes
/// responsible for showing the selected transform parameters
void QmitkRigidRegistrationSelectorView::TransformSelected( int transform )
{
  this->hideAllTransformFrames();
  this->SetSimplexDeltaVisible();
  emit TransformChanged();
  if (m_FixedNode.IsNull() || m_MovingNode.IsNull())
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
      m_Controls.m_ScalesTranslationTransformTranslationZ->hide();
      m_Controls.textLabel4_4_2->hide();
    }
    else if (m_FixedDimension >= 3)
    {
      m_Controls.m_ScalesTranslationTransformTranslationZ->show();
      m_Controls.textLabel4_4_2->show();
    }
    m_Controls.m_TranslationTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::SCALETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_ScalesScaleTransformScaleZ->hide();
      m_Controls.textLabel3_5->hide();
    }
    else if (m_FixedDimension >= 3)
    {
      m_Controls.m_ScalesScaleTransformScaleZ->show();
      m_Controls.textLabel3_5->show();
    }
    m_Controls.m_ScaleTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->hide();
      m_Controls.textLabel3_5_3->hide();
    }
    else if (m_FixedDimension >= 3)
    {
      m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->show();
      m_Controls.textLabel3_5_3->show();
    }
    m_Controls.m_ScaleLogarithmicTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::AFFINETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_ScalesAffineTransformScale5->hide();
      m_Controls.m_ScalesAffineTransformScale6->hide();
      m_Controls.m_ScalesAffineTransformScale7->hide();
      m_Controls.m_ScalesAffineTransformScale8->hide();
      m_Controls.m_ScalesAffineTransformScale9->hide();
      m_Controls.m_ScalesAffineTransformScaleTranslationZ->hide();
      m_Controls.textLabel2_7->hide();
      m_Controls.textLabel3_6->hide();
      m_Controls.textLabel4_4->hide();
      m_Controls.textLabel5_4->hide();
      m_Controls.textLabel6_4->hide();
      m_Controls.textLabel13_2->hide();
    }
    else if (m_FixedDimension >= 3)
    {
      m_Controls.m_ScalesAffineTransformScale5->show();
      m_Controls.m_ScalesAffineTransformScale6->show();
      m_Controls.m_ScalesAffineTransformScale7->show();
      m_Controls.m_ScalesAffineTransformScale8->show();
      m_Controls.m_ScalesAffineTransformScale9->show();
      m_Controls.m_ScalesAffineTransformScaleTranslationZ->show();
      m_Controls.textLabel2_7->show();
      m_Controls.textLabel3_6->show();
      m_Controls.textLabel4_4->show();
      m_Controls.textLabel5_4->show();
      m_Controls.textLabel6_4->show();
      m_Controls.textLabel13_2->show();
    }
    m_Controls.m_AffineTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->hide();
      m_Controls.textLabel2_7_2_2->hide();
      m_Controls.textLabel3_6_2_2->hide();
      m_Controls.textLabel4_4_3_2->hide();
      m_Controls.textLabel5_4_2_2->hide();
      m_Controls.textLabel6_4_2_2->hide();
      m_Controls.textLabel13_2_2_2->hide();
    }
    else if (m_FixedDimension >= 3)
    {
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->show();
      m_Controls.textLabel2_7_2_2->show();
      m_Controls.textLabel3_6_2_2->show();
      m_Controls.textLabel4_4_3_2->show();
      m_Controls.textLabel5_4_2_2->show();
      m_Controls.textLabel6_4_2_2->show();
      m_Controls.textLabel13_2_2_2->show();
    }
    m_Controls.m_FixedCenterOfRotationAffineTransformFrame->show();
  }
  else if (transform == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_Rigid3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::EULER3DTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_Euler3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_CenteredEuler3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_QuaternionRigidTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::VERSORTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_VersorFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_VersorRigid3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_ScaleSkewVersorRigid3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    if (m_FixedDimension >= 3)
    {
      m_Controls.m_Similarity3DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::RIGID2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_Rigid2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_CenteredRigid2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_Euler2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_Similarity2DTransformFrame->show();
    }
  }
  else if (transform == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_CenteredSimilarity2DTransformFrame->show();
    }
  }
}

/// this method is called whenever the combobox with the selectable optimizers changes
/// responsible for showing the selected optimizer parameters
void QmitkRigidRegistrationSelectorView::OptimizerSelected( int optimizer )
{
  this->hideAllOptimizerFrames();
  this->SetSimplexDeltaVisible();
  if (optimizer == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
  {
    m_Controls.m_ExhaustiveFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
  {
    m_Controls.m_GradientDescentFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
  {
    m_Controls.m_QuaternionRigidTransformGradientDescentFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::LBFGSOPTIMIZER)
  {
    m_Controls.m_LBFGSFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
  {
    m_Controls.m_OnePlusOneEvolutionaryFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::POWELLOPTIMIZER)
  {
    m_Controls.m_PowellFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::FRPROPTIMIZER)
  {
    m_Controls.m_FRPRFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
  {
    m_Controls.m_RegularStepGradientDescentFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
  {
    m_Controls.m_VersorTransformFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
  {
    m_Controls.m_AmoebaFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
  {
    m_Controls.m_ConjugateGradientFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::LBFGSBOPTIMIZER)
  {
    m_Controls.m_LBFGSBFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::SPSAOPTIMIZER)
  {
    m_Controls.m_SPSAFrame->show();
  }
  else if (optimizer == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
  {
    m_Controls.m_VersorRigid3DFrame->show();
  }
}

/// this method is called whenever the combobox with the selectable metrics changes
/// responsible for showing the selected metric parameters
void QmitkRigidRegistrationSelectorView::MetricSelected( int metric )
{
  this->hideAllMetricFrames();
  if (metric == mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MeanSquaresFrame->show();
  }
  else if (metric == mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NormalizedCorrelationFrame->show();
  }
  else if (metric == mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_GradientDifferenceFrame->show();
  }
  else if (metric == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_KullbackLeiblerCompareHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_CorrelationCoefficientHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MeanSquaresHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MutualInformationHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NormalizedMutualInformationHistogramFrame->show();
  }
  else if (metric == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MattesMutualInformationFrame->show();
  }
  else if (metric == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MeanReciprocalSquareDifferenceFrame->show();
  }
  else if (metric == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MutualInformationFrame->show();
  }
  else if (metric == mitk::MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_MatchCardinalityFrame->show();
  }
  else if (metric == mitk::MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_KappaStatisticFrame->show();
  }
}

/// this method writes the transform parameters from the selected transform into mitkTransformParameters class
/// so that the desired transform can be build up with these parameters
void QmitkRigidRegistrationSelectorView::setTransformParameters()
{
  //Calculate an initial transform to give to m_TransformParameters
  m_TransformParameters = mitk::TransformParameters::New();
  m_TransformParameters->SetTransform(m_Controls.m_TransformBox->currentIndex());

  itk::Array<double> m_Scales;

  if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::TRANSLATIONTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesTranslation->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_Controls.m_ScalesTranslationTransformTranslationX->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesTranslationTransformTranslationY->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesTranslationTransformTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALETRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesScale->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_Controls.m_ScalesScaleTransformScaleX->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesScaleTransformScaleY->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesScaleTransformScaleZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesScaleLogarithmic->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_Controls.m_ScalesScaleLogarithmicTransformScaleX->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesScaleLogarithmicTransformScaleY->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::AFFINETRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesAffine->isChecked())
    {
      m_Scales.SetSize(12);
      m_Scales[0] = m_Controls.m_ScalesAffineTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesAffineTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesAffineTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesAffineTransformScale4->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesAffineTransformScale5->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesAffineTransformScale6->text().toDouble();
      m_Scales[6] = m_Controls.m_ScalesAffineTransformScale7->text().toDouble();
      m_Scales[7] = m_Controls.m_ScalesAffineTransformScale8->text().toDouble();
      m_Scales[8] = m_Controls.m_ScalesAffineTransformScale9->text().toDouble();
      m_Scales[9] = m_Controls.m_ScalesAffineTransformScaleTranslationX->text().toDouble();
      m_Scales[10] = m_Controls.m_ScalesAffineTransformScaleTranslationY->text().toDouble();
      m_Scales[11] = m_Controls.m_ScalesAffineTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }

    m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerAffine->isChecked());

#if !defined(ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE)
    mitk::Geometry3D::TransformType::Pointer initialTransform = mitk::Geometry3D::TransformType::New();
    initialTransform->SetIdentity();
    initialTransform->Compose(m_GeometryItkPhysicalToWorldTransform);
    initialTransform->Compose(m_GeometryWorldToItkPhysicalTransform, 0); 
    m_TransformParameters->SetInitialParameters(initialTransform->GetParameters());  
#endif

    m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsAffine->isChecked());
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked())
    {
      m_Scales.SetSize(12);
      m_Scales[0] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->text().toDouble();
      m_Scales[6] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->text().toDouble();
      m_Scales[7] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->text().toDouble();
      m_Scales[8] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->text().toDouble();
      m_Scales[9] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->text().toDouble();
      m_Scales[10] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->text().toDouble();
      m_Scales[11] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsFixedCenterOfRotationAffine->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesRigid3D->isChecked())
    {
      m_Scales.SetSize(12);
      m_Scales[0] = m_Controls.m_ScalesRigid3DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesRigid3DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesRigid3DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesRigid3DTransformScale4->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesRigid3DTransformScale5->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesRigid3DTransformScale6->text().toDouble();
      m_Scales[6] = m_Controls.m_ScalesRigid3DTransformScale7->text().toDouble();
      m_Scales[7] = m_Controls.m_ScalesRigid3DTransformScale8->text().toDouble();
      m_Scales[8] = m_Controls.m_ScalesRigid3DTransformScale9->text().toDouble();
      m_Scales[9] = m_Controls.m_ScalesRigid3DTransformScaleTranslationX->text().toDouble();
      m_Scales[10] = m_Controls.m_ScalesRigid3DTransformScaleTranslationY->text().toDouble();
      m_Scales[11] = m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerRigid3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerRigid3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsRigid3D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::EULER3DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesEuler3D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_Controls.m_ScalesEuler3DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesEuler3DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesEuler3DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesEuler3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesEuler3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerEuler3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerEuler3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsEuler3D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesCenteredEuler3D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_Controls.m_ScalesCenteredEuler3DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesCenteredEuler3DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesCenteredEuler3DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerCenteredEuler3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerCenteredEuler3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsCenteredEuler3D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesQuaternionRigid->isChecked())
    {
      m_Scales.SetSize(7);
      m_Scales[0] = m_Controls.m_ScalesQuaternionRigidTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesQuaternionRigidTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesQuaternionRigidTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesQuaternionRigidTransformScale4->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->text().toDouble();
      m_Scales[6] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerQuaternionRigid->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerQuaternionRigid->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsQuaternionRigid->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::VERSORTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesVersor->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_Controls.m_ScalesVersorTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesVersorTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesVersorTransformScale3->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerVersor->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerVersor->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsVersor->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesVersorRigid3D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_Controls.m_ScalesVersorRigid3DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesVersorRigid3DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesVersorRigid3DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerVersorRigid3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerVersorRigid3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsVersorRigid3D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked())
    {
      m_Scales.SetSize(15);
      m_Scales[0] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->text().toDouble();
      m_Scales[4] = m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      m_Scales[6] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->text().toDouble();
      m_Scales[7] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->text().toDouble();
      m_Scales[8] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->text().toDouble();
      m_Scales[9] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->text().toDouble();
      m_Scales[10] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->text().toDouble();
      m_Scales[11] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->text().toDouble();
      m_Scales[12] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->text().toDouble();
      m_Scales[13] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->text().toDouble();
      m_Scales[14] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsScaleSkewVersorRigid3D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesSimilarity3D->isChecked())
    {
      m_Scales.SetSize(7);
      m_Scales[0] = m_Controls.m_ScalesSimilarity3DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesSimilarity3DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesSimilarity3DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesSimilarity3DTransformScale4->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->text().toDouble();
      m_Scales[6] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerSimilarity3D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerSimilarity3D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsSimilarity3D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::RIGID2DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesRigid2D->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_Controls.m_ScalesRigid2DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesRigid2DTransformScaleTranslationX->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesRigid2DTransformScaleTranslationY->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerRigid2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerRigid2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsRigid2D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesCenteredRigid2D->isChecked())
    {
      m_Scales.SetSize(5);
      m_Scales[0] = m_Controls.m_RotationScaleCenteredRigid2D->text().toDouble();
      m_Scales[1] = m_Controls.m_CenterXScaleCenteredRigid2D->text().toDouble();
      m_Scales[2] = m_Controls.m_CenterYScaleCenteredRigid2D->text().toDouble();
      m_Scales[3] = m_Controls.m_TranslationXScaleCenteredRigid2D->text().toDouble();
      m_Scales[4] = m_Controls.m_TranslationYScaleCenteredRigid2D->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    m_TransformParameters->SetAngle(m_Controls.m_AngleCenteredRigid2D->text().toFloat());
    if (m_Controls.m_CenterForInitializerCenteredRigid2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerCenteredRigid2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsCenteredRigid2D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesEuler2D->isChecked())
    {
      m_Scales.SetSize(3);
      m_Scales[0] = m_Controls.m_RotationScaleEuler2D->text().toDouble();
      m_Scales[1] = m_Controls.m_TranslationXScaleEuler2D->text().toDouble();
      m_Scales[2] = m_Controls.m_TranslationYScaleEuler2D->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    if (m_Controls.m_CenterForInitializerEuler2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerEuler2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsEuler2D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesSimilarity2D->isChecked())
    {
      m_Scales.SetSize(4);
      m_Scales[0] = m_Controls.m_ScalingScaleSimilarity2D->text().toDouble();
      m_Scales[1] = m_Controls.m_RotationScaleSimilarity2D->text().toDouble();
      m_Scales[2] = m_Controls.m_TranslationXScaleSimilarity2D->text().toDouble();
      m_Scales[3] = m_Controls.m_TranslationYScaleSimilarity2D->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    m_TransformParameters->SetScale(m_Controls.m_InitialScaleSimilarity2D->text().toFloat());
    m_TransformParameters->SetAngle(m_Controls.m_AngleSimilarity2D->text().toFloat());
    if (m_Controls.m_CenterForInitializerSimilarity2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerSimilarity2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsSimilarity2D->isChecked());
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    if (m_Controls.m_UseOptimizerScalesCenteredSimilarity2D->isChecked())
    {
      m_Scales.SetSize(6);
      m_Scales[0] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->text().toDouble();
      m_Scales[1] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->text().toDouble();
      m_Scales[2] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->text().toDouble();
      m_Scales[3] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->text().toDouble();
      m_Scales[4] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->text().toDouble();
      m_Scales[5] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->text().toDouble();
      m_TransformParameters->SetScales(m_Scales);
    }
    m_TransformParameters->SetScale(m_Controls.m_InitialScaleCenteredSimilarity2D->text().toFloat());
    m_TransformParameters->SetAngle(m_Controls.m_AngleCenteredSimilarity2D->text().toFloat());
    if (m_Controls.m_CenterForInitializerCenteredSimilarity2D->isChecked())
    {
      m_TransformParameters->SetTransformInitializerOn(m_Controls.m_CenterForInitializerCenteredSimilarity2D->isChecked());
      m_TransformParameters->SetMomentsOn(m_Controls.m_MomentsCenteredSimilarity2D->isChecked());
    }
  }  
}

/// this method writes the optimizer parameters from the selected optimizer into mitkOptimizerParameters class
/// so that the desired optimizer can be build up with these parameters
void QmitkRigidRegistrationSelectorView::setOptimizerParameters()
{
  m_OptimizerParameters = mitk::OptimizerParameters::New();
  m_OptimizerParameters->SetOptimizer(m_Controls.m_OptimizerBox->currentIndex());
  m_OptimizerParameters->SetDimension(m_FixedDimension);
  m_OptimizerParameters->SetMaximize(m_Controls.m_Maximize->isChecked());

  if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
  {
    m_OptimizerParameters->SetStepLengthExhaustive(m_Controls.m_StepLengthExhaustive->text().toFloat());
    m_OptimizerParameters->SetNumberOfStepsExhaustive(m_Controls.m_NumberOfStepsExhaustive->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
  {
    m_OptimizerParameters->SetLearningRateGradientDescent(m_Controls.m_LearningRateGradientDescent->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsGradientDescent(m_Controls.m_IterationsGradientDescent->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
  {
    m_OptimizerParameters->SetLearningRateQuaternionRigidTransformGradientDescent(m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsQuaternionRigidTransformGradientDescent(m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::LBFGSBOPTIMIZER)
  {

  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
  {
    m_OptimizerParameters->SetShrinkFactorOnePlusOneEvolutionary(m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetGrowthFactorOnePlusOneEvolutionary(m_Controls.m_GrowthFactorOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetEpsilonOnePlusOneEvolutionary(m_Controls.m_EpsilonOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetInitialRadiusOnePlusOneEvolutionary(m_Controls.m_InitialRadiusOnePlusOneEvolutionary->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsOnePlusOneEvolutionary(m_Controls.m_IterationsOnePlusOneEvolutionary->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::POWELLOPTIMIZER)
  {
    m_OptimizerParameters->SetStepLengthPowell(m_Controls.m_StepLengthPowell->text().toFloat());
    m_OptimizerParameters->SetStepTolerancePowell(m_Controls.m_StepTolerancePowell->text().toFloat());
    m_OptimizerParameters->SetValueTolerancePowell(m_Controls.m_ValueTolerancePowell->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsPowell(m_Controls.m_IterationsPowell->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::FRPROPTIMIZER)
  {
    m_OptimizerParameters->SetFletchReevesFRPR(m_Controls.m_FletchReevesFRPR->isChecked());
    m_OptimizerParameters->SetPolakRibiereFRPR(m_Controls.m_PolakRibiereFRPR->isChecked());
    m_OptimizerParameters->SetStepLengthFRPR(m_Controls.m_StepLengthFRPR->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsFRPR(m_Controls.m_IterationsFRPR->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientMagnitudeToleranceRegularStepGradientDescent(m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetMinimumStepLengthRegularStepGradientDescent(m_Controls.m_MinimumStepLengthRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetMaximumStepLengthRegularStepGradientDescent(m_Controls.m_MaximumStepLengthRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetRelaxationFactorRegularStepGradientDescent(m_Controls.m_RelaxationFactorRegularStepGradientDescent->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsRegularStepGradientDescent(m_Controls.m_IterationsRegularStepGradientDescent->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientMagnitudeToleranceVersorTransform(m_Controls.m_GradientMagnitudeToleranceVersorTransform->text().toFloat());
    m_OptimizerParameters->SetMinimumStepLengthVersorTransform(m_Controls.m_MinimumStepLengthVersorTransform->text().toFloat());
    m_OptimizerParameters->SetMaximumStepLengthVersorTransform(m_Controls.m_MaximumStepLengthVersorTransform->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsVersorTransform(m_Controls.m_IterationsVersorTransform->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
  {
    itk::Array<double> simplexDelta;
    simplexDelta.SetSize(15);
    simplexDelta[0] = m_Controls.m_SimplexDeltaAmoeba1->text().toDouble();
    simplexDelta[1] = m_Controls.m_SimplexDeltaAmoeba2->text().toDouble();
    simplexDelta[2] = m_Controls.m_SimplexDeltaAmoeba3->text().toDouble();
    simplexDelta[3] = m_Controls.m_SimplexDeltaAmoeba4->text().toDouble();
    simplexDelta[4] = m_Controls.m_SimplexDeltaAmoeba5->text().toDouble();
    simplexDelta[5] = m_Controls.m_SimplexDeltaAmoeba6->text().toDouble();
    simplexDelta[6] = m_Controls.m_SimplexDeltaAmoeba7->text().toDouble();
    simplexDelta[7] = m_Controls.m_SimplexDeltaAmoeba8->text().toDouble();
    simplexDelta[8] = m_Controls.m_SimplexDeltaAmoeba9->text().toDouble();
    simplexDelta[9] = m_Controls.m_SimplexDeltaAmoeba10->text().toDouble();
    simplexDelta[10] = m_Controls.m_SimplexDeltaAmoeba11->text().toDouble();
    simplexDelta[11] = m_Controls.m_SimplexDeltaAmoeba12->text().toDouble();
    simplexDelta[12] = m_Controls.m_SimplexDeltaAmoeba13->text().toDouble();
    simplexDelta[13] = m_Controls.m_SimplexDeltaAmoeba14->text().toDouble();
    simplexDelta[14] = m_Controls.m_SimplexDeltaAmoeba15->text().toDouble();
    m_OptimizerParameters->SetSimplexDeltaAmoeba(simplexDelta);
    m_OptimizerParameters->SetParametersConvergenceToleranceAmoeba(m_Controls.m_ParametersConvergenceToleranceAmoeba->text().toFloat());
    m_OptimizerParameters->SetFunctionConvergenceToleranceAmoeba(m_Controls.m_FunctionConvergenceToleranceAmoeba->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsAmoeba(m_Controls.m_IterationsAmoeba->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
  {
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::LBFGSOPTIMIZER)
  {
    m_OptimizerParameters->SetGradientConvergenceToleranceLBFGS(m_Controls.m_GradientMagnitudeToleranceLBFGS->text().toFloat());
    m_OptimizerParameters->SetLineSearchAccuracyLBFGS(m_Controls.m_LineSearchAccuracyLBFGS->text().toFloat());
    m_OptimizerParameters->SetDefaultStepLengthLBFGS(m_Controls.m_DefaultStepLengthLBFGS->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsLBFGS(m_Controls.m_MaximumEvaluationsLBFGS->text().toInt());
    m_OptimizerParameters->SetTraceOnLBFGS(m_Controls.m_TraceOnLBFGS->isChecked());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::SPSAOPTIMIZER)
  {
    m_OptimizerParameters->SetaSPSA(m_Controls.m_aSPSA->text().toFloat());
    m_OptimizerParameters->SetASPSA(m_Controls.m_ASPSA->text().toFloat());
    m_OptimizerParameters->SetAlphaSPSA(m_Controls.m_AlphaSPSA->text().toFloat());
    m_OptimizerParameters->SetcSPSA(m_Controls.m_cSPSA->text().toFloat());
    m_OptimizerParameters->SetGammaSPSA(m_Controls.m_GammaSPSA->text().toFloat());
    m_OptimizerParameters->SetToleranceSPSA(m_Controls.m_ToleranceSPSA->text().toFloat());
    m_OptimizerParameters->SetStateOfConvergenceDecayRateSPSA(m_Controls.m_StateOfConvergenceDecayRateSPSA->text().toFloat());
    m_OptimizerParameters->SetMinimumNumberOfIterationsSPSA(m_Controls.m_MinimumNumberOfIterationsSPSA->text().toInt());
    m_OptimizerParameters->SetNumberOfPerturbationsSPSA(m_Controls.m_NumberOfPerturbationsSPSA->text().toInt());
    m_OptimizerParameters->SetNumberOfIterationsSPSA(m_Controls.m_IterationsSPSA->text().toInt());
  }
  else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
  {
    //TODO: CREATE INTERFACE FOR THIS. THE INTERFACE COMPONENTS USED HERE WERE PART OF THE LBFGS OPTIMIZER FRAME
    //WHICH WAS CHANGED BECAUSE IT DIDN'T MATCH THE LBFGS OPTIMIZER PARAMETERS.
    /*m_OptimizerParameters->SetGradientMagnitudeToleranceVersorRigid3DTransform(m_Controls.m_GradientConvergenceToleranceLBFGSB->text().toFloat());
    m_OptimizerParameters->SetMinimumStepLengthVersorRigid3DTransform(m_Controls.m_MinimumStepLengthVersorRigid3DTransform->text().toFloat());
    m_OptimizerParameters->SetMaximumStepLengthVersorRigid3DTransform(m_Controls.m_MaximumStepLengthVersorRigid3DTransform->text().toFloat());
    m_OptimizerParameters->SetNumberOfIterationsVersorRigid3DTransform(m_Controls.m_IterationsVersorRigid3DTransform->text().toInt());*/
  }
}

/// this method writes the metric parameters from the selected metric into mitkMetricParameters class
/// so that the desired metric can be build up with these parameters
void QmitkRigidRegistrationSelectorView::setMetricParameters()
{
  m_MetricParameters = mitk::MetricParameters::New();
  m_MetricParameters->SetMetric(m_Controls.m_MetricBox->currentIndex());
  m_MetricParameters->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsKullbackLeiblerCompareHistogram(m_Controls.m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsCorrelationCoefficientHistogram(m_Controls.m_NumberOfHistogramBinsCorrelationCoefficientHistogram->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsMeanSquaresHistogram(m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsMutualInformationHistogram(m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetNumberOfHistogramBinsNormalizedMutualInformationHistogram(m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetUseSamplesMattesMutualInformation(m_Controls.m_UseSamplingMattesMutualInformation->isChecked());
    m_MetricParameters->SetSpatialSamplesMattesMutualInformation(m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->text().toInt());
    m_MetricParameters->SetNumberOfHistogramBinsMattesMutualInformation(m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_MetricParameters->SetLambdaMeanReciprocalSquareDifference(m_Controls.m_LambdaMeanReciprocalSquareDifference->text().toInt());
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    mitk::Image* fixedImage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    unsigned int nPixel = 1;
    for (int i = 0; i < m_FixedDimension; i++)
    {
      nPixel *= fixedImage->GetDimension(i);
    }
    m_MetricParameters->SetSpatialSamplesMutualInformation(nPixel * m_Controls.m_NumberOfSpatialSamplesMutualInformation->text().toInt()/100);
    m_MetricParameters->SetFixedImageStandardDeviationMutualInformation(m_Controls.m_FixedImageStandardDeviationMutualInformation->text().toFloat());
    m_MetricParameters->SetMovingImageStandardDeviationMutualInformation(m_Controls.m_MovingImageStandardDeviationMutualInformation->text().toFloat());
    if (m_Controls.m_UseNormalizerAndSmoother)
    {
      m_MetricParameters->SetUseNormalizerAndSmootherMutualInformation(m_Controls.m_UseNormalizerAndSmoother->isChecked());
      m_MetricParameters->SetFixedSmootherVarianceMutualInformation(m_Controls.m_FixedSmootherVarianceMutualInformation->text().toFloat());
      m_MetricParameters->SetMovingSmootherVarianceMutualInformation(m_Controls.m_MovingSmootherVarianceMutualInformation->text().toFloat());
    }
    else
    {
      m_MetricParameters->SetUseNormalizerAndSmootherMutualInformation(m_Controls.m_UseNormalizerAndSmoother->isChecked());
    }
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC)
  {
  }
  else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
  {
  }
}

/// this method is needed because the Amoeba optimizer needs a simplex delta parameter, which defines the search range for each transform parameter
/// every transform has its own set of parameters, so this method looks for every time showing the correct number of simplex delta input fields for the
/// Amoeba optimizer
void QmitkRigidRegistrationSelectorView::SetSimplexDeltaVisible()
{
  m_Controls.m_SimplexDeltaAmoebaLabel1->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel2->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel3->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel4->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel5->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel6->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel7->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel8->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel9->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel10->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel11->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel12->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel13->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel14->hide();
  m_Controls.m_SimplexDeltaAmoebaLabel15->hide();
  m_Controls.m_SimplexDeltaAmoeba1->hide();
  m_Controls.m_SimplexDeltaAmoeba2->hide();
  m_Controls.m_SimplexDeltaAmoeba3->hide();
  m_Controls.m_SimplexDeltaAmoeba4->hide();
  m_Controls.m_SimplexDeltaAmoeba5->hide();
  m_Controls.m_SimplexDeltaAmoeba6->hide();
  m_Controls.m_SimplexDeltaAmoeba7->hide();
  m_Controls.m_SimplexDeltaAmoeba8->hide();
  m_Controls.m_SimplexDeltaAmoeba9->hide();
  m_Controls.m_SimplexDeltaAmoeba10->hide();
  m_Controls.m_SimplexDeltaAmoeba11->hide();
  m_Controls.m_SimplexDeltaAmoeba12->hide();
  m_Controls.m_SimplexDeltaAmoeba13->hide();
  m_Controls.m_SimplexDeltaAmoeba14->hide();
  m_Controls.m_SimplexDeltaAmoeba15->hide();
  if (m_FixedNode.IsNull() || m_MovingNode.IsNull())
  {
    return;
  }
  else if (m_FixedDimension != m_MovingDimension)
  {
    return;
  }
  if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::TRANSLATIONTRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALETRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_SimplexDeltaAmoebaLabel1->show();
      m_Controls.m_SimplexDeltaAmoeba1->show();
      m_Controls.m_SimplexDeltaAmoebaLabel2->show();
      m_Controls.m_SimplexDeltaAmoeba2->show();
    }
    else if (m_FixedDimension == 3)
    {
      m_Controls.m_SimplexDeltaAmoebaLabel1->show();
      m_Controls.m_SimplexDeltaAmoeba1->show();
      m_Controls.m_SimplexDeltaAmoebaLabel2->show();
      m_Controls.m_SimplexDeltaAmoeba2->show();
      m_Controls.m_SimplexDeltaAmoebaLabel3->show();
      m_Controls.m_SimplexDeltaAmoeba3->show();
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::AFFINETRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    if (m_FixedDimension == 2)
    {
      m_Controls.m_SimplexDeltaAmoebaLabel1->show();
      m_Controls.m_SimplexDeltaAmoeba1->show();
      m_Controls.m_SimplexDeltaAmoebaLabel2->show();
      m_Controls.m_SimplexDeltaAmoeba2->show();
      m_Controls.m_SimplexDeltaAmoebaLabel3->show();
      m_Controls.m_SimplexDeltaAmoeba3->show();
      m_Controls.m_SimplexDeltaAmoebaLabel4->show();
      m_Controls.m_SimplexDeltaAmoeba4->show();
      m_Controls.m_SimplexDeltaAmoebaLabel5->show();
      m_Controls.m_SimplexDeltaAmoeba5->show();
      m_Controls.m_SimplexDeltaAmoebaLabel6->show();
      m_Controls.m_SimplexDeltaAmoeba6->show();
    }
    else if (m_FixedDimension == 3)
    {
      m_Controls.m_SimplexDeltaAmoebaLabel1->show();
      m_Controls.m_SimplexDeltaAmoeba1->show();
      m_Controls.m_SimplexDeltaAmoebaLabel2->show();
      m_Controls.m_SimplexDeltaAmoeba2->show();
      m_Controls.m_SimplexDeltaAmoebaLabel3->show();
      m_Controls.m_SimplexDeltaAmoeba3->show();
      m_Controls.m_SimplexDeltaAmoebaLabel4->show();
      m_Controls.m_SimplexDeltaAmoeba4->show();
      m_Controls.m_SimplexDeltaAmoebaLabel5->show();
      m_Controls.m_SimplexDeltaAmoeba5->show();
      m_Controls.m_SimplexDeltaAmoebaLabel6->show();
      m_Controls.m_SimplexDeltaAmoeba6->show();
      m_Controls.m_SimplexDeltaAmoebaLabel7->show();
      m_Controls.m_SimplexDeltaAmoeba7->show();
      m_Controls.m_SimplexDeltaAmoebaLabel8->show();
      m_Controls.m_SimplexDeltaAmoeba8->show();
      m_Controls.m_SimplexDeltaAmoebaLabel9->show();
      m_Controls.m_SimplexDeltaAmoeba9->show();
      m_Controls.m_SimplexDeltaAmoebaLabel10->show();
      m_Controls.m_SimplexDeltaAmoeba10->show();
      m_Controls.m_SimplexDeltaAmoebaLabel11->show();
      m_Controls.m_SimplexDeltaAmoeba11->show();
      m_Controls.m_SimplexDeltaAmoebaLabel12->show();
      m_Controls.m_SimplexDeltaAmoeba12->show();
    }
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
    m_Controls.m_SimplexDeltaAmoebaLabel4->show();
    m_Controls.m_SimplexDeltaAmoeba4->show();
    m_Controls.m_SimplexDeltaAmoebaLabel5->show();
    m_Controls.m_SimplexDeltaAmoeba5->show();
    m_Controls.m_SimplexDeltaAmoebaLabel6->show();
    m_Controls.m_SimplexDeltaAmoeba6->show();
    m_Controls.m_SimplexDeltaAmoebaLabel7->show();
    m_Controls.m_SimplexDeltaAmoeba7->show();
    m_Controls.m_SimplexDeltaAmoebaLabel8->show();
    m_Controls.m_SimplexDeltaAmoeba8->show();
    m_Controls.m_SimplexDeltaAmoebaLabel9->show();
    m_Controls.m_SimplexDeltaAmoeba9->show();
    m_Controls.m_SimplexDeltaAmoebaLabel10->show();
    m_Controls.m_SimplexDeltaAmoeba10->show();
    m_Controls.m_SimplexDeltaAmoebaLabel11->show();
    m_Controls.m_SimplexDeltaAmoeba11->show();
    m_Controls.m_SimplexDeltaAmoebaLabel12->show();
    m_Controls.m_SimplexDeltaAmoeba12->show();
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::EULER3DTRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
    m_Controls.m_SimplexDeltaAmoebaLabel4->show();
    m_Controls.m_SimplexDeltaAmoeba4->show();
    m_Controls.m_SimplexDeltaAmoebaLabel5->show();
    m_Controls.m_SimplexDeltaAmoeba5->show();
    m_Controls.m_SimplexDeltaAmoebaLabel6->show();
    m_Controls.m_SimplexDeltaAmoeba6->show();
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
    m_Controls.m_SimplexDeltaAmoebaLabel4->show();
    m_Controls.m_SimplexDeltaAmoeba4->show();
    m_Controls.m_SimplexDeltaAmoebaLabel5->show();
    m_Controls.m_SimplexDeltaAmoeba5->show();
    m_Controls.m_SimplexDeltaAmoebaLabel6->show();
    m_Controls.m_SimplexDeltaAmoeba6->show();
    m_Controls.m_SimplexDeltaAmoebaLabel7->show();
    m_Controls.m_SimplexDeltaAmoeba7->show();
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::VERSORTRANSFORM 
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::RIGID2DTRANSFORM
    || m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
  }

  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
    m_Controls.m_SimplexDeltaAmoebaLabel4->show();
    m_Controls.m_SimplexDeltaAmoeba4->show();
    m_Controls.m_SimplexDeltaAmoebaLabel5->show();
    m_Controls.m_SimplexDeltaAmoeba5->show();
    m_Controls.m_SimplexDeltaAmoebaLabel6->show();
    m_Controls.m_SimplexDeltaAmoeba6->show();
    m_Controls.m_SimplexDeltaAmoebaLabel7->show();
    m_Controls.m_SimplexDeltaAmoeba7->show();
    m_Controls.m_SimplexDeltaAmoebaLabel8->show();
    m_Controls.m_SimplexDeltaAmoeba8->show();
    m_Controls.m_SimplexDeltaAmoebaLabel9->show();
    m_Controls.m_SimplexDeltaAmoeba9->show();
    m_Controls.m_SimplexDeltaAmoebaLabel10->show();
    m_Controls.m_SimplexDeltaAmoeba10->show();
    m_Controls.m_SimplexDeltaAmoebaLabel11->show();
    m_Controls.m_SimplexDeltaAmoeba11->show();
    m_Controls.m_SimplexDeltaAmoebaLabel12->show();
    m_Controls.m_SimplexDeltaAmoeba12->show();
    m_Controls.m_SimplexDeltaAmoebaLabel13->show();
    m_Controls.m_SimplexDeltaAmoeba13->show();
    m_Controls.m_SimplexDeltaAmoebaLabel14->show();
    m_Controls.m_SimplexDeltaAmoeba14->show();
    m_Controls.m_SimplexDeltaAmoebaLabel15->show();
    m_Controls.m_SimplexDeltaAmoeba15->show();
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
    m_Controls.m_SimplexDeltaAmoebaLabel4->show();
    m_Controls.m_SimplexDeltaAmoeba4->show();
    m_Controls.m_SimplexDeltaAmoebaLabel5->show();
    m_Controls.m_SimplexDeltaAmoeba5->show();
  }
  else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    m_Controls.m_SimplexDeltaAmoebaLabel1->show();
    m_Controls.m_SimplexDeltaAmoeba1->show();
    m_Controls.m_SimplexDeltaAmoebaLabel2->show();
    m_Controls.m_SimplexDeltaAmoeba2->show();
    m_Controls.m_SimplexDeltaAmoebaLabel3->show();
    m_Controls.m_SimplexDeltaAmoeba3->show();
    m_Controls.m_SimplexDeltaAmoebaLabel4->show();
    m_Controls.m_SimplexDeltaAmoeba4->show();
  }
}

void QmitkRigidRegistrationSelectorView::LoadRigidRegistrationParameter()
{
  this->DoLoadRigidRegistrationParameter(false);
}

void QmitkRigidRegistrationSelectorView::LoadRigidRegistrationTestParameter()
{
  this->DoLoadRigidRegistrationParameter(true);
}

void QmitkRigidRegistrationSelectorView::DoLoadRigidRegistrationParameter(bool testPreset)
{
  std::map<std::string, itk::Array<double> > existingPresets;
  if (testPreset)
  {
    existingPresets = m_TestPreset->getTransformValuesPresets(); 
  }
  else
  {
    existingPresets = m_Preset->getTransformValuesPresets();
  }
   
  std::map<std::string, itk::Array<double> >::iterator iter;
  std::list<std::string> presets;
  for( iter = existingPresets.begin(); iter != existingPresets.end(); iter++ ) 
  {
    presets.push_back( (*iter).first );
  }
  if (presets.empty())
  {
    QMessageBox::warning( NULL, "RigidRegistrationParameters.xml", "RigidRegistrationParameters.xml is empty/does not exist. There are no presets to select.");
    return;
  }
  presets.sort();
  // ask about the name to load a preset
  QmitkLoadPresetDialog dialog( this, 0, "Load Preset", presets ); // needs a QWidget as parent
  int dialogReturnValue = dialog.exec();
  if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

  this->DoLoadRigidRegistrationPreset(dialog.GetPresetName(), testPreset);
}

void QmitkRigidRegistrationSelectorView::DoLoadRigidRegistrationPreset(std::string presetName, bool testPreset)
{
  itk::Array<double> transformValues;
  if (testPreset)
  {
    transformValues = m_TestPreset->getTransformValues(presetName);
  }
  else
  {
    transformValues = m_Preset->getTransformValues(presetName);
  }
  m_Controls.m_TransformGroup->setChecked(true);
  m_Controls.m_TransformFrame->setVisible(true);
  m_Controls.m_TransformBox->setCurrentIndex((int)transformValues[0]);
  m_Controls.m_TransformWidgetStack->setCurrentIndex((int)transformValues[0]);
  this->TransformSelected((int)transformValues[0]);
  if (transformValues[0] == mitk::TransformParameters::TRANSLATIONTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesTranslation->setChecked(transformValues[1]);
    m_Controls.m_ScalesTranslationTransformTranslationX->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesTranslationTransformTranslationY->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesTranslationTransformTranslationZ->setText(QString::number(transformValues[4]));
  }
  else if (transformValues[0] == mitk::TransformParameters::SCALETRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesScale->setChecked(transformValues[1]);
    m_Controls.m_ScalesScaleTransformScaleX->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesScaleTransformScaleY->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesScaleTransformScaleZ->setText(QString::number(transformValues[4]));
  }
  else if (transformValues[0] == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesScaleLogarithmic->setChecked(transformValues[1]);
    m_Controls.m_ScalesScaleLogarithmicTransformScaleX->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesScaleLogarithmicTransformScaleY->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->setText(QString::number(transformValues[4]));
  }
  else if (transformValues[0] == mitk::TransformParameters::AFFINETRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesAffine->setChecked(transformValues[1]);
    m_Controls.m_ScalesAffineTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesAffineTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesAffineTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesAffineTransformScale4->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesAffineTransformScale5->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesAffineTransformScale6->setText(QString::number(transformValues[7]));
    m_Controls.m_ScalesAffineTransformScale7->setText(QString::number(transformValues[8]));
    m_Controls.m_ScalesAffineTransformScale8->setText(QString::number(transformValues[9]));
    m_Controls.m_ScalesAffineTransformScale9->setText(QString::number(transformValues[10]));
    m_Controls.m_ScalesAffineTransformScaleTranslationX->setText(QString::number(transformValues[11]));
    m_Controls.m_ScalesAffineTransformScaleTranslationY->setText(QString::number(transformValues[12]));
    m_Controls.m_ScalesAffineTransformScaleTranslationZ->setText(QString::number(transformValues[13]));
    m_Controls.m_CenterForInitializerAffine->setChecked(transformValues[14]);
    m_Controls.m_MomentsAffine->setChecked(transformValues[15]);
    m_Controls.m_GeometryAffine->setChecked(!transformValues[15]);
  }
  else if (transformValues[0] == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->setChecked(transformValues[1]);
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->setText(QString::number(transformValues[7]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->setText(QString::number(transformValues[8]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->setText(QString::number(transformValues[9]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->setText(QString::number(transformValues[10]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->setText(QString::number(transformValues[11]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->setText(QString::number(transformValues[12]));
    m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->setText(QString::number(transformValues[13]));
    m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->setChecked(transformValues[14]);
    m_Controls.m_MomentsFixedCenterOfRotationAffine->setChecked(transformValues[15]);
    m_Controls.m_GeometryFixedCenterOfRotationAffine->setChecked(!transformValues[15]);
  }
  else if (transformValues[0] == mitk::TransformParameters::RIGID3DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesRigid3D->setChecked(transformValues[1]);
    m_Controls.m_ScalesRigid3DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesRigid3DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesRigid3DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesRigid3DTransformScale4->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesRigid3DTransformScale5->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesRigid3DTransformScale6->setText(QString::number(transformValues[7]));
    m_Controls.m_ScalesRigid3DTransformScale7->setText(QString::number(transformValues[8]));
    m_Controls.m_ScalesRigid3DTransformScale8->setText(QString::number(transformValues[9]));
    m_Controls.m_ScalesRigid3DTransformScale9->setText(QString::number(transformValues[10]));
    m_Controls.m_ScalesRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[11]));
    m_Controls.m_ScalesRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[12]));
    m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[13]));
    m_Controls.m_CenterForInitializerRigid3D->setChecked(transformValues[14]);
    m_Controls.m_MomentsRigid3D->setChecked(transformValues[15]);
    m_Controls.m_GeometryRigid3D->setChecked(!transformValues[15]);
  }
  else if (transformValues[0] == mitk::TransformParameters::EULER3DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesEuler3D->setChecked(transformValues[1]);
    m_Controls.m_ScalesEuler3DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesEuler3DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesEuler3DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesEuler3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesEuler3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_Controls.m_CenterForInitializerEuler3D->setChecked(transformValues[8]);
    m_Controls.m_MomentsEuler3D->setChecked(transformValues[9]);
    m_Controls.m_GeometryEuler3D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesCenteredEuler3D->setChecked(transformValues[1]);
    m_Controls.m_ScalesCenteredEuler3DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesCenteredEuler3DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesCenteredEuler3DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_Controls.m_CenterForInitializerCenteredEuler3D->setChecked(transformValues[8]);
    m_Controls.m_MomentsCenteredEuler3D->setChecked(transformValues[9]);
    m_Controls.m_GeometryCenteredEuler3D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesQuaternionRigid->setChecked(transformValues[1]);
    m_Controls.m_ScalesQuaternionRigidTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesQuaternionRigidTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesQuaternionRigidTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesQuaternionRigidTransformScale4->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->setText(QString::number(transformValues[7]));
    m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->setText(QString::number(transformValues[8]));
    m_Controls.m_CenterForInitializerQuaternionRigid->setChecked(transformValues[9]);
    m_Controls.m_MomentsQuaternionRigid->setChecked(transformValues[10]);
    m_Controls.m_GeometryQuaternionRigid->setChecked(!transformValues[10]);
  }
  else if (transformValues[0] == mitk::TransformParameters::VERSORTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesVersor->setChecked(transformValues[1]);
    m_Controls.m_ScalesVersorTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesVersorTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesVersorTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_CenterForInitializerVersor->setChecked(transformValues[5]);
    m_Controls.m_MomentsVersor->setChecked(transformValues[6]);
    m_Controls.m_GeometryVersor->setChecked(!transformValues[6]);
  }
  else if (transformValues[0] == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesVersorRigid3D->setChecked(transformValues[1]);
    m_Controls.m_ScalesVersorRigid3DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesVersorRigid3DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesVersorRigid3DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_Controls.m_CenterForInitializerVersorRigid3D->setChecked(transformValues[8]);
    m_Controls.m_MomentsVersorRigid3D->setChecked(transformValues[9]);
    m_Controls.m_GeometryVersorRigid3D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->setChecked(transformValues[1]);
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
    m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->setText(QString::number(transformValues[8]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->setText(QString::number(transformValues[9]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->setText(QString::number(transformValues[10]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->setText(QString::number(transformValues[11]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->setText(QString::number(transformValues[12]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->setText(QString::number(transformValues[13]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->setText(QString::number(transformValues[14]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->setText(QString::number(transformValues[15]));
    m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->setText(QString::number(transformValues[16]));
    m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->setChecked(transformValues[17]);
    m_Controls.m_MomentsScaleSkewVersorRigid3D->setChecked(transformValues[18]);
    m_Controls.m_GeometryScaleSkewVersorRigid3D->setChecked(!transformValues[18]);
  }
  else if (transformValues[0] == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesSimilarity3D->setChecked(transformValues[1]);
    m_Controls.m_ScalesSimilarity3DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesSimilarity3DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesSimilarity3DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesSimilarity3DTransformScale4->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->setText(QString::number(transformValues[7]));
    m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->setText(QString::number(transformValues[8]));
    m_Controls.m_CenterForInitializerSimilarity3D->setChecked(transformValues[9]);
    m_Controls.m_MomentsSimilarity3D->setChecked(transformValues[10]);
    m_Controls.m_GeometrySimilarity3D->setChecked(!transformValues[10]);
  }
  else if (transformValues[0] == mitk::TransformParameters::RIGID2DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesRigid2D->setChecked(transformValues[1]);
    m_Controls.m_ScalesRigid2DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesRigid2DTransformScaleTranslationX->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesRigid2DTransformScaleTranslationY->setText(QString::number(transformValues[4]));
    m_Controls.m_CenterForInitializerRigid2D->setChecked(transformValues[5]);
    m_Controls.m_MomentsRigid2D->setChecked(transformValues[6]);
    m_Controls.m_GeometryRigid2D->setChecked(!transformValues[6]);
  }
  else if (transformValues[0] == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesCenteredRigid2D->setChecked(transformValues[1]);
    m_Controls.m_RotationScaleCenteredRigid2D->setText(QString::number(transformValues[2]));
    m_Controls.m_CenterXScaleCenteredRigid2D->setText(QString::number(transformValues[3]));
    m_Controls.m_CenterYScaleCenteredRigid2D->setText(QString::number(transformValues[4]));
    m_Controls.m_TranslationXScaleCenteredRigid2D->setText(QString::number(transformValues[5]));
    m_Controls.m_TranslationYScaleCenteredRigid2D->setText(QString::number(transformValues[6]));
    m_Controls.m_AngleCenteredRigid2D->setText(QString::number(transformValues[7]));
    m_Controls.m_CenterForInitializerCenteredRigid2D->setChecked(transformValues[8]);
    m_Controls.m_MomentsCenteredRigid2D->setChecked(transformValues[9]);
    m_Controls.m_GeometryCenteredRigid2D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::EULER2DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesEuler2D->setChecked(transformValues[1]);
    m_Controls.m_RotationScaleEuler2D->setText(QString::number(transformValues[2]));
    m_Controls.m_TranslationXScaleEuler2D->setText(QString::number(transformValues[3]));
    m_Controls.m_TranslationYScaleEuler2D->setText(QString::number(transformValues[4]));
    m_Controls.m_CenterForInitializerEuler2D->setChecked(transformValues[5]);
    m_Controls.m_MomentsEuler2D->setChecked(transformValues[6]);
    m_Controls.m_GeometryEuler2D->setChecked(!transformValues[6]);
  }
  else if (transformValues[0] == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesSimilarity2D->setChecked(transformValues[1]);
    m_Controls.m_ScalingScaleSimilarity2D->setText(QString::number(transformValues[2]));
    m_Controls.m_RotationScaleSimilarity2D->setText(QString::number(transformValues[3]));
    m_Controls.m_TranslationXScaleSimilarity2D->setText(QString::number(transformValues[4]));
    m_Controls.m_TranslationYScaleSimilarity2D->setText(QString::number(transformValues[5]));
    m_Controls.m_InitialScaleSimilarity2D->setText(QString::number(transformValues[6]));
    m_Controls.m_AngleSimilarity2D->setText(QString::number(transformValues[7]));
    m_Controls.m_CenterForInitializerSimilarity2D->setChecked(transformValues[8]);
    m_Controls.m_MomentsSimilarity2D->setChecked(transformValues[9]);
    m_Controls.m_GeometrySimilarity2D->setChecked(!transformValues[9]);
  }
  else if (transformValues[0] == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
  {
    m_Controls.m_UseOptimizerScalesCenteredSimilarity2D->setChecked(transformValues[1]);
    m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->setText(QString::number(transformValues[2]));
    m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->setText(QString::number(transformValues[3]));
    m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->setText(QString::number(transformValues[4]));
    m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->setText(QString::number(transformValues[5]));
    m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->setText(QString::number(transformValues[6]));
    m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->setText(QString::number(transformValues[7]));
    m_Controls.m_InitialScaleCenteredSimilarity2D->setText(QString::number(transformValues[8]));
    m_Controls.m_AngleCenteredSimilarity2D->setText(QString::number(transformValues[9]));
    m_Controls.m_CenterForInitializerCenteredSimilarity2D->setChecked(transformValues[10]);
    m_Controls.m_MomentsCenteredSimilarity2D->setChecked(transformValues[11]);
    m_Controls.m_GeometryCenteredSimilarity2D->setChecked(!transformValues[11]);
  }

  itk::Array<double> metricValues;
  if (testPreset)
  {
    metricValues = m_TestPreset->getMetricValues(presetName);
  }
  else
  {
    metricValues = m_Preset->getMetricValues(presetName);
  }
  m_Controls.m_MetricGroup->setChecked(true);
  m_Controls.m_MetricFrame->setVisible(true);
  m_Controls.m_MetricBox->setCurrentIndex((int)metricValues[0]);
  m_Controls.m_MetricWidgetStack->setCurrentIndex((int)metricValues[0]);
  this->MetricSelected((int)metricValues[0]);
  m_Controls.m_ComputeGradient->setChecked(metricValues[1]);
  if (metricValues[0] == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NumberOfHistogramBinsCorrelationCoefficientHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_UseSamplingMattesMutualInformation->setChecked(metricValues[2]);
    m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->setText(QString::number(metricValues[3]));
    m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->setText(QString::number(metricValues[4]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_LambdaMeanReciprocalSquareDifference->setText(QString::number(metricValues[2]));
  }
  else if (metricValues[0] == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
  {
    m_Controls.m_NumberOfSpatialSamplesMutualInformation->setText(QString::number(metricValues[2]));
    m_Controls.m_FixedImageStandardDeviationMutualInformation->setText(QString::number(metricValues[3]));
    m_Controls.m_MovingImageStandardDeviationMutualInformation->setText(QString::number(metricValues[4]));
    m_Controls.m_UseNormalizerAndSmoother->setChecked(metricValues[5]);
    m_Controls.m_FixedSmootherVarianceMutualInformation->setText(QString::number(metricValues[6]));
    m_Controls.m_MovingSmootherVarianceMutualInformation->setText(QString::number(metricValues[7]));
  }

  itk::Array<double> optimizerValues;
  if (testPreset)
  {
    optimizerValues = m_TestPreset->getOptimizerValues(presetName);
  }
  else
  {
    optimizerValues = m_Preset->getOptimizerValues(presetName);
  }
  m_Controls.m_OptimizerGroup->setChecked(true);
  m_Controls.m_OptimizerFrame->setVisible(true);
  m_Controls.m_OptimizerBox->setCurrentIndex((int)optimizerValues[0]);
  m_Controls.m_OptimizerWidgetStack->setCurrentIndex((int)optimizerValues[0]);
  this->OptimizerSelected((int)optimizerValues[0]);
  m_Controls.m_Maximize->setChecked(optimizerValues[1]);
  if (optimizerValues[0] == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
  {
    m_Controls.m_StepLengthExhaustive->setText(QString::number(optimizerValues[2]));
    m_Controls.m_NumberOfStepsExhaustive->setText(QString::number(optimizerValues[3]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
  {
    m_Controls.m_LearningRateGradientDescent->setText(QString::number(optimizerValues[2]));
    m_Controls.m_IterationsGradientDescent->setText(QString::number(optimizerValues[3]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
  {
    m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->setText(QString::number(optimizerValues[2]));
    m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->setText(QString::number(optimizerValues[3]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
  {
    m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->setText(QString::number(optimizerValues[2]));
    m_Controls.m_GrowthFactorOnePlusOneEvolutionary->setText(QString::number(optimizerValues[3]));
    m_Controls.m_EpsilonOnePlusOneEvolutionary->setText(QString::number(optimizerValues[4]));
    m_Controls.m_InitialRadiusOnePlusOneEvolutionary->setText(QString::number(optimizerValues[5]));
    m_Controls.m_IterationsOnePlusOneEvolutionary->setText(QString::number(optimizerValues[6]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::POWELLOPTIMIZER)
  {
    m_Controls.m_StepLengthPowell->setText(QString::number(optimizerValues[2]));
    m_Controls.m_StepTolerancePowell->setText(QString::number(optimizerValues[3]));
    m_Controls.m_ValueTolerancePowell->setText(QString::number(optimizerValues[4]));
    m_Controls.m_IterationsPowell->setText(QString::number(optimizerValues[5]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::FRPROPTIMIZER)
  {
    m_Controls.m_FletchReevesFRPR->setChecked(optimizerValues[2]);
    m_Controls.m_PolakRibiereFRPR->setChecked(!optimizerValues[2]);
    m_Controls.m_StepLengthFRPR->setText(QString::number(optimizerValues[3]));
    m_Controls.m_IterationsFRPR->setText(QString::number(optimizerValues[4]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
  {
    m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->setText(QString::number(optimizerValues[2]));
    m_Controls.m_MinimumStepLengthRegularStepGradientDescent->setText(QString::number(optimizerValues[3]));
    m_Controls.m_MaximumStepLengthRegularStepGradientDescent->setText(QString::number(optimizerValues[4]));
    m_Controls.m_RelaxationFactorRegularStepGradientDescent->setText(QString::number(optimizerValues[5]));
    m_Controls.m_IterationsRegularStepGradientDescent->setText(QString::number(optimizerValues[6]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
  {
    m_Controls.m_GradientMagnitudeToleranceVersorTransform->setText(QString::number(optimizerValues[2]));
    m_Controls.m_MinimumStepLengthVersorTransform->setText(QString::number(optimizerValues[3]));
    m_Controls.m_MaximumStepLengthVersorTransform->setText(QString::number(optimizerValues[4]));
    m_Controls.m_IterationsVersorTransform->setText(QString::number(optimizerValues[5]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
  {
    m_Controls.m_SimplexDeltaAmoeba1->setText(QString::number(optimizerValues[2]));
    m_Controls.m_SimplexDeltaAmoeba2->setText(QString::number(optimizerValues[3]));
    m_Controls.m_SimplexDeltaAmoeba3->setText(QString::number(optimizerValues[4]));
    m_Controls.m_SimplexDeltaAmoeba4->setText(QString::number(optimizerValues[5]));
    m_Controls.m_SimplexDeltaAmoeba5->setText(QString::number(optimizerValues[6]));
    m_Controls.m_SimplexDeltaAmoeba6->setText(QString::number(optimizerValues[7]));
    m_Controls.m_SimplexDeltaAmoeba7->setText(QString::number(optimizerValues[8]));
    m_Controls.m_SimplexDeltaAmoeba8->setText(QString::number(optimizerValues[9]));
    m_Controls.m_SimplexDeltaAmoeba9->setText(QString::number(optimizerValues[10]));
    m_Controls.m_SimplexDeltaAmoeba10->setText(QString::number(optimizerValues[11]));
    m_Controls.m_SimplexDeltaAmoeba11->setText(QString::number(optimizerValues[12]));
    m_Controls.m_SimplexDeltaAmoeba12->setText(QString::number(optimizerValues[13]));
    m_Controls.m_SimplexDeltaAmoeba13->setText(QString::number(optimizerValues[14]));
    m_Controls.m_SimplexDeltaAmoeba14->setText(QString::number(optimizerValues[15]));
    m_Controls.m_SimplexDeltaAmoeba15->setText(QString::number(optimizerValues[16]));
    m_Controls.m_ParametersConvergenceToleranceAmoeba->setText(QString::number(optimizerValues[17]));
    m_Controls.m_FunctionConvergenceToleranceAmoeba->setText(QString::number(optimizerValues[18]));
    m_Controls.m_IterationsAmoeba->setText(QString::number(optimizerValues[19]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::LBFGSOPTIMIZER)
  {
    m_Controls.m_GradientMagnitudeToleranceLBFGS->setText(QString::number(optimizerValues[2]));
    m_Controls.m_LineSearchAccuracyLBFGS->setText(QString::number(optimizerValues[3]));
    m_Controls.m_DefaultStepLengthLBFGS->setText(QString::number(optimizerValues[4]));
    m_Controls.m_MaximumEvaluationsLBFGS->setText(QString::number(optimizerValues[5]));
    m_Controls.m_TraceOnLBFGS->setChecked(optimizerValues[6]);
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::SPSAOPTIMIZER)
  {
    m_Controls.m_aSPSA->setText(QString::number(optimizerValues[2]));
    m_Controls.m_ASPSA->setText(QString::number(optimizerValues[3]));
    m_Controls.m_AlphaSPSA->setText(QString::number(optimizerValues[4]));
    m_Controls.m_cSPSA->setText(QString::number(optimizerValues[5]));
    m_Controls.m_GammaSPSA->setText(QString::number(optimizerValues[6]));
    m_Controls.m_ToleranceSPSA->setText(QString::number(optimizerValues[7]));
    m_Controls.m_StateOfConvergenceDecayRateSPSA->setText(QString::number(optimizerValues[8]));
    m_Controls.m_MinimumNumberOfIterationsSPSA->setText(QString::number(optimizerValues[9]));
    m_Controls.m_NumberOfPerturbationsSPSA->setText(QString::number(optimizerValues[10]));
    m_Controls.m_IterationsSPSA->setText(QString::number(optimizerValues[11]));
  }
  else if (optimizerValues[0] == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
  {
    /*m_Controls.m_GradientConvergenceToleranceLBFGSB->setText(QString::number(optimizerValues[2]));
    m_Controls.m_MinimumStepLengthVersorRigid3DTransform->setText(QString::number(optimizerValues[3]));
    m_Controls.m_MaximumStepLengthVersorRigid3DTransform->setText(QString::number(optimizerValues[4]));
    m_Controls.m_IterationsVersorRigid3DTransform->setText(QString::number(optimizerValues[5]));*/
  }

  itk::Array<double> interpolatorValues;
  if (testPreset)
  {
    interpolatorValues = m_TestPreset->getInterpolatorValues(presetName);
  }
  else
  {
    interpolatorValues = m_Preset->getInterpolatorValues(presetName);
  }
  m_Controls.m_InterpolatorGroup->setChecked(true);
  m_Controls.m_InterpolatorFrame->setVisible(true);
  m_Controls.m_InterpolatorBox->setCurrentIndex((int)interpolatorValues[0]);
}

void QmitkRigidRegistrationSelectorView::SaveRigidRegistrationParameter()
{
  this->DoSaveRigidRegistrationParameter(false);
}

void QmitkRigidRegistrationSelectorView::SaveRigidRegistrationTestParameter()
{
  this->DoSaveRigidRegistrationParameter(true);
}

void QmitkRigidRegistrationSelectorView::DoSaveRigidRegistrationParameter(bool testPreset)
{
  bool ok;
  QString text = QInputDialog::getText(this, 
    "Save Parameter Preset", "Enter name for preset:", QLineEdit::Normal,
    QString::null, &ok );
  if ( ok )
  {
    std::map<std::string, itk::Array<double> > existingPresets;
    if (testPreset)
    {
      existingPresets = m_TestPreset->getTransformValuesPresets();
    }
    else
    {
      existingPresets = m_Preset->getTransformValuesPresets();
    }
    std::map<std::string, itk::Array<double> >::iterator iter = existingPresets.find(std::string((const char*)text.toLatin1()));
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
    transformValues[0] = m_Controls.m_TransformBox->currentIndex();
    if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::TRANSLATIONTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesTranslation->isChecked();
      transformValues[2] = m_Controls.m_ScalesTranslationTransformTranslationX->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesTranslationTransformTranslationY->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesTranslationTransformTranslationZ->text().toDouble();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALETRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesScale->isChecked();
      transformValues[2] = m_Controls.m_ScalesScaleTransformScaleX->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesScaleTransformScaleY->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesScaleTransformScaleZ->text().toDouble();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesScaleLogarithmic->isChecked();
      transformValues[2] = m_Controls.m_ScalesScaleLogarithmicTransformScaleX->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesScaleLogarithmicTransformScaleY->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->text().toDouble();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::AFFINETRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesAffine->isChecked();
      transformValues[2] = m_Controls.m_ScalesAffineTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesAffineTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesAffineTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesAffineTransformScale4->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesAffineTransformScale5->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesAffineTransformScale6->text().toDouble();
      transformValues[8] = m_Controls.m_ScalesAffineTransformScale7->text().toDouble();
      transformValues[9] = m_Controls.m_ScalesAffineTransformScale8->text().toDouble();
      transformValues[10] = m_Controls.m_ScalesAffineTransformScale9->text().toDouble();
      transformValues[11] = m_Controls.m_ScalesAffineTransformScaleTranslationX->text().toDouble();
      transformValues[12] = m_Controls.m_ScalesAffineTransformScaleTranslationY->text().toDouble();
      transformValues[13] = m_Controls.m_ScalesAffineTransformScaleTranslationZ->text().toDouble();
      transformValues[14] = m_Controls.m_CenterForInitializerAffine->isChecked();
      transformValues[15] = m_Controls.m_MomentsAffine->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked();
      transformValues[2] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->text().toDouble();
      transformValues[8] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->text().toDouble();
      transformValues[9] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->text().toDouble();
      transformValues[10] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->text().toDouble();
      transformValues[11] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->text().toDouble();
      transformValues[12] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->text().toDouble();
      transformValues[13] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->text().toDouble();
      transformValues[14] = m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->isChecked();
      transformValues[15] = m_Controls.m_MomentsFixedCenterOfRotationAffine->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::RIGID3DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesRigid3D->isChecked();
      transformValues[2] = m_Controls.m_ScalesRigid3DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesRigid3DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesRigid3DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesRigid3DTransformScale4->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesRigid3DTransformScale5->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesRigid3DTransformScale6->text().toDouble();
      transformValues[8] = m_Controls.m_ScalesRigid3DTransformScale7->text().toDouble();
      transformValues[9] = m_Controls.m_ScalesRigid3DTransformScale8->text().toDouble();
      transformValues[10] = m_Controls.m_ScalesRigid3DTransformScale9->text().toDouble();
      transformValues[11] = m_Controls.m_ScalesRigid3DTransformScaleTranslationX->text().toDouble();
      transformValues[12] = m_Controls.m_ScalesRigid3DTransformScaleTranslationY->text().toDouble();
      transformValues[13] = m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->text().toDouble();
      transformValues[14] = m_Controls.m_CenterForInitializerRigid3D->isChecked();
      transformValues[15] = m_Controls.m_MomentsRigid3D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::EULER3DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesEuler3D->isChecked();
      transformValues[2] = m_Controls.m_ScalesEuler3DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesEuler3DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesEuler3DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesEuler3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesEuler3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_Controls.m_CenterForInitializerEuler3D->isChecked();
      transformValues[9] = m_Controls.m_MomentsEuler3D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesCenteredEuler3D->isChecked();
      transformValues[2] = m_Controls.m_ScalesCenteredEuler3DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesCenteredEuler3DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesCenteredEuler3DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_Controls.m_CenterForInitializerCenteredEuler3D->isChecked();
      transformValues[9] = m_Controls.m_MomentsCenteredEuler3D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesQuaternionRigid->isChecked();
      transformValues[2] = m_Controls.m_ScalesQuaternionRigidTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesQuaternionRigidTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesQuaternionRigidTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesQuaternionRigidTransformScale4->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->text().toDouble();
      transformValues[8] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->text().toDouble();
      transformValues[9] = m_Controls.m_CenterForInitializerQuaternionRigid->isChecked();
      transformValues[10] = m_Controls.m_MomentsQuaternionRigid->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::VERSORTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesVersor->isChecked();
      transformValues[2] = m_Controls.m_ScalesVersorTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesVersorTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesVersorTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_CenterForInitializerVersor->isChecked();
      transformValues[6] = m_Controls.m_MomentsVersor->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesVersorRigid3D->isChecked();
      transformValues[2] = m_Controls.m_ScalesVersorRigid3DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesVersorRigid3DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesVersorRigid3DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_Controls.m_CenterForInitializerVersorRigid3D->isChecked();
      transformValues[9] = m_Controls.m_MomentsVersorRigid3D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked();
      transformValues[2] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->text().toDouble();
      transformValues[6] = m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->text().toDouble();
      transformValues[8] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->text().toDouble();
      transformValues[9] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->text().toDouble();
      transformValues[10] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->text().toDouble();
      transformValues[11] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->text().toDouble();
      transformValues[12] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->text().toDouble();
      transformValues[13] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->text().toDouble();
      transformValues[14] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->text().toDouble();
      transformValues[15] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->text().toDouble();
      transformValues[16] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->text().toDouble();
      transformValues[17] = m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->isChecked();
      transformValues[18] = m_Controls.m_MomentsScaleSkewVersorRigid3D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesSimilarity3D->isChecked();
      transformValues[2] = m_Controls.m_ScalesSimilarity3DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesSimilarity3DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesSimilarity3DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesSimilarity3DTransformScale4->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->text().toDouble();
      transformValues[8] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->text().toDouble();
      transformValues[9] = m_Controls.m_CenterForInitializerSimilarity3D->isChecked();
      transformValues[10] = m_Controls.m_MomentsSimilarity3D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::RIGID2DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesRigid2D->isChecked();
      transformValues[2] = m_Controls.m_ScalesRigid2DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesRigid2DTransformScaleTranslationX->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesRigid2DTransformScaleTranslationY->text().toDouble();
      transformValues[5] = m_Controls.m_CenterForInitializerRigid2D->isChecked();
      transformValues[6] = m_Controls.m_MomentsRigid2D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesCenteredRigid2D->isChecked();
      transformValues[2] = m_Controls.m_RotationScaleCenteredRigid2D->text().toDouble();
      transformValues[3] = m_Controls.m_CenterXScaleCenteredRigid2D->text().toDouble();
      transformValues[4] = m_Controls.m_CenterYScaleCenteredRigid2D->text().toDouble();
      transformValues[5] = m_Controls.m_TranslationXScaleCenteredRigid2D->text().toDouble();
      transformValues[6] = m_Controls.m_TranslationYScaleCenteredRigid2D->text().toDouble();
      transformValues[7] = m_Controls.m_AngleCenteredRigid2D->text().toFloat();
      transformValues[8] = m_Controls.m_CenterForInitializerCenteredRigid2D->isChecked();
      transformValues[9] = m_Controls.m_MomentsCenteredRigid2D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::EULER2DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesEuler2D->isChecked();
      transformValues[2] = m_Controls.m_RotationScaleEuler2D->text().toDouble();
      transformValues[3] = m_Controls.m_TranslationXScaleEuler2D->text().toDouble();
      transformValues[4] = m_Controls.m_TranslationYScaleEuler2D->text().toDouble();
      transformValues[5] = m_Controls.m_CenterForInitializerEuler2D->isChecked();
      transformValues[6] = m_Controls.m_MomentsEuler2D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesSimilarity2D->isChecked();
      transformValues[2] = m_Controls.m_ScalingScaleSimilarity2D->text().toDouble();
      transformValues[3] = m_Controls.m_RotationScaleSimilarity2D->text().toDouble();
      transformValues[4] = m_Controls.m_TranslationXScaleSimilarity2D->text().toDouble();
      transformValues[5] = m_Controls.m_TranslationYScaleSimilarity2D->text().toDouble();
      transformValues[6] = m_Controls.m_InitialScaleSimilarity2D->text().toFloat();
      transformValues[7] = m_Controls.m_AngleSimilarity2D->text().toFloat();
      transformValues[8] = m_Controls.m_CenterForInitializerSimilarity2D->isChecked();
      transformValues[9] = m_Controls.m_MomentsSimilarity2D->isChecked();
    }
    else if (m_Controls.m_TransformBox->currentIndex() == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      transformValues[1] = m_Controls.m_UseOptimizerScalesCenteredSimilarity2D->isChecked();
      transformValues[2] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->text().toDouble();
      transformValues[3] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->text().toDouble();
      transformValues[4] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->text().toDouble();
      transformValues[5] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->text().toDouble();
      transformValues[6] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->text().toDouble();
      transformValues[7] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->text().toDouble();
      transformValues[8] = m_Controls.m_InitialScaleCenteredSimilarity2D->text().toFloat();
      transformValues[9] = m_Controls.m_AngleCenteredSimilarity2D->text().toFloat();
      transformValues[10] = m_Controls.m_CenterForInitializerCenteredSimilarity2D->isChecked();
      transformValues[11] = m_Controls.m_MomentsCenteredSimilarity2D->isChecked();
    }
    std::map<std::string, itk::Array<double> > transformMap;
    if (testPreset)
    {
      transformMap = m_TestPreset->getTransformValuesPresets();
    }
    else
    {
      transformMap = m_Preset->getTransformValuesPresets();
    }
    transformMap[std::string((const char*)text.toLatin1())] = transformValues;

    itk::Array<double> metricValues;
    metricValues.SetSize(25);
    metricValues.fill(0);
    metricValues[0] = m_Controls.m_MetricBox->currentIndex();
    metricValues[1] = m_Controls.m_ComputeGradient->isChecked();
    if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_NumberOfHistogramBinsCorrelationCoefficientHistogram->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_UseSamplingMattesMutualInformation->isChecked();
      metricValues[3] = m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->text().toInt();
      metricValues[4] = m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_LambdaMeanReciprocalSquareDifference->text().toInt();
    }
    else if (m_Controls.m_MetricBox->currentIndex() == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      metricValues[2] = m_Controls.m_NumberOfSpatialSamplesMutualInformation->text().toInt();
      metricValues[3] = m_Controls.m_FixedImageStandardDeviationMutualInformation->text().toFloat();
      metricValues[4] = m_Controls.m_MovingImageStandardDeviationMutualInformation->text().toFloat();
      metricValues[5] = m_Controls.m_UseNormalizerAndSmoother->isChecked();
      metricValues[6] = m_Controls.m_FixedSmootherVarianceMutualInformation->text().toFloat();
      metricValues[7] = m_Controls.m_MovingSmootherVarianceMutualInformation->text().toFloat();
    }

    std::map<std::string, itk::Array<double> > metricMap;
    if (testPreset)
    {
      metricMap = m_TestPreset->getMetricValuesPresets();
    }
    else
    {
      metricMap = m_Preset->getMetricValuesPresets();
    }
    metricMap[std::string((const char*)text.toLatin1())] = metricValues;

    itk::Array<double> optimizerValues;
    optimizerValues.SetSize(25);
    optimizerValues.fill(0);
    optimizerValues[0] = m_Controls.m_OptimizerBox->currentIndex();
    optimizerValues[1] = m_Controls.m_Maximize->isChecked();
    if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_StepLengthExhaustive->text().toFloat();
      optimizerValues[3] = m_Controls.m_NumberOfStepsExhaustive->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_LearningRateGradientDescent->text().toFloat();
      optimizerValues[3] = m_Controls.m_IterationsGradientDescent->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->text().toFloat();
      optimizerValues[3] = m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[3] = m_Controls.m_GrowthFactorOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[4] = m_Controls.m_EpsilonOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[5] = m_Controls.m_InitialRadiusOnePlusOneEvolutionary->text().toFloat();
      optimizerValues[6] = m_Controls.m_IterationsOnePlusOneEvolutionary->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::POWELLOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_StepLengthPowell->text().toFloat();
      optimizerValues[3] = m_Controls.m_StepTolerancePowell->text().toFloat();
      optimizerValues[4] = m_Controls.m_ValueTolerancePowell->text().toFloat();
      optimizerValues[5] = m_Controls.m_IterationsPowell->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::FRPROPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_FletchReevesFRPR->isChecked();
      optimizerValues[3] = m_Controls.m_StepLengthFRPR->text().toFloat();
      optimizerValues[4] = m_Controls.m_IterationsFRPR->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->text().toFloat();
      optimizerValues[3] = m_Controls.m_MinimumStepLengthRegularStepGradientDescent->text().toFloat();
      optimizerValues[4] = m_Controls.m_MaximumStepLengthRegularStepGradientDescent->text().toFloat();
      optimizerValues[5] = m_Controls.m_RelaxationFactorRegularStepGradientDescent->text().toFloat();
      optimizerValues[6] = m_Controls.m_IterationsRegularStepGradientDescent->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_GradientMagnitudeToleranceVersorTransform->text().toFloat();
      optimizerValues[3] = m_Controls.m_MinimumStepLengthVersorTransform->text().toFloat();
      optimizerValues[4] = m_Controls.m_MaximumStepLengthVersorTransform->text().toFloat();
      optimizerValues[5] = m_Controls.m_IterationsVersorTransform->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_SimplexDeltaAmoeba1->text().toDouble();
      optimizerValues[3] = m_Controls.m_SimplexDeltaAmoeba2->text().toDouble();
      optimizerValues[4] = m_Controls.m_SimplexDeltaAmoeba3->text().toDouble();
      optimizerValues[5] = m_Controls.m_SimplexDeltaAmoeba4->text().toDouble();
      optimizerValues[6] = m_Controls.m_SimplexDeltaAmoeba5->text().toDouble();
      optimizerValues[7] = m_Controls.m_SimplexDeltaAmoeba6->text().toDouble();
      optimizerValues[8] = m_Controls.m_SimplexDeltaAmoeba7->text().toDouble();
      optimizerValues[9] = m_Controls.m_SimplexDeltaAmoeba8->text().toDouble();
      optimizerValues[10] = m_Controls.m_SimplexDeltaAmoeba9->text().toDouble();
      optimizerValues[11] = m_Controls.m_SimplexDeltaAmoeba10->text().toDouble();
      optimizerValues[12] = m_Controls.m_SimplexDeltaAmoeba11->text().toDouble();
      optimizerValues[13] = m_Controls.m_SimplexDeltaAmoeba12->text().toDouble();
      optimizerValues[14] = m_Controls.m_SimplexDeltaAmoeba13->text().toDouble();
      optimizerValues[15] = m_Controls.m_SimplexDeltaAmoeba14->text().toDouble();
      optimizerValues[16] = m_Controls.m_SimplexDeltaAmoeba15->text().toDouble();
      optimizerValues[17] = m_Controls.m_ParametersConvergenceToleranceAmoeba->text().toFloat();
      optimizerValues[18] = m_Controls.m_FunctionConvergenceToleranceAmoeba->text().toFloat();
      optimizerValues[19] = m_Controls.m_IterationsAmoeba->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::LBFGSOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_GradientMagnitudeToleranceLBFGS->text().toFloat();
      optimizerValues[3] = m_Controls.m_LineSearchAccuracyLBFGS->text().toFloat();
      optimizerValues[4] = m_Controls.m_DefaultStepLengthLBFGS->text().toFloat();
      optimizerValues[5] = m_Controls.m_MaximumEvaluationsLBFGS->text().toInt();
      optimizerValues[6] = m_Controls.m_TraceOnLBFGS->isChecked();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::SPSAOPTIMIZER)
    {
      optimizerValues[2] = m_Controls.m_aSPSA->text().toFloat();
      optimizerValues[3] = m_Controls.m_ASPSA->text().toFloat();
      optimizerValues[4] = m_Controls.m_AlphaSPSA->text().toFloat();
      optimizerValues[5] = m_Controls.m_cSPSA->text().toFloat();
      optimizerValues[6] = m_Controls.m_GammaSPSA->text().toFloat();
      optimizerValues[7] = m_Controls.m_ToleranceSPSA->text().toFloat();
      optimizerValues[8] = m_Controls.m_StateOfConvergenceDecayRateSPSA->text().toFloat();
      optimizerValues[9] = m_Controls.m_MinimumNumberOfIterationsSPSA->text().toInt();
      optimizerValues[10] = m_Controls.m_NumberOfPerturbationsSPSA->text().toInt();
      optimizerValues[11] = m_Controls.m_IterationsSPSA->text().toInt();
    }
    else if (m_Controls.m_OptimizerBox->currentIndex() == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
    {
      /* optimizerValues[2] = m_Controls.m_GradientConvergenceToleranceLBFGSB->text().toFloat();
      optimizerValues[3] = m_Controls.m_MinimumStepLengthVersorRigid3DTransform->text().toFloat();
      optimizerValues[4] = m_Controls.m_MaximumStepLengthVersorRigid3DTransform->text().toFloat();
      optimizerValues[5] = m_Controls.m_IterationsVersorRigid3DTransform->text().toInt();*/
    }

    std::map<std::string, itk::Array<double> > optimizerMap;
    if (testPreset)
    {
      optimizerMap = m_TestPreset->getOptimizerValuesPresets();
    }
    else
    {
      optimizerMap = m_Preset->getOptimizerValuesPresets();
    }
    optimizerMap[std::string((const char*)text.toLatin1())] = optimizerValues;

    itk::Array<double> interpolatorValues;
    interpolatorValues.SetSize(25);
    interpolatorValues.fill(0);
    interpolatorValues[0] = m_Controls.m_InterpolatorBox->currentIndex();

    std::map<std::string, itk::Array<double> > interpolatorMap;
    if (testPreset)
    {
      interpolatorMap = m_TestPreset->getInterpolatorValuesPresets();
    }
    else
    {
      interpolatorMap = m_Preset->getInterpolatorValuesPresets();
    }
    interpolatorMap[std::string((const char*)text.toLatin1())] = interpolatorValues;

    if (testPreset)
    {
      m_TestPreset->newPresets(transformMap, metricMap, optimizerMap, interpolatorMap);
    }
    else
    {
      m_Preset->newPresets(transformMap, metricMap, optimizerMap, interpolatorMap);
    }
  }
  else 
  {
    // user pressed Cancel
  }
}

void QmitkRigidRegistrationSelectorView::StopOptimization(bool stopOptimization)
{
  m_StopOptimization = stopOptimization;
}

int QmitkRigidRegistrationSelectorView::GetSelectedTransform()
{
  return m_Controls.m_TransformBox->currentIndex();
}

void QmitkRigidRegistrationSelectorView::SetFixedMaskNode( mitk::DataTreeNode * fixedMaskNode )
{
  m_FixedMaskNode = fixedMaskNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetMovingMaskNode( mitk::DataTreeNode * movingMaskNode )
{
  m_MovingMaskNode = movingMaskNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetMovingNodeChildren(mitk::DataStorage::SetOfObjects::ConstPointer children)
{
  m_MovingNodeChildren = children;
}
