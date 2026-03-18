/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkNodePredicateDataProperty.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkRegVisPropertyTags.h>
#include <mitkMatchPointPropertyTags.h>
#include <mitkRegEvaluationObject.h>
#include <mitkRegistrationHelper.h>
#include <mitkRegEvaluationMapper2D.h>
#include <mitkMAPAlgorithmHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkUIDHelper.h>
#include <mitkProperties.h>
#include <mitkRegistrationManipulationInteractor.h>
#include <mitkCrosshairData.h>

#include <usModuleRegistry.h>

// Qmitk
#include <QmitkRenderWindow.h>
#include "QmitkMatchPointRegistrationManipulator.h"
#include <QmitkMappingJob.h>

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>
#include <QThreadPool>

//MatchPoint
#include <mapRegistrationManipulator.h>
#include <mapPreCachedRegistrationKernel.h>
#include <mapCombinedRegistrationKernel.h>
#include <mapNullRegistrationKernel.h>
#include <mapRegistrationCombinator.h>

#include <itkCompositeTransform.h>

#include <boost/math/constants/constants.hpp>

#include <ui_QmitkMatchPointRegistrationManipulator.h>

const std::string QmitkMatchPointRegistrationManipulator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.manipulator";

const std::string QmitkMatchPointRegistrationManipulator::HelperNodeName =
    "RegistrationManipulationEvaluationHelper";

QmitkMatchPointRegistrationManipulator::QmitkMatchPointRegistrationManipulator()
  : m_Controls(std::make_unique<Ui::MatchPointRegistrationManipulatorControls>()),
    m_Parent(nullptr),
    m_activeManipulation(false),
    m_currentSelectedTimePoint(0.),
    m_internalUpdate(false)
{
  m_currentSelectedPosition.Fill(0.0);
}

QmitkMatchPointRegistrationManipulator::~QmitkMatchPointRegistrationManipulator()
{
  if (this->m_EvalNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_EvalNode);
  }

}

void QmitkMatchPointRegistrationManipulator::SetFocus()
{

}

void QmitkMatchPointRegistrationManipulator::Error(QString msg)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
  MITK_ERROR << msg.toStdString().c_str();
}

void QmitkMatchPointRegistrationManipulator::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls->setupUi(parent);

  m_Parent = parent;

  m_Controls->registrationNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls->registrationNodeSelector->SetSelectionIsOptional(false);
  m_Controls->movingNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls->movingNodeSelector->SetSelectionIsOptional(false);
  m_Controls->targetNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls->targetNodeSelector->SetSelectionIsOptional(false);

  m_Controls->registrationNodeSelector->SetInvalidInfo("Select base registration.");
  m_Controls->registrationNodeSelector->SetPopUpTitel("Select registration.");
  m_Controls->registrationNodeSelector->SetPopUpHint("Select a registration object that should be used as starting point for the manual manipulation.");

  m_Controls->movingNodeSelector->SetInvalidInfo("Select moving image.");
  m_Controls->movingNodeSelector->SetPopUpTitel("Select moving image.");
  m_Controls->movingNodeSelector->SetPopUpHint("Select the moving image for the evaluation. This is the image that will be mapped by the registration.");
  m_Controls->targetNodeSelector->SetInvalidInfo("Select target image.");
  m_Controls->targetNodeSelector->SetPopUpTitel("Select target image.");
  m_Controls->targetNodeSelector->SetPopUpHint("Select the target image for the evaluation.");
  m_Controls->checkAutoSelect->setChecked(true);

  this->ConfigureNodePredicates();

  connect(m_Controls->pbStart, SIGNAL(clicked()), this, SLOT(OnStartBtnPushed()));
  connect(m_Controls->pbCancel, SIGNAL(clicked()), this, SLOT(OnCancelBtnPushed()));
  connect(m_Controls->pbStore, SIGNAL(clicked()), this, SLOT(OnStoreBtnPushed()));
  connect(m_Controls->evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));
  connect(m_Controls->radioSelectedReg, SIGNAL(toggled(bool)), this, SLOT(OnRegSourceChanged()));

  connect(m_Controls->comboCenter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCenterTypeChanged(int)));
  connect(m_Controls->manipulationWidget, SIGNAL(RegistrationChanged(map::core::RegistrationBase*)), this, SLOT(OnRegistrationChanged()));
  connect(m_Controls->pbInteractionTool, SIGNAL(toggled(bool)), this, SLOT(OnInteractionToolToggled(bool)));
  connect(m_Controls->checkScaling, SIGNAL(toggled(bool)), this, SLOT(OnScalingCheckboxToggled(bool)));

  connect(m_Controls->registrationNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationManipulator::OnNodeSelectionChanged);
  connect(m_Controls->movingNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationManipulator::OnNodeSelectionChanged);
  connect(m_Controls->targetNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationManipulator::OnNodeSelectionChanged);

  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN));
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

  m_Controls->radioNewReg->setChecked(true);

  m_EvalNode = this->GetDataStorage()->GetNamedNode(HelperNodeName);

  this->CheckInputs();
  this->StopSession();
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkMatchPointRegistrationManipulator::RenderWindowPartDeactivated(
  mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkMatchPointRegistrationManipulator::ConfigureNodePredicates()
{
  m_Controls->registrationNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::RegNodePredicate());

  m_Controls->movingNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::ImageNodePredicate());
  m_Controls->targetNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::ImageNodePredicate());
}

void QmitkMatchPointRegistrationManipulator::CheckInputs()
{
  if (!m_activeManipulation)
  {
    bool autoSelectInput = m_Controls->checkAutoSelect->isChecked() && this->m_SelectedPreRegNode != m_Controls->registrationNodeSelector->GetSelectedNode();
    this->m_SelectedPreRegNode = m_Controls->registrationNodeSelector->GetSelectedNode();
    this->m_SelectedMovingNode = m_Controls->movingNodeSelector->GetSelectedNode();
    this->m_SelectedTargetNode = m_Controls->targetNodeSelector->GetSelectedNode();

    if (this->m_SelectedPreRegNode.IsNotNull())
    {
      mitk::MAPRegistrationWrapper* regWrapper = dynamic_cast<mitk::MAPRegistrationWrapper*>(m_SelectedPreRegNode->GetData());
      if (regWrapper)
      {
        this->m_SelectedPreReg = dynamic_cast<MAPRegistrationType*>(regWrapper->GetRegistration());
      }
    }

    if (this->m_SelectedPreRegNode.IsNotNull() && (this->m_SelectedMovingNode.IsNull() || autoSelectInput))
    {
      mitk::BaseProperty* uidProp = m_SelectedPreRegNode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);

      if (uidProp)
      {
        //search for the moving node
        mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
          uidProp);
        mitk::DataNode::Pointer movingNode = this->GetDataStorage()->GetNode(predicate);
        if (movingNode.IsNotNull())
        {
          this->m_SelectedMovingNode = movingNode;
          QmitkSingleNodeSelectionWidget::NodeList selection({ movingNode });
          m_Controls->movingNodeSelector->SetCurrentSelection(selection);
        }
      }
    }

    if (this->m_SelectedPreRegNode.IsNotNull() && (this->m_SelectedTargetNode.IsNull() || autoSelectInput))
    {
      mitk::BaseProperty* uidProp = m_SelectedPreRegNode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);

      if (uidProp)
      {
        //search for the target node
        mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
          uidProp);
        mitk::DataNode::Pointer targetNode = this->GetDataStorage()->GetNode(predicate);
        if (targetNode.IsNotNull())
        {
          this->m_SelectedTargetNode = targetNode;
          QmitkSingleNodeSelectionWidget::NodeList selection({ targetNode });
          m_Controls->targetNodeSelector->SetCurrentSelection(selection);
        }
      }
    }
  }
}

void QmitkMatchPointRegistrationManipulator::OnRegSourceChanged()
{
  this->CheckInputs();
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  this->CheckInputs();
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::NodeRemoved(const mitk::DataNode* node)
{
  if (node == this->m_SelectedMovingNode
    || node == this->m_SelectedTargetNode
    || node == this->m_EvalNode)
  {
    if (node == this->m_EvalNode)
    {
      this->m_EvalNode = nullptr;
    }
    if (this->m_activeManipulation)
    {
      MITK_INFO << "Stopped current MatchPoint manual registration session, because at least one relevant node was removed from storage.";
    }
    this->OnCancelBtnPushed();

  }
}

void QmitkMatchPointRegistrationManipulator::ConfigureControls()
{
  if (!m_activeManipulation)
  {
    QString name = "ManualRegistration";

    if (m_SelectedPreRegNode.IsNotNull())
    {
      name = QString::fromStdString(m_SelectedPreRegNode->GetName()) + " manual refined";
    }
    m_Controls->lbNewRegName->setText(name);
  }

  //config settings widget
  m_Controls->groupReg->setEnabled(!m_activeManipulation);

  m_Controls->pbStart->setEnabled(this->m_SelectedMovingNode.IsNotNull()
    && this->m_SelectedTargetNode.IsNotNull()
    && !m_activeManipulation
    && (m_Controls->radioNewReg->isChecked() || this->m_SelectedPreReg.IsNotNull()));

  m_Controls->lbNewRegName->setEnabled(m_activeManipulation);
  m_Controls->checkMapEntity->setEnabled(m_activeManipulation);
  m_Controls->tabWidget->setEnabled(m_activeManipulation);
  m_Controls->pbCancel->setEnabled(m_activeManipulation);
  m_Controls->pbStore->setEnabled(m_activeManipulation);
  m_Controls->registrationNodeSelector->setEnabled(!m_activeManipulation && m_Controls->radioSelectedReg->isChecked());
  m_Controls->checkAutoSelect->setEnabled(!m_activeManipulation && m_Controls->radioSelectedReg->isChecked());
  m_Controls->movingNodeSelector->setEnabled(!m_activeManipulation);
  m_Controls->targetNodeSelector->setEnabled(!m_activeManipulation);

  // Interaction tool controls: only available during active manipulation
  m_Controls->pbInteractionTool->setEnabled(m_activeManipulation);
  m_Controls->checkScaling->setEnabled(m_activeManipulation);
  m_Controls->lblInteractionInfo->setVisible(m_activeManipulation && m_InteractionToolActive);
  m_Controls->checkScaling->setVisible(m_activeManipulation);

  if (!m_activeManipulation)
  {
    m_Controls->pbInteractionTool->setChecked(false);
  }
}

void QmitkMatchPointRegistrationManipulator::InitSession()
{
  if (m_Controls->radioNewReg->isChecked())
  { //init to map the image centers
    auto movingCenter = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    auto targetCenter = m_SelectedTargetNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    m_Controls->manipulationWidget->Initialize(movingCenter, targetCenter);
  }
  else
  { //use selected pre registration as baseline
    m_Controls->manipulationWidget->Initialize(m_SelectedPreReg);
  }

  this->m_CurrentRegistration = m_Controls->manipulationWidget->GetInterimRegistration();
  this->m_CurrentRegistrationWrapper = mitk::MAPRegistrationWrapper::New(m_CurrentRegistration);

  m_Controls->comboCenter->setCurrentIndex(0);
  this->OnCenterTypeChanged(0);

  //reinit view
  mitk::RenderingManager::GetInstance()->InitializeViews(m_SelectedTargetNode->GetData()->GetTimeGeometry());

  //generate evaluation node

  mitk::RegEvaluationObject::Pointer regEval = mitk::RegEvaluationObject::New();
  regEval->SetRegistration(this->m_CurrentRegistrationWrapper);
  regEval->SetTargetNode(this->m_SelectedTargetNode);
  regEval->SetMovingNode(this->m_SelectedMovingNode);

  this->m_EvalNode = mitk::DataNode::New();
  this->m_EvalNode->SetData(regEval);

  mitk::RegEvaluationMapper2D::SetDefaultProperties(this->m_EvalNode);
  this->m_EvalNode->SetName(HelperNodeName);
  this->m_EvalNode->SetBoolProperty("helper object", true);
  this->GetDataStorage()->Add(this->m_EvalNode);

  m_Controls->evalSettings->SetNode(this->m_EvalNode);

  this->m_activeManipulation = true;
}

void QmitkMatchPointRegistrationManipulator::StopSession()
{
  if (m_InteractionToolActive)
  {
    this->DeactivateInteractionTool();
  }

  this->m_activeManipulation = false;

  if (this->m_EvalNode.IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_EvalNode);
  }

  this->m_EvalNode = nullptr;

  this->m_CurrentRegistration = nullptr;
  this->m_CurrentRegistrationWrapper = nullptr;
  m_Controls->manipulationWidget->Initialize();
}


void QmitkMatchPointRegistrationManipulator::OnRegistrationChanged()
{
  if (this->m_EvalNode.IsNotNull())
  {
    this->m_EvalNode->Modified();
  }
  if (this->m_CurrentRegistrationWrapper.IsNotNull())
  {
    this->m_CurrentRegistrationWrapper->Modified();
  }

  auto* renderWindowPart = this->GetRenderWindowPart();

  if (nullptr != renderWindowPart)
    renderWindowPart->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnSliceChanged()
{
  auto* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  auto currentSelectedPosition = renderWindowPart->GetSelectedPosition(nullptr);
  auto currentTimePoint = renderWindowPart->GetSelectedTimePoint();

  if (m_currentSelectedPosition != currentSelectedPosition
    || m_currentSelectedTimePoint != currentTimePoint
    || m_selectedNodeTime > m_currentPositionTime)
  {
    //the current position has been changed or the selected node has been changed since the last position validation -> check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimePoint = currentTimePoint;
    m_currentPositionTime.Modified();

    if (this->m_EvalNode.IsNotNull())
    {
      this->m_EvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::Point3dProperty::New(currentSelectedPosition));
    }

    if (m_activeManipulation && m_Controls->comboCenter->currentIndex() == 2)
    { //update transform with the current position.
      m_Controls->manipulationWidget->SetCenterOfRotation(m_currentSelectedPosition);
      if (m_Interactor.IsNotNull())
      {
        m_Interactor->SetCenterOfRotation(m_currentSelectedPosition);
      }
    }
  }
}

void QmitkMatchPointRegistrationManipulator::OnSettingsChanged(mitk::DataNode*)
{
  auto* renderWindowPart = this->GetRenderWindowPart();

  if (nullptr != renderWindowPart)
    renderWindowPart->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnStartBtnPushed()
{
  this->InitSession();
  this->OnSliceChanged();

  auto* renderWindowPart = this->GetRenderWindowPart();

  if (nullptr != renderWindowPart)
    renderWindowPart->RequestUpdate();

  this->CheckInputs();
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::OnCancelBtnPushed()
{
  this->StopSession();

  this->CheckInputs();
  this->ConfigureControls();
  if (this->GetRenderWindowPart())
  {
    this->GetRenderWindowPart()->RequestUpdate();
  }

}

void QmitkMatchPointRegistrationManipulator::OnStoreBtnPushed()
{
  map::core::RegistrationBase::Pointer newReg = m_Controls->manipulationWidget->GenerateRegistration();
  auto newRegWrapper = mitk::MAPRegistrationWrapper::New(newReg);

  mitk::DataNode::Pointer spResultRegistrationNode = mitk::generateRegistrationResultNode(
    m_Controls->lbNewRegName->text().toStdString(), newRegWrapper, "org.mitk::manual_registration",
    mitk::EnsureUID(m_SelectedMovingNode->GetData()), mitk::EnsureUID(m_SelectedTargetNode->GetData()));

  this->GetDataStorage()->Add(spResultRegistrationNode);

  if (m_Controls->checkMapEntity->checkState() == Qt::Checked)
  {
    QmitkMappingJob* pMapJob = new QmitkMappingJob();
    pMapJob->setAutoDelete(true);

    pMapJob->m_spInputData = this->m_SelectedMovingNode->GetData();
    pMapJob->m_InputDataUID = mitk::EnsureUID(m_SelectedMovingNode->GetData());
    pMapJob->m_spRegNode = spResultRegistrationNode;
    pMapJob->m_doGeometryRefinement = false;
    pMapJob->m_spRefGeometry = this->m_SelectedTargetNode->GetData()->GetGeometry()->Clone().GetPointer();

    pMapJob->m_MappedName = m_Controls->lbNewRegName->text().toStdString() + std::string(" mapped moving data");
    pMapJob->m_allowUndefPixels = true;
    pMapJob->m_paddingValue = 100;
    pMapJob->m_allowUnregPixels = true;
    pMapJob->m_errorValue = 200;
    pMapJob->m_InterpolatorLabel = "Linear Interpolation";
    pMapJob->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;

    connect(pMapJob, SIGNAL(Error(QString)), this, SLOT(Error(QString)));
    connect(pMapJob, SIGNAL(MapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
      this, SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
      Qt::BlockingQueuedConnection);

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pMapJob);
  }

  this->StopSession();

  this->CheckInputs();
  this->ConfigureControls();

  auto* renderWindowPart = this->GetRenderWindowPart();

  if (nullptr != renderWindowPart)
    renderWindowPart->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
  const QmitkMappingJob* job)
{
  mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
    spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID,
    job->m_doGeometryRefinement, job->m_InterpolatorLabel);
  this->GetDataStorage()->Add(spMappedNode);

  auto* renderWindowPart = this->GetRenderWindowPart();

  if (nullptr != renderWindowPart)
    renderWindowPart->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnCenterTypeChanged(int index)
{
  ConfigureTransformCenter(index);

  if (this->m_EvalNode.IsNotNull())
  {
    this->m_EvalNode->Modified();
  }
  if (this->m_CurrentRegistrationWrapper.IsNotNull())
  {
    this->m_CurrentRegistrationWrapper->Modified();
  }

  auto* renderWindowPart = this->GetRenderWindowPart();

  if (nullptr != renderWindowPart)
    renderWindowPart->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::ConfigureTransformCenter(int centerType)
{
  mitk::Point3D center;

  if (centerType == 0)
  { //image center
    center = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    m_Controls->manipulationWidget->SetCenterOfRotationIsRelativeToTarget(false);
    m_Controls->manipulationWidget->SetCenterOfRotation(center);
  }
  else if (centerType == 1)
  { //world origin
    center.Fill(0.0);
    m_Controls->manipulationWidget->SetCenterOfRotationIsRelativeToTarget(false);
    m_Controls->manipulationWidget->SetCenterOfRotation(center);
  }
  else
  { //current selected point
    center = m_currentSelectedPosition;
    m_Controls->manipulationWidget->SetCenterOfRotationIsRelativeToTarget(true);
    m_Controls->manipulationWidget->SetCenterOfRotation(center);
  }

  if (m_Interactor.IsNotNull())
  {
    m_Interactor->SetCenterOfRotation(center);
  }
  this->UpdateCenterOfRotationIndicator();
}

void QmitkMatchPointRegistrationManipulator::OnInteractionToolToggled(bool checked)
{
  if (checked && m_activeManipulation)
  {
    this->ActivateInteractionTool();
  }
  else
  {
    this->DeactivateInteractionTool();
  }
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::OnScalingCheckboxToggled(bool checked)
{
  if (m_Interactor.IsNotNull())
  {
    m_Interactor->SetScalingEnabled(checked);
  }
}

void QmitkMatchPointRegistrationManipulator::ActivateInteractionTool()
{
  if (m_InteractionToolActive)
    return;

  auto* regModule = us::ModuleRegistry::GetModule("MitkMatchPointRegistration");
  if (nullptr == regModule)
    return;

  // Create and configure the interactor, attached to the moving data node
  m_Interactor = mitk::RegistrationManipulationInteractor::New();
  m_Interactor->LoadStateMachine("RegistrationManipulationStates.xml", regModule);
  m_Interactor->SetEventConfig("RegistrationManipulationConfig.xml", regModule);
  m_Interactor->SetDataNode(m_SelectedMovingNode);
  m_Interactor->SetScalingEnabled(m_Controls->checkScaling->isChecked());

  // Set current center of rotation
  this->ConfigureTransformCenter(m_Controls->comboCenter->currentIndex());

  // Connect ITK observer events to our handler methods
  auto translationCmd = itk::SimpleMemberCommand<QmitkMatchPointRegistrationManipulator>::New();
  translationCmd->SetCallbackFunction(this, &QmitkMatchPointRegistrationManipulator::OnInteractorTranslation);
  m_TranslationObserverTag = m_Interactor->AddObserver(mitk::RegistrationTranslationEvent(), translationCmd);

  auto rotationCmd = itk::SimpleMemberCommand<QmitkMatchPointRegistrationManipulator>::New();
  rotationCmd->SetCallbackFunction(this, &QmitkMatchPointRegistrationManipulator::OnInteractorRotation);
  m_RotationObserverTag = m_Interactor->AddObserver(mitk::RegistrationRotationEvent(), rotationCmd);

  auto scaleCmd = itk::SimpleMemberCommand<QmitkMatchPointRegistrationManipulator>::New();
  scaleCmd->SetCallbackFunction(this, &QmitkMatchPointRegistrationManipulator::OnInteractorScale);
  m_ScaleObserverTag = m_Interactor->AddObserver(mitk::RegistrationScaleEvent(), scaleCmd);

  auto selectPosCmd = itk::SimpleMemberCommand<QmitkMatchPointRegistrationManipulator>::New();
  selectPosCmd->SetCallbackFunction(this, &QmitkMatchPointRegistrationManipulator::OnInteractorSelectPosition);
  m_SelectPositionObserverTag = m_Interactor->AddObserver(mitk::RegistrationSelectPositionEvent(), selectPosCmd);

  // Block LMB display interactions (via DisplayConfigBlockLMB.xml) to prevent
  // conflict with our modifier+drag and plain-click gestures
  m_Interactor->DisableOriginalInteraction();

  m_InteractionToolActive = true;

  // Push the base manipulation cursor
  m_Interactor->PushManipulationCursor();

  // Setup center of rotation indicator
  this->UpdateCenterOfRotationIndicator();
}

void QmitkMatchPointRegistrationManipulator::DeactivateInteractionTool()
{
  if (!m_InteractionToolActive)
    return;

  // Disconnect observers and restore display interactions
  if (m_Interactor.IsNotNull())
  {
    m_Interactor->PopManipulationCursor();
    m_Interactor->RemoveObserver(m_TranslationObserverTag);
    m_Interactor->RemoveObserver(m_RotationObserverTag);
    m_Interactor->RemoveObserver(m_ScaleObserverTag);
    m_Interactor->RemoveObserver(m_SelectPositionObserverTag);
    m_Interactor->EnableOriginalInteraction();
    m_Interactor->SetDataNode(nullptr);
  }
  m_Interactor = nullptr;

  // Remove center indicator
  if (m_CenterOfRotationIndicatorNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(m_CenterOfRotationIndicatorNode);
  }
  m_CenterOfRotationIndicatorNode = nullptr;

  m_InteractionToolActive = false;
}

void QmitkMatchPointRegistrationManipulator::UpdateCenterOfRotationIndicator()
{
  if (!m_InteractionToolActive)
    return;

  const int centerMode = m_Controls->comboCenter->currentIndex();

  // Don't show indicator when using "Current navigator position" mode (already shown by crosshair)
  if (centerMode == 2)
  {
    if (m_CenterOfRotationIndicatorNode.IsNotNull() && this->GetDataStorage().IsNotNull())
    {
      this->GetDataStorage()->Remove(m_CenterOfRotationIndicatorNode);
      m_CenterOfRotationIndicatorNode = nullptr;
    }
    return;
  }

  // Determine center position
  mitk::Point3D center;
  if (centerMode == 0 && m_SelectedMovingNode.IsNotNull())
  {
    center = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
  }
  else
  {
    center.Fill(0.0);
  }

  // Create or update the indicator node
  if (m_CenterOfRotationIndicatorNode.IsNull())
  {
    auto crosshairData = mitk::CrosshairData::New();
    crosshairData->SetPosition(center);

    m_CenterOfRotationIndicatorNode = mitk::DataNode::New();
    m_CenterOfRotationIndicatorNode->SetData(crosshairData);
    m_CenterOfRotationIndicatorNode->SetName("RegistrationRotationCenter");
    m_CenterOfRotationIndicatorNode->SetBoolProperty("helper object", true);
    m_CenterOfRotationIndicatorNode->SetColor(1.0, 0.8, 0.0); // yellow/orange
    m_CenterOfRotationIndicatorNode->SetIntProperty("Crosshair.Gap Size", 16);
    m_CenterOfRotationIndicatorNode->SetFloatProperty("Line width", 2);
    this->GetDataStorage()->Add(m_CenterOfRotationIndicatorNode);
  }
  else
  {
    auto* crosshairData = dynamic_cast<mitk::CrosshairData*>(
      m_CenterOfRotationIndicatorNode->GetData());
    if (crosshairData != nullptr)
    {
      crosshairData->SetPosition(center);
      m_CenterOfRotationIndicatorNode->Modified();
    }
  }
}

void QmitkMatchPointRegistrationManipulator::OnInteractorTranslation()
{
  if (m_Interactor.IsNull())
    return;

  m_Controls->manipulationWidget->ApplyTranslationDelta(m_Interactor->GetTranslationDelta());
}

void QmitkMatchPointRegistrationManipulator::OnInteractorRotation()
{
  if (m_Interactor.IsNull())
    return;

  const auto& rotDelta = m_Interactor->GetRotationDelta();
  m_Controls->manipulationWidget->ApplyRotationDelta(rotDelta.Axis, rotDelta.AngleDeg);
}

void QmitkMatchPointRegistrationManipulator::OnInteractorScale()
{
  // Scaling not yet implemented in widget - reserved for Phase 2
  // Will call m_Controls->manipulationWidget->ApplyScaleDelta(m_Interactor->GetScaleFactor());
}

void QmitkMatchPointRegistrationManipulator::OnInteractorSelectPosition()
{
  if (m_Interactor.IsNull())
    return;

  auto* rwPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  if (rwPart != nullptr)
  {
    rwPart->SetSelectedPosition(m_Interactor->GetSelectPosition(), nullptr);
  }
}

