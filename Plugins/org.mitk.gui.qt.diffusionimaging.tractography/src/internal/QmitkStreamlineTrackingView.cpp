/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

// Qmitk
#include "QmitkStreamlineTrackingView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>

// MITK
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkImageToItk.h>
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDimension.h>
#include <mitkOdfImage.h>
#include <mitkShImage.h>
#include <mitkSliceNavigationController.h>

// VTK
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyLine.h>
#include <vtkCellData.h>

#include <itkTensorImageToOdfImageFilter.h>
#include <omp.h>
#include <mitkLexicalCast.h>

const std::string QmitkStreamlineTrackingView::VIEW_ID = "org.mitk.views.streamlinetracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkStreamlineTrackingWorker::QmitkStreamlineTrackingWorker(QmitkStreamlineTrackingView* view)
    : m_View(view)
{
}

void QmitkStreamlineTrackingWorker::run()
{
    m_View->m_Tracker->Update();
    m_View->m_TrackingThread.quit();
}

QmitkStreamlineTrackingView::QmitkStreamlineTrackingView()
  : m_TrackingWorker(this)
  , m_Controls(nullptr)
  , m_FirstTensorProbRun(true)
  , m_FirstInteractiveRun(true)
  , m_TrackingHandler(nullptr)
  , m_ThreadIsRunning(false)
  , m_DeleteTrackingHandler(false)
  , m_Visible(false)
  , m_LastPrior(nullptr)
  , m_TrackingPriorHandler(nullptr)
{
  m_TrackingWorker.moveToThread(&m_TrackingThread);
  connect(&m_TrackingThread, SIGNAL(started()), this, SLOT(BeforeThread()));
  connect(&m_TrackingThread, SIGNAL(started()), &m_TrackingWorker, SLOT(run()));
  connect(&m_TrackingThread, SIGNAL(finished()), this, SLOT(AfterThread()));
  m_TrackingTimer = new QTimer(this);
}

// Destructor
QmitkStreamlineTrackingView::~QmitkStreamlineTrackingView()
{
  if (m_Tracker.IsNull())
    return;

  m_Tracker->SetStopTracking(true);
  m_TrackingThread.wait();
}

void QmitkStreamlineTrackingView::CreateQtPartControl( QWidget *parent )
{
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkStreamlineTrackingViewControls;
    m_Controls->setupUi( parent );
    m_Controls->m_FaImageSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_SeedImageSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MaskImageSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TargetImageSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_PriorImageSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_StopImageSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ForestSelectionWidget->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ExclusionImageSelectionWidget->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::PeakImage>::Pointer isPeakImagePredicate = mitk::TNodePredicateDataType<mitk::PeakImage>::New();
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::TNodePredicateDataType<mitk::TractographyForest>::Pointer isTractographyForest = mitk::TNodePredicateDataType<mitk::TractographyForest>::New();

    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer isNotBinaryPredicate = mitk::NodePredicateNot::New( isBinaryPredicate );
    mitk::NodePredicateAnd::Pointer isNotABinaryImagePredicate = mitk::NodePredicateAnd::New( isImagePredicate, isNotBinaryPredicate );
    mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);

    m_Controls->m_ForestSelectionWidget->SetNodePredicate(isTractographyForest);
    m_Controls->m_FaImageSelectionWidget->SetNodePredicate( mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, dimensionPredicate) );
    m_Controls->m_FaImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_FaImageSelectionWidget->SetSelectionIsOptional(true);

    m_Controls->m_SeedImageSelectionWidget->SetNodePredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_SeedImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_SeedImageSelectionWidget->SetSelectionIsOptional(true);

    m_Controls->m_MaskImageSelectionWidget->SetNodePredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_MaskImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_MaskImageSelectionWidget->SetSelectionIsOptional(true);

    m_Controls->m_StopImageSelectionWidget->SetNodePredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_StopImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_StopImageSelectionWidget->SetSelectionIsOptional(true);

    m_Controls->m_TargetImageSelectionWidget->SetNodePredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_TargetImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_TargetImageSelectionWidget->SetSelectionIsOptional(true);

    m_Controls->m_PriorImageSelectionWidget->SetNodePredicate( isPeakImagePredicate );
    m_Controls->m_PriorImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_PriorImageSelectionWidget->SetSelectionIsOptional(true);

    m_Controls->m_ExclusionImageSelectionWidget->SetNodePredicate( mitk::NodePredicateAnd::New(isImagePredicate, dimensionPredicate) );
    m_Controls->m_ExclusionImageSelectionWidget->SetEmptyInfo("--");
    m_Controls->m_ExclusionImageSelectionWidget->SetSelectionIsOptional(true);

    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()) );
    connect( m_Controls->m_SaveParametersButton, SIGNAL(clicked()), this, SLOT(SaveParameters()) );
    connect( m_Controls->m_LoadParametersButton, SIGNAL(clicked()), this, SLOT(LoadParameters()) );
    connect( m_Controls->commandLinkButton_2, SIGNAL(clicked()), this, SLOT(StopTractography()) );
    connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
    connect( m_Controls->m_InteractiveBox, SIGNAL(stateChanged(int)), this, SLOT(ToggleInteractive()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()) );
    connect( m_Controls->m_FaImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(DeleteTrackingHandler()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DeleteTrackingHandler()) );
    connect( m_Controls->m_OutputProbMap, SIGNAL(stateChanged(int)), this, SLOT(OutputStyleSwitched()) );

    connect( m_Controls->m_SeedImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_StopImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_TargetImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_PriorImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ExclusionImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MaskImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FaImageSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ForestSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(ForestSwitched()) );
    connect( m_Controls->m_ForestSelectionWidget, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SeedsPerVoxelBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_NumFibersBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_ScalarThresholdBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_OdfCutoffBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_StepSizeBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SamplingDistanceBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_AngularThresholdBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MinTractLengthBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MaxTractLengthBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_fBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_gBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_NumSamplesBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SeedRadiusBox, SIGNAL(editingFinished()), this, SLOT(InteractiveSeedChanged()) );
    connect( m_Controls->m_NumSeedsBox, SIGNAL(editingFinished()), this, SLOT(InteractiveSeedChanged()) );
    connect( m_Controls->m_OutputProbMap, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_SharpenOdfsBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_InterpolationBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_MaskInterpolationBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FlipXBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FlipYBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FlipZBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_PriorFlipXBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_PriorFlipYBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_PriorFlipZBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_FrontalSamplesBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_StopVotesBox, SIGNAL(stateChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_LoopCheckBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_TrialsPerSeedBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_EpConstraintsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnParameterChanged()) );
    connect( m_Controls->m_PeakJitterBox, SIGNAL(editingFinished()), this, SLOT(OnParameterChanged()) );

    m_Controls->m_SeedsPerVoxelBox->editingFinished();
    m_Controls->m_NumFibersBox->editingFinished();
    m_Controls->m_ScalarThresholdBox->editingFinished();
    m_Controls->m_OdfCutoffBox->editingFinished();
    m_Controls->m_StepSizeBox->editingFinished();
    m_Controls->m_SamplingDistanceBox->editingFinished();
    m_Controls->m_AngularThresholdBox->editingFinished();
    m_Controls->m_MinTractLengthBox->editingFinished();
    m_Controls->m_MaxTractLengthBox->editingFinished();
    m_Controls->m_fBox->editingFinished();
    m_Controls->m_gBox->editingFinished();
    m_Controls->m_NumSamplesBox->editingFinished();
    m_Controls->m_SeedRadiusBox->editingFinished();
    m_Controls->m_NumSeedsBox->editingFinished();
    m_Controls->m_LoopCheckBox->editingFinished();
    m_Controls->m_TrialsPerSeedBox->editingFinished();
    m_Controls->m_PeakJitterBox->editingFinished();

    StartStopTrackingGui(false);
  }
  m_ParameterFile = QDir::currentPath()+"/param.stp";

  UpdateGui();
}

void QmitkStreamlineTrackingView::ParametersToGui(mitk::StreamlineTractographyParameters& params)
{
  m_Controls->m_SeedRadiusBox->setValue(params.m_InteractiveRadiusMm);

  m_Controls->m_NumSeedsBox->setValue(params.m_NumInteractiveSeeds);
  m_Controls->m_InteractiveBox->setChecked(params.m_EnableInteractive);
  m_Controls->m_FiberErrorBox->setValue(params.m_Compression);
  m_Controls->m_ResampleFibersBox->setChecked(params.m_CompressFibers);

  m_Controls->m_SeedRadiusBox->setValue(params.m_InteractiveRadiusMm);
  m_Controls->m_NumFibersBox->setValue(params.m_MaxNumFibers);
  m_Controls->m_ScalarThresholdBox->setValue(params.m_Cutoff);
  m_Controls->m_fBox->setValue(params.m_F);
  m_Controls->m_gBox->setValue(params.m_G);

  m_Controls->m_OdfCutoffBox->setValue(params.m_OdfCutoff);
  m_Controls->m_SharpenOdfsBox->setChecked(params.m_SharpenOdfs);

  m_Controls->m_PriorWeightBox->setValue(params.m_Weight);
  m_Controls->m_PriorAsMaskBox->setChecked(params.m_RestrictToPrior);
  m_Controls->m_NewDirectionsFromPriorBox->setChecked(params.m_NewDirectionsFromPrior);

  m_Controls->m_PriorFlipXBox->setChecked(params.m_PriorFlipX);
  m_Controls->m_PriorFlipYBox->setChecked(params.m_PriorFlipY);
  m_Controls->m_PriorFlipZBox->setChecked(params.m_PriorFlipZ);

  m_Controls->m_FlipXBox->setChecked(params.m_FlipX);
  m_Controls->m_FlipYBox->setChecked(params.m_FlipY);
  m_Controls->m_FlipZBox->setChecked(params.m_FlipZ);
  m_Controls->m_InterpolationBox->setChecked(params.m_InterpolateTractographyData);


  m_Controls->m_MaskInterpolationBox->setChecked(params.m_InterpolateRoiImages);
  m_Controls->m_SeedsPerVoxelBox->setValue(params.m_SeedsPerVoxel);
  m_Controls->m_StepSizeBox->setValue(params.GetStepSizeVox());
  m_Controls->m_SamplingDistanceBox->setValue(params.GetSamplingDistanceVox());
  m_Controls->m_StopVotesBox->setChecked(params.m_StopVotes);
  m_Controls->m_FrontalSamplesBox->setChecked(params.m_OnlyForwardSamples);
  m_Controls->m_TrialsPerSeedBox->setValue(params.m_TrialsPerSeed);

  m_Controls->m_NumSamplesBox->setValue(params.m_NumSamples);
  m_Controls->m_LoopCheckBox->setValue(params.GetLoopCheckDeg());
  m_Controls->m_AngularThresholdBox->setValue(params.GetAngularThresholdDeg());
  m_Controls->m_MinTractLengthBox->setValue(params.m_MinTractLengthMm);
  m_Controls->m_MaxTractLengthBox->setValue(params.m_MaxTractLengthMm);
  m_Controls->m_OutputProbMap->setChecked(params.m_OutputProbMap);
  m_Controls->m_FixSeedBox->setChecked(params.m_FixRandomSeed);
  m_Controls->m_PeakJitterBox->setValue(params.m_PeakJitter);

  switch (params.m_Mode)
  {
  case mitk::TrackingDataHandler::MODE::DETERMINISTIC:
    m_Controls->m_ModeBox->setCurrentIndex(0);
    break;
  case mitk::TrackingDataHandler::MODE::PROBABILISTIC:
    m_Controls->m_ModeBox->setCurrentIndex(1);
    break;
  }

  switch (params.m_EpConstraints)
  {
  case itk::StreamlineTrackingFilter::EndpointConstraints::NONE:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(0);
    break;
  case itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(1);
    break;
  case itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET_LABELDIFF:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(2);
    break;
  case itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_SEED_AND_TARGET:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(3);
    break;
  case itk::StreamlineTrackingFilter::EndpointConstraints::MIN_ONE_EP_IN_TARGET:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(4);
    break;
  case itk::StreamlineTrackingFilter::EndpointConstraints::ONE_EP_IN_TARGET:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(5);
    break;
  case itk::StreamlineTrackingFilter::EndpointConstraints::NO_EP_IN_TARGET:
    m_Controls->m_EpConstraintsBox->setCurrentIndex(6);
    break;
  }
}

std::shared_ptr<mitk::StreamlineTractographyParameters> QmitkStreamlineTrackingView::GetParametersFromGui()
{

  // NOT IN GUI
//  unsigned int m_NumPreviousDirections = 1;
//  bool m_AvoidStop = true;
//  bool m_RandomSampling = false;
//  float m_DeflectionMod = 1.0;
//  bool m_ApplyDirectionMatrix = false;

  std::shared_ptr<mitk::StreamlineTractographyParameters> params = std::make_shared<mitk::StreamlineTractographyParameters>();
  params->m_InteractiveRadiusMm = m_Controls->m_SeedRadiusBox->value();
  params->m_NumInteractiveSeeds = m_Controls->m_NumSeedsBox->value();
  params->m_EnableInteractive = m_Controls->m_InteractiveBox->isChecked();
  params->m_Compression = m_Controls->m_FiberErrorBox->value();
  params->m_CompressFibers = m_Controls->m_ResampleFibersBox->isChecked();

  params->m_InteractiveRadiusMm = m_Controls->m_SeedRadiusBox->value();
  params->m_MaxNumFibers = m_Controls->m_NumFibersBox->value();
  params->m_Cutoff = static_cast<float>(m_Controls->m_ScalarThresholdBox->value());
  params->m_F = static_cast<float>(m_Controls->m_fBox->value());
  params->m_G = static_cast<float>(m_Controls->m_gBox->value());

  params->m_OdfCutoff = static_cast<float>(m_Controls->m_OdfCutoffBox->value());
  params->m_SharpenOdfs = m_Controls->m_SharpenOdfsBox->isChecked();

  params->m_Weight = static_cast<float>(m_Controls->m_PriorWeightBox->value());
  params->m_RestrictToPrior = m_Controls->m_PriorAsMaskBox->isChecked();
  params->m_NewDirectionsFromPrior = m_Controls->m_NewDirectionsFromPriorBox->isChecked();

  params->m_PriorFlipX = m_Controls->m_PriorFlipXBox->isChecked();
  params->m_PriorFlipY = m_Controls->m_PriorFlipYBox->isChecked();
  params->m_PriorFlipZ = m_Controls->m_PriorFlipZBox->isChecked();

  params->m_FlipX = m_Controls->m_FlipXBox->isChecked();
  params->m_FlipY = m_Controls->m_FlipYBox->isChecked();
  params->m_FlipZ = m_Controls->m_FlipZBox->isChecked();
  params->m_InterpolateTractographyData = m_Controls->m_InterpolationBox->isChecked();


  params->m_InterpolateRoiImages = m_Controls->m_MaskInterpolationBox->isChecked();
  params->m_SeedsPerVoxel = m_Controls->m_SeedsPerVoxelBox->value();
  params->SetStepSizeVox(m_Controls->m_StepSizeBox->value());
  params->SetSamplingDistanceVox(m_Controls->m_SamplingDistanceBox->value());
  params->m_StopVotes = m_Controls->m_StopVotesBox->isChecked();
  params->m_OnlyForwardSamples = m_Controls->m_FrontalSamplesBox->isChecked();
  params->m_TrialsPerSeed = m_Controls->m_TrialsPerSeedBox->value();

  params->m_NumSamples = m_Controls->m_NumSamplesBox->value();
  params->SetLoopCheckDeg(m_Controls->m_LoopCheckBox->value());
  params->SetAngularThresholdDeg(m_Controls->m_AngularThresholdBox->value());
  params->m_MinTractLengthMm = m_Controls->m_MinTractLengthBox->value();
  params->m_MaxTractLengthMm = m_Controls->m_MaxTractLengthBox->value();
  params->m_OutputProbMap = m_Controls->m_OutputProbMap->isChecked();
  params->m_FixRandomSeed = m_Controls->m_FixSeedBox->isChecked();
  params->m_PeakJitter = static_cast<float>(m_Controls->m_PeakJitterBox->value());

  switch (m_Controls->m_ModeBox->currentIndex())
  {
  case 0:
    params->m_Mode = mitk::TrackingDataHandler::MODE::DETERMINISTIC;
    break;
  case 1:
    params->m_Mode = mitk::TrackingDataHandler::MODE::PROBABILISTIC;
    break;
  default:
    params->m_Mode = mitk::TrackingDataHandler::MODE::DETERMINISTIC;
  }

  switch (m_Controls->m_EpConstraintsBox->currentIndex())
  {
  case 0:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::NONE;
    break;
  case 1:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET;
    break;
  case 2:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET_LABELDIFF;
    break;
  case 3:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_SEED_AND_TARGET;
    break;
  case 4:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::MIN_ONE_EP_IN_TARGET;
    break;
  case 5:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::ONE_EP_IN_TARGET;
    break;
  case 6:
    params->m_EpConstraints = itk::StreamlineTrackingFilter::EndpointConstraints::NO_EP_IN_TARGET;
    break;
  }

  return params;
}

void QmitkStreamlineTrackingView::SaveParameters()
{
  QString filename = QFileDialog::getSaveFileName(
        0,
        tr("Save Tractography Parameters"),
        m_ParameterFile,
        tr("Streamline Tractography Parameters (*.stp)") );

  if(filename.isEmpty() || filename.isNull())
    return;

  m_ParameterFile = filename;

  auto params = GetParametersFromGui();
  params->SaveParameters(m_ParameterFile.toStdString());
}

void QmitkStreamlineTrackingView::LoadParameters()
{
  QString filename = QFileDialog::getOpenFileName(
        0,
        tr("Load Tractography Parameters"),
        m_ParameterFile,
        tr("Streamline Tractography Parameters (*.stp)") );

  if(filename.isEmpty() || filename.isNull())
    return;

  m_ParameterFile = filename;

  mitk::StreamlineTractographyParameters params;
  params.LoadParameters(m_ParameterFile.toStdString());
  ParametersToGui(params);
}

void QmitkStreamlineTrackingView::StopTractography()
{
  if (m_Tracker.IsNull())
    return;

  m_Tracker->SetStopTracking(true);
}

void QmitkStreamlineTrackingView::TimerUpdate()
{
  if (m_Tracker.IsNull())
    return;

  QString status_text(m_Tracker->GetStatusText().c_str());
  m_Controls->m_StatusTextBox->setText(status_text);
}

void QmitkStreamlineTrackingView::BeforeThread()
{
  m_TrackingTimer->start(1000);
}

void QmitkStreamlineTrackingView::AfterThread()
{
  auto params = m_Tracker->GetParameters();
  m_TrackingTimer->stop();
  if (!params->m_OutputProbMap)
  {
    vtkSmartPointer<vtkPolyData> fiberBundle = m_Tracker->GetFiberPolyData();

    if (!m_Controls->m_InteractiveBox->isChecked() && fiberBundle->GetNumberOfLines() == 0)
    {
      QMessageBox warnBox;
      warnBox.setWindowTitle("Warning");
      warnBox.setText("No fiberbundle was generated!");
      warnBox.setDetailedText("No fibers were generated using the chosen parameters. Typical reasons are:\n\n- Cutoff too high. Some images feature very low FA/GFA/peak size. Try to lower this parameter.\n- Angular threshold too strict. Try to increase this parameter.\n- A small step sizes also means many steps to go wrong. Especially in the case of probabilistic tractography. Try to adjust the angular threshold.");
      warnBox.setIcon(QMessageBox::Warning);
      warnBox.exec();

      if (m_InteractivePointSetNode.IsNotNull())
        m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
      StartStopTrackingGui(false);
      if (m_DeleteTrackingHandler)
        DeleteTrackingHandler();
      UpdateGui();

      return;
    }

    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(fiberBundle);
    fib->SetReferenceGeometry(dynamic_cast<mitk::Image*>(m_ParentNode->GetData())->GetGeometry());
    if (m_Controls->m_ResampleFibersBox->isChecked() && fiberBundle->GetNumberOfLines()>0)
      fib->Compress(m_Controls->m_FiberErrorBox->value());
    fib->ColorFibersByOrientation();
    m_Tracker->SetDicomProperties(fib);

    if (m_Controls->m_InteractiveBox->isChecked())
    {
      if (m_InteractiveNode.IsNull())
      {
        m_InteractiveNode = mitk::DataNode::New();
        QString name("Interactive");
        m_InteractiveNode->SetName(name.toStdString());
        GetDataStorage()->Add(m_InteractiveNode);
      }
      m_InteractiveNode->SetData(fib);
      m_InteractiveNode->SetFloatProperty("Fiber2DSliceThickness", params->GetMinVoxelSizeMm()/2);

      if (auto renderWindowPart = this->GetRenderWindowPart())
          renderWindowPart->RequestUpdate();
    }
    else
    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(fib);
      QString name("FiberBundle_");
      name += m_ParentNode->GetName().c_str();
      name += "_Streamline";
      node->SetName(name.toStdString());
      node->SetFloatProperty("Fiber2DSliceThickness", params->GetMinVoxelSizeMm()/2);
      GetDataStorage()->Add(node, m_ParentNode);
    }
  }
  else
  {
    TrackerType::ItkDoubleImgType::Pointer outImg = m_Tracker->GetOutputProbabilityMap();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    if (m_Controls->m_InteractiveBox->isChecked())
    {
      if (m_InteractiveNode.IsNull())
      {
        m_InteractiveNode = mitk::DataNode::New();
        QString name("Interactive");
        m_InteractiveNode->SetName(name.toStdString());
        GetDataStorage()->Add(m_InteractiveNode);
      }
      m_InteractiveNode->SetData(img);

      mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
      lut->SetType(mitk::LookupTable::JET_TRANSPARENT);
      mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
      lut_prop->SetLookupTable(lut);
      m_InteractiveNode->SetProperty("LookupTable", lut_prop);
      m_InteractiveNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
      m_InteractiveNode->SetFloatProperty("Fiber2DSliceThickness", params->GetMinVoxelSizeMm()/2);

      if (auto renderWindowPart = this->GetRenderWindowPart())
          renderWindowPart->RequestUpdate();
    }
    else
    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(img);
      QString name("ProbabilityMap_");
      name += m_ParentNode->GetName().c_str();
      node->SetName(name.toStdString());

      mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
      lut->SetType(mitk::LookupTable::JET_TRANSPARENT);
      mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
      lut_prop->SetLookupTable(lut);
      node->SetProperty("LookupTable", lut_prop);
      node->SetProperty("opacity", mitk::FloatProperty::New(0.5));

      GetDataStorage()->Add(node, m_ParentNode);
    }
  }
  if (m_InteractivePointSetNode.IsNotNull())
    m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
  StartStopTrackingGui(false);
  if (m_DeleteTrackingHandler)
    DeleteTrackingHandler();
  UpdateGui();
}

void QmitkStreamlineTrackingView::InteractiveSeedChanged(bool posChanged)
{
  if(!CheckAndStoreLastParams(sender()) && !posChanged)
    return;
  if (m_ThreadIsRunning || !m_Visible)
    return;

  if (!posChanged && (!m_Controls->m_InteractiveBox->isChecked() || !m_Controls->m_ParamUpdateBox->isChecked()) )
    return;

  std::srand(std::time(0));
  m_SeedPoints.clear();

  itk::Point<float> world_pos = this->GetRenderWindowPart()->GetSelectedPosition();

  m_SeedPoints.push_back(world_pos);
  float radius = m_Controls->m_SeedRadiusBox->value();
  int num = m_Controls->m_NumSeedsBox->value();

  mitk::PointSet::Pointer pointset = mitk::PointSet::New();
  pointset->InsertPoint(0, world_pos);
  m_InteractivePointSetNode->SetProperty("pointsize", mitk::FloatProperty::New(radius*2));
  m_InteractivePointSetNode->SetProperty("point 2D size", mitk::FloatProperty::New(radius*2));
  m_InteractivePointSetNode->SetData(pointset);

  for (int i=1; i<num; i++)
  {
    itk::Vector<float> p;
    p[0] = rand()%1000-500;
    p[1] = rand()%1000-500;
    p[2] = rand()%1000-500;
    p.Normalize();
    p *= radius;
    m_SeedPoints.push_back(world_pos+p);
  }
  m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,0,0));
  DoFiberTracking();
}

bool QmitkStreamlineTrackingView::CheckAndStoreLastParams(QObject* obj)
{
  if (obj!=nullptr)
  {
    std::string new_val = "";
    if(qobject_cast<QDoubleSpinBox*>(obj)!=nullptr)
      new_val = boost::lexical_cast<std::string>(qobject_cast<QDoubleSpinBox*>(obj)->value());
    else if (qobject_cast<QSpinBox*>(obj)!=nullptr)
      new_val = boost::lexical_cast<std::string>(qobject_cast<QSpinBox*>(obj)->value());
    else
      return true;

    if (m_LastTractoParams.find(obj->objectName())==m_LastTractoParams.end())
    {
      m_LastTractoParams[obj->objectName()] = new_val;
      return false;
    }
    else if (m_LastTractoParams.at(obj->objectName()) != new_val)
    {
      m_LastTractoParams[obj->objectName()] = new_val;
      return true;
    }
    else if (m_LastTractoParams.at(obj->objectName()) == new_val)
      return false;
  }
  return true;
}

void QmitkStreamlineTrackingView::OnParameterChanged()
{
  UpdateGui();

  if(!CheckAndStoreLastParams(sender()))
    return;

  if (m_Controls->m_InteractiveBox->isChecked() && m_Controls->m_ParamUpdateBox->isChecked())
    DoFiberTracking();
}

void QmitkStreamlineTrackingView::ToggleInteractive()
{
  UpdateGui();

  m_Controls->m_SeedsPerVoxelBox->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
  m_Controls->m_SeedsPerVoxelLabel->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
  m_Controls->m_SeedImageSelectionWidget->setEnabled(!m_Controls->m_InteractiveBox->isChecked());
  m_Controls->label_6->setEnabled(!m_Controls->m_InteractiveBox->isChecked());

  if ( m_Controls->m_InteractiveBox->isChecked() )
  {
    if (m_FirstInteractiveRun)
    {
      QMessageBox::information(nullptr, "Information", "Place and move a spherical seed region anywhere in the image by left-clicking and dragging. If the seed region is colored red, tracking is in progress. If the seed region is colored white, tracking is finished.\nPlacing the seed region for the first time in a newly selected dataset might cause a short delay, since the tracker needs to be initialized.");
      m_FirstInteractiveRun = false;
    }

    QApplication::setOverrideCursor(Qt::PointingHandCursor);
    QApplication::processEvents();

    m_InteractivePointSetNode = mitk::DataNode::New();
    m_InteractivePointSetNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
    m_InteractivePointSetNode->SetName("InteractiveSeedRegion");
    mitk::PointSetShapeProperty::Pointer shape_prop = mitk::PointSetShapeProperty::New();
    shape_prop->SetValue(mitk::PointSetShapeProperty::PointSetShape::CIRCLE);
    m_InteractivePointSetNode->SetProperty("Pointset.2D.shape", shape_prop);
    GetDataStorage()->Add(m_InteractivePointSetNode);


    m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
    connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
  else
  {
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();

    m_InteractiveNode = nullptr;
    m_InteractivePointSetNode = nullptr;

    m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
    disconnect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
}

void QmitkStreamlineTrackingView::Activated()
{

}

void QmitkStreamlineTrackingView::Deactivated()
{

}

void QmitkStreamlineTrackingView::Visible()
{
  m_Visible = true;
}

void QmitkStreamlineTrackingView::Hidden()
{
  m_Visible = false;
  m_Controls->m_InteractiveBox->setChecked(false);
  ToggleInteractive();
}

void QmitkStreamlineTrackingView::OnSliceChanged()
{
  InteractiveSeedChanged(true);
}

void QmitkStreamlineTrackingView::SetFocus()
{
}

void QmitkStreamlineTrackingView::DeleteTrackingHandler()
{
  if (!m_ThreadIsRunning && m_TrackingHandler != nullptr)
  {
    if (m_TrackingPriorHandler != nullptr)
    {
      delete m_TrackingPriorHandler;
      m_TrackingPriorHandler = nullptr;
    }
    delete m_TrackingHandler;
    m_TrackingHandler = nullptr;
    m_DeleteTrackingHandler = false;
    m_LastPrior = nullptr;
  }
  else if (m_ThreadIsRunning)
  {
    m_DeleteTrackingHandler = true;
  }
}

void QmitkStreamlineTrackingView::ForestSwitched()
{
  DeleteTrackingHandler();
}

void QmitkStreamlineTrackingView::OutputStyleSwitched()
{
  if (m_InteractiveNode.IsNotNull())
    GetDataStorage()->Remove(m_InteractiveNode);
  m_InteractiveNode = nullptr;
}

void QmitkStreamlineTrackingView::OnSelectionChanged( berry::IWorkbenchPart::Pointer , const QList<mitk::DataNode::Pointer>& nodes )
{
  std::vector< mitk::DataNode::Pointer > last_nodes = m_InputImageNodes;
  m_InputImageNodes.clear();
  m_AdditionalInputImages.clear();
  bool retrack = false;

  for( auto node : nodes )
  {

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      if( dynamic_cast<mitk::TensorImage*>(node->GetData()) ||
          dynamic_cast<mitk::OdfImage*>(node->GetData()) ||
          dynamic_cast<mitk::ShImage*>(node->GetData()) ||
          dynamic_cast<mitk::PeakImage*>(node->GetData()) ||
          mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(node->GetData())))
      {
        m_InputImageNodes.push_back(node);
        retrack = true;
      }
      else
      {
        mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
        if (img!=nullptr && img->GetDimension()==3)
          m_AdditionalInputImages.push_back(dynamic_cast<mitk::Image*>(node->GetData()));
      }
    }
  }

  // sometimes the OnSelectionChanged event is sent twice and actually no selection has changed for the first event. We need to catch that.
  if (last_nodes.size() == m_InputImageNodes.size())
  {
    bool same_nodes = true;
    for (unsigned int i=0; i<m_InputImageNodes.size(); i++)
      if (last_nodes.at(i)!=m_InputImageNodes.at(i))
      {
        same_nodes = false;
        break;
      }
    if (same_nodes)
      return;
  }

  DeleteTrackingHandler();
  UpdateGui();
  if (retrack)
    OnParameterChanged();
}

void QmitkStreamlineTrackingView::UpdateGui()
{
  m_Controls->m_TensorImageLabel->setText("<font color='red'>select in data-manager</font>");

  m_Controls->m_fBox->setEnabled(false);
  m_Controls->m_fLabel->setEnabled(false);
  m_Controls->m_gBox->setEnabled(false);
  m_Controls->m_gLabel->setEnabled(false);
  m_Controls->m_FaImageSelectionWidget->setEnabled(true);
  m_Controls->mFaImageLabel->setEnabled(true);
  m_Controls->m_OdfCutoffBox->setEnabled(false);
  m_Controls->m_OdfCutoffLabel->setEnabled(false);
  m_Controls->m_SharpenOdfsBox->setEnabled(false);
  m_Controls->m_ForestSelectionWidget->setVisible(false);
  m_Controls->m_ForestLabel->setVisible(false);
  m_Controls->commandLinkButton->setEnabled(false);
  m_Controls->m_TrialsPerSeedBox->setEnabled(false);
  m_Controls->m_TrialsPerSeedLabel->setEnabled(false);
  m_Controls->m_TargetImageSelectionWidget->setEnabled(false);
  m_Controls->m_TargetImageLabel->setEnabled(false);
  m_Controls->m_PeakJitterBox->setEnabled(false);

  if (m_Controls->m_InteractiveBox->isChecked())
  {
    m_Controls->m_InteractiveSeedingFrame->setVisible(true);
    m_Controls->m_StaticSeedingFrame->setVisible(false);
    m_Controls->commandLinkButton_2->setVisible(false);
    m_Controls->commandLinkButton->setVisible(false);
  }
  else
  {
    m_Controls->m_InteractiveSeedingFrame->setVisible(false);
    m_Controls->m_StaticSeedingFrame->setVisible(true);
    m_Controls->commandLinkButton_2->setVisible(m_ThreadIsRunning);
    m_Controls->commandLinkButton->setVisible(!m_ThreadIsRunning);
  }

  if (m_Controls->m_EpConstraintsBox->currentIndex()>0)
  {
    m_Controls->m_TargetImageSelectionWidget->setEnabled(true);
    m_Controls->m_TargetImageLabel->setEnabled(true);
  }

  // stuff that is only important for probabilistic tractography
  if (m_Controls->m_ModeBox->currentIndex()==1)
  {
    m_Controls->m_TrialsPerSeedBox->setEnabled(true);
    m_Controls->m_TrialsPerSeedLabel->setEnabled(true);
    m_Controls->m_PeakJitterBox->setEnabled(true);
  }

  if(!m_InputImageNodes.empty())
  {
    if (m_InputImageNodes.size()>1)
      m_Controls->m_TensorImageLabel->setText( ( std::to_string(m_InputImageNodes.size()) + " images selected").c_str() );
    else
      m_Controls->m_TensorImageLabel->setText(m_InputImageNodes.at(0)->GetName().c_str());
    m_Controls->commandLinkButton->setEnabled(!m_Controls->m_InteractiveBox->isChecked() && !m_ThreadIsRunning);

    m_Controls->m_ScalarThresholdBox->setEnabled(true);
    m_Controls->m_FaThresholdLabel->setEnabled(true);

    if ( dynamic_cast<mitk::TensorImage*>(m_InputImageNodes.at(0)->GetData()) )
    {
      m_Controls->m_fBox->setEnabled(true);
      m_Controls->m_fLabel->setEnabled(true);
      m_Controls->m_gBox->setEnabled(true);
      m_Controls->m_gLabel->setEnabled(true);
    }
    else if ( dynamic_cast<mitk::OdfImage*>(m_InputImageNodes.at(0)->GetData()) ||
              dynamic_cast<mitk::ShImage*>(m_InputImageNodes.at(0)->GetData()))
    {
      m_Controls->m_OdfCutoffBox->setEnabled(true);
      m_Controls->m_OdfCutoffLabel->setEnabled(true);
      m_Controls->m_SharpenOdfsBox->setEnabled(true);
    }
    else if (  mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(m_InputImageNodes.at(0)->GetData())) )
    {
      m_Controls->m_ForestSelectionWidget->setVisible(true);
      m_Controls->m_ForestLabel->setVisible(true);
      m_Controls->m_ScalarThresholdBox->setEnabled(false);
      m_Controls->m_FaThresholdLabel->setEnabled(false);
    }
  }
}

void QmitkStreamlineTrackingView::StartStopTrackingGui(bool start)
{
  m_ThreadIsRunning = start;

  if (!m_Controls->m_InteractiveBox->isChecked())
  {
    m_Controls->commandLinkButton_2->setVisible(start);
    m_Controls->commandLinkButton->setVisible(!start);
    m_Controls->m_InteractiveBox->setEnabled(!start);
    m_Controls->m_StatusTextBox->setVisible(start);
  }
}

void QmitkStreamlineTrackingView::DoFiberTracking()
{
  auto params = GetParametersFromGui();

  if (m_InputImageNodes.empty())
  {
    QMessageBox::information(nullptr, "Information", "Please select an input image in the datamaneger (tensor, ODF, peak or dMRI image)!");
    return;
  }
  if (m_ThreadIsRunning || !m_Visible)
    return;
  if (m_Controls->m_InteractiveBox->isChecked() && m_SeedPoints.empty())
    return;
  StartStopTrackingGui(true);

  m_Tracker = TrackerType::New();

  if (params->m_EpConstraints == itk::StreamlineTrackingFilter::EndpointConstraints::NONE)
      m_Tracker->SetTargetRegions(nullptr);

  if( dynamic_cast<mitk::TensorImage*>(m_InputImageNodes.at(0)->GetData()) )
  {
    if (m_Controls->m_ModeBox->currentIndex()==1)
    {
      if (m_InputImageNodes.size()>1)
      {
        QMessageBox::information(nullptr, "Information", "Probabilistic tensor tractography is only implemented for single-tensor mode!");
        StartStopTrackingGui(false);
        return;
      }

      if (m_TrackingHandler==nullptr)
      {
        m_TrackingHandler = new mitk::TrackingHandlerOdf();
        typedef itk::TensorImageToOdfImageFilter< float, float > FilterType;
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput( mitk::convert::GetItkTensorFromTensorImage(dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())) );
        filter->Update();
        dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfImage(filter->GetOutput());

        if (m_Controls->m_FaImageSelectionWidget->GetSelectedNode().IsNotNull())
        {
          ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
          mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageSelectionWidget->GetSelectedNode()->GetData()), itkImg);

          dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetGfaImage(itkImg);
        }
      }

      dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetIsOdfFromTensor(true);
    }
    else
    {
      if (m_TrackingHandler==nullptr)
      {
        m_TrackingHandler = new mitk::TrackingHandlerTensor();
        for (unsigned int i=0; i<m_InputImageNodes.size(); ++i)
          dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->AddTensorImage(mitk::convert::GetItkTensorFromTensorImage(dynamic_cast<mitk::Image*>(m_InputImageNodes.at(i)->GetData())).GetPointer());

        if (m_Controls->m_FaImageSelectionWidget->GetSelectedNode().IsNotNull())
        {
          ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
          mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageSelectionWidget->GetSelectedNode()->GetData()), itkImg);

          dynamic_cast<mitk::TrackingHandlerTensor*>(m_TrackingHandler)->SetFaImage(itkImg);
        }
      }
    }
  }
  else if ( dynamic_cast<mitk::OdfImage*>(m_InputImageNodes.at(0)->GetData()) ||
            dynamic_cast<mitk::ShImage*>(m_InputImageNodes.at(0)->GetData()))
  {
    if (m_TrackingHandler==nullptr)
    {
      m_TrackingHandler = new mitk::TrackingHandlerOdf();

      if (dynamic_cast<mitk::ShImage*>(m_InputImageNodes.at(0)->GetData()))
        dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfImage(mitk::convert::GetItkOdfFromShImage(dynamic_cast<mitk::ShImage*>(m_InputImageNodes.at(0)->GetData())));
      else
        dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetOdfImage(mitk::convert::GetItkOdfFromOdfImage(dynamic_cast<mitk::OdfImage*>(m_InputImageNodes.at(0)->GetData())));

      if (m_Controls->m_FaImageSelectionWidget->GetSelectedNode().IsNotNull())
      {
        ItkFloatImageType::Pointer itkImg = ItkFloatImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_FaImageSelectionWidget->GetSelectedNode()->GetData()), itkImg);
        dynamic_cast<mitk::TrackingHandlerOdf*>(m_TrackingHandler)->SetGfaImage(itkImg);
      }
    }
  }
  else if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())) )
  {
    if ( m_Controls->m_ForestSelectionWidget->GetSelectedNode().IsNull() )
    {
      QMessageBox::information(nullptr, "Information", "Not random forest for machine learning based tractography (raw dMRI tractography) selected. Did you accidentally select the raw diffusion-weighted image in the datamanager?");
      StartStopTrackingGui(false);
      return;
    }

    if (m_TrackingHandler==nullptr)
    {
      mitk::TractographyForest::Pointer forest = dynamic_cast<mitk::TractographyForest*>(m_Controls->m_ForestSelectionWidget->GetSelectedNode()->GetData());
      mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData());

      std::vector< std::vector< ItkFloatImageType::Pointer > > additionalFeatureImages;
      additionalFeatureImages.push_back(std::vector< ItkFloatImageType::Pointer >());
      for (auto img : m_AdditionalInputImages)
      {
        ItkFloatImageType::Pointer itkimg = ItkFloatImageType::New();
        mitk::CastToItkImage(img, itkimg);
        additionalFeatureImages.at(0).push_back(itkimg);
      }

      bool forest_valid = false;
      if (forest->GetNumFeatures()>=100)
      {
        params->m_NumPreviousDirections = static_cast<unsigned int>((forest->GetNumFeatures() - (100 + additionalFeatureImages.at(0).size()))/3);
        m_TrackingHandler = new mitk::TrackingHandlerRandomForest<6, 100>();
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->AddDwi(dwi);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->SetAdditionalFeatureImages(additionalFeatureImages);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->SetForest(forest);
        forest_valid = dynamic_cast<mitk::TrackingHandlerRandomForest<6, 100>*>(m_TrackingHandler)->IsForestValid();
      }
      else
      {
        params->m_NumPreviousDirections = static_cast<unsigned int>((forest->GetNumFeatures() - (28 + additionalFeatureImages.at(0).size()))/3);
        m_TrackingHandler = new mitk::TrackingHandlerRandomForest<6, 28>();
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->AddDwi(dwi);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->SetAdditionalFeatureImages(additionalFeatureImages);
        dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->SetForest(forest);
        forest_valid = dynamic_cast<mitk::TrackingHandlerRandomForest<6, 28>*>(m_TrackingHandler)->IsForestValid();
      }

      if (!forest_valid)
      {
        QMessageBox::information(nullptr, "Information", "Random forest is invalid. The forest signatue does not match the parameters of TrackingHandlerRandomForest.");
        StartStopTrackingGui(false);
        return;
      }
    }
  }
  else
  {
    if (m_TrackingHandler==nullptr)
    {
      m_TrackingHandler = new mitk::TrackingHandlerPeaks();
      dynamic_cast<mitk::TrackingHandlerPeaks*>(m_TrackingHandler)->SetPeakImage(mitk::convert::GetItkPeakFromPeakImage(dynamic_cast<mitk::Image*>(m_InputImageNodes.at(0)->GetData())));
    }
  }

  if (m_Controls->m_InteractiveBox->isChecked())
  {
    m_Tracker->SetSeedPoints(m_SeedPoints);
  }
  else if (m_Controls->m_SeedImageSelectionWidget->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_SeedImageSelectionWidget->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetSeedImage(mask);
  }

  if (m_Controls->m_MaskImageSelectionWidget->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_MaskImageSelectionWidget->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetMaskImage(mask);
  }

  if (m_Controls->m_StopImageSelectionWidget->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_StopImageSelectionWidget->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetStoppingRegions(mask);
  }

  if (m_Controls->m_TargetImageSelectionWidget->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_TargetImageSelectionWidget->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetTargetRegions(mask);
  }

  if (m_Controls->m_PriorImageSelectionWidget->GetSelectedNode().IsNotNull())
  {
    auto prior_params = GetParametersFromGui();
    if (m_LastPrior!=m_Controls->m_PriorImageSelectionWidget->GetSelectedNode() || m_TrackingPriorHandler==nullptr)
    {
      typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(dynamic_cast<mitk::PeakImage*>(m_Controls->m_PriorImageSelectionWidget->GetSelectedNode()->GetData()));
      caster->SetCopyMemFlag(true);
      caster->Update();
      mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();
      m_TrackingPriorHandler = new mitk::TrackingHandlerPeaks();
      dynamic_cast<mitk::TrackingHandlerPeaks*>(m_TrackingPriorHandler)->SetPeakImage(itkImg);
      m_LastPrior = m_Controls->m_PriorImageSelectionWidget->GetSelectedNode();
    }

    prior_params->m_FlipX = m_Controls->m_PriorFlipXBox->isChecked();
    prior_params->m_FlipY = m_Controls->m_PriorFlipYBox->isChecked();
    prior_params->m_FlipZ = m_Controls->m_PriorFlipZBox->isChecked();
    m_TrackingPriorHandler->SetParameters(prior_params);

    m_Tracker->SetTrackingPriorHandler(m_TrackingPriorHandler);
  }
  else if (m_Controls->m_PriorImageSelectionWidget->GetSelectedNode().IsNull())
    m_Tracker->SetTrackingPriorHandler(nullptr);

  if (m_Controls->m_ExclusionImageSelectionWidget->GetSelectedNode().IsNotNull())
  {
    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_ExclusionImageSelectionWidget->GetSelectedNode()->GetData()), mask);
    m_Tracker->SetExclusionRegions(mask);
  }

  if (params->m_EpConstraints!=itk::StreamlineTrackingFilter::EndpointConstraints::NONE && m_Controls->m_TargetImageSelectionWidget->GetSelectedNode().IsNull())
  {
    QMessageBox::information(nullptr, "Error", "Endpoint constraints are used but no target image is set!");
    StartStopTrackingGui(false);
    return;
  }
  else if (params->m_EpConstraints==itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_SEED_AND_TARGET
           && (m_Controls->m_SeedImageSelectionWidget->GetSelectedNode().IsNull()|| m_Controls->m_TargetImageSelectionWidget->GetSelectedNode().IsNull()) )
  {
    QMessageBox::information(nullptr, "Error", "Endpoint constraint EPS_IN_SEED_AND_TARGET is used but no target or no seed image is set!");
    StartStopTrackingGui(false);
    return;
  }

  m_Tracker->SetParameters(params);
  m_Tracker->SetTrackingHandler(m_TrackingHandler);
  m_Tracker->SetVerbose(!m_Controls->m_InteractiveBox->isChecked());

  m_ParentNode = m_InputImageNodes.at(0);
  m_TrackingThread.start(QThread::LowestPriority);
}
