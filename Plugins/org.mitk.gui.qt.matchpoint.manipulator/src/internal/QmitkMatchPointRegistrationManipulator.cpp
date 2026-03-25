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
#include <mitkPointSet.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkSurface.h>
#include <mitkVtkRepresentationProperty.h>

#include <vtkCubeSource.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <usModuleRegistry.h>

#include <algorithm>

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
  connect(m_Controls->pbReinitMoving, &QPushButton::clicked, this, &QmitkMatchPointRegistrationManipulator::OnReinitMovingPerspective);
  connect(m_Controls->pbReinitTarget, &QPushButton::clicked, this, &QmitkMatchPointRegistrationManipulator::OnReinitTargetPerspective);
  connect(m_Controls->pbTranslateToNavigator, &QPushButton::clicked, this, &QmitkMatchPointRegistrationManipulator::OnTranslateMovingCenterToNavigator);
  connect(m_Controls->pbInteractionTool, SIGNAL(toggled(bool)), this, SLOT(OnInteractionToolToggled(bool)));
  connect(m_Controls->checkPreview3D, &QCheckBox::toggled, this, &QmitkMatchPointRegistrationManipulator::OnPreview3DToggled);
  connect(m_Controls->checkLockMovingPerspective, &QCheckBox::toggled, this, [this](bool checked) {
    if (m_activeManipulation)
    {
      if (checked)
        this->OnReinitMovingPerspective();
      else
        this->OnReinitTargetPerspective();
    }
  });

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
      MITK_DEBUG << "Stopped current MatchPoint manual registration session, because at least one relevant node was removed from storage.";
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
  m_Controls->checkRefineGeometry->setEnabled(m_activeManipulation);
  m_Controls->tabWidget->setEnabled(m_activeManipulation);
  m_Controls->pbCancel->setEnabled(m_activeManipulation);
  m_Controls->pbStore->setEnabled(m_activeManipulation);
  m_Controls->registrationNodeSelector->setEnabled(!m_activeManipulation && m_Controls->radioSelectedReg->isChecked());
  m_Controls->checkAutoSelect->setEnabled(!m_activeManipulation && m_Controls->radioSelectedReg->isChecked());
  m_Controls->movingNodeSelector->setEnabled(!m_activeManipulation);
  m_Controls->targetNodeSelector->setEnabled(!m_activeManipulation);

  // Interaction tool controls: only available during active manipulation
  m_Controls->pbInteractionTool->setEnabled(m_activeManipulation);
  m_Controls->lblInteractionInfo->setVisible(m_activeManipulation && m_InteractionToolActive);

  m_Controls->pbTranslateToNavigator->setEnabled(m_activeManipulation);
  m_Controls->pbReinitMoving->setEnabled(m_activeManipulation);
  m_Controls->pbReinitTarget->setEnabled(m_activeManipulation);
  m_Controls->checkPreview3D->setEnabled(m_activeManipulation);
  m_Controls->checkLockMovingPerspective->setEnabled(m_activeManipulation);

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

  // Mark session active before setting up the center indicator so
  // UpdateCenterOfRotationIndicator() can run when OnCenterTypeChanged fires.
  this->m_activeManipulation = true;

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

  if (m_Controls->checkPreview3D->isChecked())
  {
    this->Start3DPreview();
  }
}

void QmitkMatchPointRegistrationManipulator::StopSession()
{
  if (m_Controls->checkLockMovingPerspective->isChecked())
  {
    m_Controls->checkLockMovingPerspective->setChecked(false);
    this->OnReinitTargetPerspective();
  }

  this->Stop3DPreview();

  if (m_InteractionToolActive)
  {
    this->DeactivateInteractionTool();
  }

  // Remove center indicator independently of interaction tool state
  // (indicator is now shown whenever session is active, not only when tool is active)
  if (m_CenterOfRotationIndicatorNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(m_CenterOfRotationIndicatorNode);
  }
  m_CenterOfRotationIndicatorNode = nullptr;

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

  if (m_3DPreviewActive)
  {
    this->Update3DPreviewGeometry();
  }

  if (m_Controls->checkLockMovingPerspective->isChecked())
  {
    this->OnReinitMovingPerspective();
  }

  // Update indicator position: for "Moving image center" mode the center moves with the image
  this->UpdateCenterOfRotationIndicator();

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
      this->UpdateCenterOfRotationIndicator();
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

  if (m_Controls->checkRefineGeometry->checkState() == Qt::Checked)
  {
    const auto* movingImage =
      dynamic_cast<const mitk::Image*>(this->m_SelectedMovingNode->GetData());

    if (movingImage != nullptr &&
        mitk::ImageMappingHelper::canRefineGeometry(newRegWrapper.GetPointer()))
    {
      try
      {
        mitk::ImageMappingHelper::ResultImageType::Pointer refinedImage =
          mitk::ImageMappingHelper::refineGeometry(movingImage, newRegWrapper.GetPointer(), true);

        const std::string refinedName =
          m_Controls->lbNewRegName->text().toStdString() + std::string(" refined geometry");

        mitk::DataNode::Pointer spRefinedNode = mitk::generateMappedResultNode(
          refinedName, refinedImage,
          newReg->getRegistrationUID(),
          mitk::EnsureUID(m_SelectedMovingNode->GetData()),
          true, "geometry refinement");

        this->GetDataStorage()->Add(spRefinedNode);
      }
      catch (const std::exception& e)
      {
        this->Error(QString("Geometry refinement failed: ") + QString::fromStdString(e.what()));
      }
    }
    else if (movingImage != nullptr)
    {
      this->Error(QString("Cannot refine geometry: registration does not support geometry refinement (requires affine transform)."));
    }
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

  // Set current center of rotation
  this->ConfigureTransformCenter(m_Controls->comboCenter->currentIndex());

  // Connect ITK observer events to our handler methods
  auto translationCmd = itk::SimpleMemberCommand<QmitkMatchPointRegistrationManipulator>::New();
  translationCmd->SetCallbackFunction(this, &QmitkMatchPointRegistrationManipulator::OnInteractorTranslation);
  m_TranslationObserverTag = m_Interactor->AddObserver(mitk::RegistrationTranslationEvent(), translationCmd);

  auto rotationCmd = itk::SimpleMemberCommand<QmitkMatchPointRegistrationManipulator>::New();
  rotationCmd->SetCallbackFunction(this, &QmitkMatchPointRegistrationManipulator::OnInteractorRotation);
  m_RotationObserverTag = m_Interactor->AddObserver(mitk::RegistrationRotationEvent(), rotationCmd);

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
  if (!m_activeManipulation)
    return;

  const int centerMode = m_Controls->comboCenter->currentIndex();

  // Determine center position in target/world space.
  mitk::Point3D center;
  center.Fill(0.0);

  if (centerMode == 0 && m_SelectedMovingNode.IsNotNull())
  {
    center = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();

    // Apply current direct (moving->target) registration transform to show the current position.
    if (m_CurrentRegistrationWrapper.IsNotNull())
    {
      const auto inverseAffine =
        mitk::MITKRegistrationHelper::getAffineMatrix(m_CurrentRegistrationWrapper, true);
      if (inverseAffine.IsNotNull())
      {
        mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer directTransform =
          mitk::MITKRegistrationHelper::Affine3DTransformType::New();
        if (inverseAffine->GetInverse(directTransform))
        {
          center = directTransform->TransformPoint(center);
        }
      }
    }
  }
  else if (centerMode == 1)
  {
    // World origin — center is already (0,0,0).
  }
  else if (centerMode == 2)
  {
    center = m_currentSelectedPosition;
  }

  // Scale indicator size relative to the moving image extent so it remains
  // visible but does not dominate small images (e.g. 2 cm field of view).
  float indicatorSize = 8.0f;
  if (m_SelectedMovingNode.IsNotNull())
  {
    const auto* geometry = m_SelectedMovingNode->GetData()->GetGeometry();
    const double extX = geometry->GetExtentInMM(0);
    const double extY = geometry->GetExtentInMM(1);
    const double extZ = geometry->GetExtentInMM(2);
    const double minExtent = std::min({extX, extY, extZ});
    // Use ~5% of the smallest extent, clamped to a reasonable range.
    indicatorSize = static_cast<float>(std::clamp(minExtent * 0.05, 1.0, 20.0));
  }

  // Create or update the indicator as a PointSet glyph.
  if (m_CenterOfRotationIndicatorNode.IsNull())
  {
    auto pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(0, center);

    m_CenterOfRotationIndicatorNode = mitk::DataNode::New();
    m_CenterOfRotationIndicatorNode->SetData(pointSet);
    m_CenterOfRotationIndicatorNode->SetName("RegistrationRotationCenter");
    m_CenterOfRotationIndicatorNode->SetBoolProperty("helper object", true);
    m_CenterOfRotationIndicatorNode->SetColor(1.0, 0.8, 0.0);
    m_CenterOfRotationIndicatorNode->SetFloatProperty("pointsize", indicatorSize);
    m_CenterOfRotationIndicatorNode->SetFloatProperty("point 2D size", indicatorSize);
    m_CenterOfRotationIndicatorNode->SetProperty("Pointset.2D.shape",
      mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::DIAMOND));
    m_CenterOfRotationIndicatorNode->SetBoolProperty("Pointset.2D.fill shape", true);
    m_CenterOfRotationIndicatorNode->SetIntProperty("point line width", 2);
    this->GetDataStorage()->Add(m_CenterOfRotationIndicatorNode);
  }
  else
  {
    auto* pointSet = dynamic_cast<mitk::PointSet*>(m_CenterOfRotationIndicatorNode->GetData());
    if (pointSet != nullptr)
    {
      pointSet->SetPoint(0, center);
      m_CenterOfRotationIndicatorNode->Modified();
    }
    m_CenterOfRotationIndicatorNode->SetFloatProperty("pointsize", indicatorSize);
    m_CenterOfRotationIndicatorNode->SetFloatProperty("point 2D size", indicatorSize);
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

void QmitkMatchPointRegistrationManipulator::OnReinitMovingPerspective()
{
  if (!m_activeManipulation || m_SelectedMovingNode.IsNull() || m_CurrentRegistrationWrapper.IsNull())
    return;

  const auto* movingData = m_SelectedMovingNode->GetData();
  if (nullptr == movingData)
    return;

  // Clone the moving image's TimeGeometry so we don't modify the original
  auto composedTimeGeometry = movingData->GetTimeGeometry()->Clone();

  // Get the direct (moving->target) transform from the interim registration
  const auto inverseAffine =
    mitk::MITKRegistrationHelper::getAffineMatrix(m_CurrentRegistrationWrapper, true);
  if (inverseAffine.IsNull())
    return;

  mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer directTransform =
    mitk::MITKRegistrationHelper::Affine3DTransformType::New();
  if (!inverseAffine->GetInverse(directTransform))
    return;

  // Compose the registration transform onto each timestep geometry
  for (unsigned int i = 0; i < composedTimeGeometry->CountTimeSteps(); ++i)
  {
    composedTimeGeometry->GetGeometryForTimeStep(i)->Compose(directTransform);
  }
  composedTimeGeometry->Update();

  mitk::RenderingManager::GetInstance()->InitializeViews(composedTimeGeometry.GetPointer());
}

void QmitkMatchPointRegistrationManipulator::OnReinitTargetPerspective()
{
  if (!m_activeManipulation || m_SelectedTargetNode.IsNull())
    return;

  const auto* targetData = m_SelectedTargetNode->GetData();
  if (nullptr == targetData)
    return;

  mitk::RenderingManager::GetInstance()->InitializeViews(targetData->GetTimeGeometry());
}

void QmitkMatchPointRegistrationManipulator::OnTranslateMovingCenterToNavigator()
{
  if (!m_activeManipulation || m_SelectedMovingNode.IsNull() || m_CurrentRegistrationWrapper.IsNull())
    return;

  // Original moving image center in world space (before any registration transform).
  const auto movingCenter = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();

  // Determine where the moving image center currently appears in world/target space by
  // applying the full direct (moving->target) interim registration transform to it.
  // The interim registration only has a valid inverse kernel; invert it to get the direct one.
  mitk::Point3D currentTransformedCenter = movingCenter;

  const auto inverseAffine =
    mitk::MITKRegistrationHelper::getAffineMatrix(m_CurrentRegistrationWrapper, true);
  if (inverseAffine.IsNotNull())
  {
    mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer directTransform =
      mitk::MITKRegistrationHelper::Affine3DTransformType::New();
    if (inverseAffine->GetInverse(directTransform))
    {
      currentTransformedCenter = directTransform->TransformPoint(movingCenter);
    }
  }

  // The required translation delta shifts the moving center to the current navigator position.
  mitk::Vector3D delta;
  delta[0] = m_currentSelectedPosition[0] - currentTransformedCenter[0];
  delta[1] = m_currentSelectedPosition[1] - currentTransformedCenter[1];
  delta[2] = m_currentSelectedPosition[2] - currentTransformedCenter[2];

  m_Controls->manipulationWidget->ApplyTranslationDelta(delta);
}

void QmitkMatchPointRegistrationManipulator::OnPreview3DToggled(bool checked)
{
  if (checked && m_activeManipulation)
  {
    this->Start3DPreview();
  }
  else
  {
    this->Stop3DPreview();
  }
}

void QmitkMatchPointRegistrationManipulator::Start3DPreview()
{
  if (m_3DPreviewActive || !m_activeManipulation || m_SelectedMovingNode.IsNull())
    return;

  auto* movingData = m_SelectedMovingNode->GetData();
  if (nullptr == movingData)
    return;

  auto clonedObj = movingData->Clone();
  mitk::BaseData::Pointer clonedData = dynamic_cast<mitk::BaseData*>(clonedObj.GetPointer());
  if (clonedData.IsNull())
    return;

  m_OriginalMovingVtkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  m_OriginalMovingVtkMatrix->DeepCopy(clonedData->GetGeometry()->GetVtkMatrix());

  // The clone must remain visible (ImageVtkMapper2D::Update skips invisible nodes,
  // which would break the 3D cut-plane texture via PlaneGeometryDataVtkMapper3D).
  m_3DPreviewCloneNode = mitk::DataNode::New();
  m_3DPreviewCloneNode->SetData(clonedData);
  m_3DPreviewCloneNode->SetName("RegistrationManipulation3DPreview");
  m_3DPreviewCloneNode->SetBoolProperty("helper object", true);

  // Wireframe bounding box — Surface mappers have no cross-mapper dependency,
  // so it can simply be hidden per 2D renderer.
  mitk::BaseGeometry* geom = clonedData->GetGeometry();
  const auto bounds = geom->GetBounds();

  // MITK's index-to-world transform places the origin at voxel-center 0.
  // The physical image extent therefore runs from index -0.5 to N-0.5 per dimension.
  // Subtracting 0.5 from each bound aligns the wireframe with the actual pixel corners.
  auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
  cubeSource->SetBounds(bounds[0] - 0.5, bounds[1] - 0.5,
                        bounds[2] - 0.5, bounds[3] - 0.5,
                        bounds[4] - 0.5, bounds[5] - 0.5);
  cubeSource->Update();

  auto wireframeSurface = mitk::Surface::New();
  wireframeSurface->SetVtkPolyData(cubeSource->GetOutput());
  wireframeSurface->SetGeometry(geom->Clone());

  m_3DPreviewWireframeNode = mitk::DataNode::New();
  m_3DPreviewWireframeNode->SetData(wireframeSurface);
  m_3DPreviewWireframeNode->SetName("RegistrationManipulation3DWireframe");
  m_3DPreviewWireframeNode->SetBoolProperty("helper object", true);
  m_3DPreviewWireframeNode->SetColor(0.0, 1.0, 0.0);
  m_3DPreviewWireframeNode->SetFloatProperty("material.wireframeLineWidth", 2.0f);

  auto reprProp = mitk::VtkRepresentationProperty::New();
  reprProp->SetRepresentationToWireframe();
  m_3DPreviewWireframeNode->SetProperty("material.representation", reprProp);

  this->Update3DPreviewGeometry();

  this->GetDataStorage()->Add(m_3DPreviewCloneNode);
  this->GetDataStorage()->Add(m_3DPreviewWireframeNode);

  // After adding to storage, the DataManager auto-assigns a high layer to each node.
  // Explicitly order layers so that: moving/target < clone < eval node.
  // This ensures the clone is covered by the eval node in 2D views, but still
  // sits above the original moving and target images.
  int movingLayer = 0;
  if (m_SelectedMovingNode.IsNotNull())
  {
    m_SelectedMovingNode->GetIntProperty("layer", movingLayer);
  }
  int targetLayer = 0;
  if (m_SelectedTargetNode.IsNotNull())
  {
    m_SelectedTargetNode->GetIntProperty("layer", targetLayer);
  }
  const int cloneLayer = std::max(movingLayer, targetLayer) + 1;
  m_3DPreviewCloneNode->SetIntProperty("layer", cloneLayer);

  // Ensure the eval node is strictly above the clone.
  int evalNodeLayer = 0;
  if (m_EvalNode.IsNotNull())
  {
    m_EvalNode->GetIntProperty("layer", evalNodeLayer);
    if (evalNodeLayer <= cloneLayer)
    {
      m_EvalNode->SetIntProperty("layer", cloneLayer + 1);
    }
  }

  auto* renderWindowPart = this->GetRenderWindowPart();
  if (renderWindowPart != nullptr)
  {
    const QStringList planeIDs = {"axial", "sagittal", "coronal"};
    for (const auto& id : planeIDs)
    {
      auto* renderWindow = renderWindowPart->GetQmitkRenderWindow(id);
      if (renderWindow != nullptr)
      {
        m_3DPreviewWireframeNode->SetVisibility(false, renderWindow->GetRenderer());
      }
    }
  }

  m_3DPreviewActive = true;
}

void QmitkMatchPointRegistrationManipulator::Stop3DPreview()
{
  if (!m_3DPreviewActive)
    return;

  if (m_3DPreviewCloneNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(m_3DPreviewCloneNode);
  }
  if (m_3DPreviewWireframeNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(m_3DPreviewWireframeNode);
  }

  m_3DPreviewCloneNode = nullptr;
  m_3DPreviewWireframeNode = nullptr;
  m_OriginalMovingVtkMatrix = nullptr;
  m_3DPreviewActive = false;
}

void QmitkMatchPointRegistrationManipulator::Update3DPreviewGeometry()
{
  if (m_CurrentRegistrationWrapper.IsNull() || m_OriginalMovingVtkMatrix == nullptr)
    return;

  // The interim registration only has a valid inverse kernel (direct is NullRegistrationKernel).
  // Get the inverse kernel (target->moving) and invert to obtain the direct (moving->target) transform.
  const auto inverseAffine =
    mitk::MITKRegistrationHelper::getAffineMatrix(m_CurrentRegistrationWrapper, true);
  if (inverseAffine.IsNull())
    return;

  mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer directTransform =
    mitk::MITKRegistrationHelper::Affine3DTransformType::New();
  if (!inverseAffine->GetInverse(directTransform))
    return;

  // Follow the refineGeometry pattern: reset geometry to original, compose with
  // registration transform for each time step, then update the time geometry.
  if (m_3DPreviewCloneNode.IsNotNull() && m_3DPreviewCloneNode->GetData() != nullptr)
  {
    auto* data = m_3DPreviewCloneNode->GetData();
    for (unsigned int i = 0; i < data->GetTimeSteps(); ++i)
    {
      data->GetGeometry(i)->SetIndexToWorldTransformByVtkMatrix(m_OriginalMovingVtkMatrix);
      data->GetGeometry(i)->Compose(directTransform);
    }
    data->GetTimeGeometry()->Update();
  }

  if (m_3DPreviewWireframeNode.IsNotNull() && m_3DPreviewWireframeNode->GetData() != nullptr)
  {
    auto* data = m_3DPreviewWireframeNode->GetData();
    for (unsigned int i = 0; i < data->GetTimeSteps(); ++i)
    {
      data->GetGeometry(i)->SetIndexToWorldTransformByVtkMatrix(m_OriginalMovingVtkMatrix);
      data->GetGeometry(i)->Compose(directTransform);
    }
    data->GetTimeGeometry()->Update();
  }
}

