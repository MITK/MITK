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

// Mitk
#include <mitkStatusBar.h>
#include <mitkNodePredicateDataProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkMAPRegistrationWrapper.h>
#include "mitkRegVisPropertyTags.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegistrationHelper.h"
#include "mitkRegEvaluationMapper2D.h"
#include <mitkAlgorithmHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkUIDHelper.h>

// Qmitk
#include "QmitkRenderWindow.h"
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

const std::string QmitkMatchPointRegistrationManipulator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.manipulator";

const std::string QmitkMatchPointRegistrationManipulator::HelperNodeName =
    "RegistrationManipulationEvaluationHelper";

QmitkMatchPointRegistrationManipulator::QmitkMatchPointRegistrationManipulator()
  : m_Parent(nullptr), m_activeManipulation(false), m_autoMoving(false), m_autoTarget(false),
    m_currentSelectedTimeStep(0), m_internalUpdate(false)
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
	m_Controls.setupUi(parent);

	m_Parent = parent;

  connect(m_Controls.pbStart, SIGNAL(clicked()), this, SLOT(OnStartBtnPushed()));
  connect(m_Controls.pbCancel, SIGNAL(clicked()), this, SLOT(OnCancelBtnPushed()));
  connect(m_Controls.pbStore, SIGNAL(clicked()), this, SLOT(OnStoreBtnPushed()));
  connect(m_Controls.evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));
  connect(m_Controls.radioSelectedReg, SIGNAL(toggled(bool)), m_Controls.lbRegistrationName, SLOT(setVisible(bool)));
  connect(m_Controls.radioSelectedReg, SIGNAL(toggled(bool)), this, SLOT(OnSelectionChanged()));

  connect(m_Controls.comboCenter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCenterTypeChanged(int)));
  connect(m_Controls.manipulationWidget, SIGNAL(RegistrationChanged(map::core::RegistrationBase*)), this, SLOT(OnRegistrationChanged()));

  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

  m_Controls.radioNewReg->setChecked(true);
  m_Controls.lbRegistrationName->setVisible(false);

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

void QmitkMatchPointRegistrationManipulator::CheckInputs()
{
  if (!m_activeManipulation)
  {
  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
  this->m_autoMoving = false;
  this->m_autoTarget = false;
  this->m_SelectedMovingNode = nullptr;
  this->m_SelectedTargetNode = nullptr;
  this->m_SelectedPreRegNode = nullptr;
  this->m_SelectedPreReg = nullptr;

  if (dataNodes.size() > 0)
  {
    //test if auto select works
    if (mitk::MITKRegistrationHelper::IsRegNode(dataNodes[0]))
    {
      mitk::DataNode::Pointer regNode = dataNodes[0];
      dataNodes.pop_front();

      mitk::MAPRegistrationWrapper* regWrapper = dynamic_cast<mitk::MAPRegistrationWrapper*>(regNode->GetData());
      if (regWrapper)
      {
        this->m_SelectedPreReg = dynamic_cast<MAPRegistrationType*>(regWrapper->GetRegistration());
      }

      if (this->m_SelectedPreReg.IsNotNull())
      {
        this->m_SelectedPreRegNode = regNode;

        mitk::BaseProperty* uidProp = m_SelectedPreRegNode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);

        if (uidProp)
        {
          //search for the moving node
          mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
            uidProp);
          this->m_SelectedMovingNode = this->GetDataStorage()->GetNode(predicate);
          this->m_autoMoving = this->m_SelectedMovingNode.IsNotNull();
        }

        uidProp = m_SelectedPreRegNode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);

        if (uidProp)
        {
          //search for the target node
          mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
            uidProp);
          this->m_SelectedTargetNode = this->GetDataStorage()->GetNode(predicate);
          this->m_autoTarget = this->m_SelectedTargetNode.IsNotNull();
        }
      }
    }

    //if still nodes are selected -> ignore possible auto select
    if (!dataNodes.empty())
    {
      mitk::Image* inputImage = dynamic_cast<mitk::Image*>(dataNodes[0]->GetData());

      if (inputImage)
      {
        this->m_SelectedMovingNode = dataNodes[0];
        this->m_autoMoving = false;
        dataNodes.pop_front();
      }
    }

    if (!dataNodes.empty())
    {
      mitk::Image* inputImage = dynamic_cast<mitk::Image*>(dataNodes[0]->GetData());

      if (inputImage)
      {
        this->m_SelectedTargetNode = dataNodes[0];
        this->m_autoTarget = false;
        dataNodes.pop_front();
      }
    }
  }
  }
}

void QmitkMatchPointRegistrationManipulator::OnSelectionChanged()
{
  this->CheckInputs();
  this->ConfigureControls();
};

void QmitkMatchPointRegistrationManipulator::OnSelectionChanged(berry::IWorkbenchPart::Pointer,
        const QList<mitk::DataNode::Pointer>&)
{
  this->OnSelectionChanged();
};


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
  //configure input data widgets
  if (this->m_SelectedPreRegNode.IsNull())
  {
    m_Controls.lbRegistrationName->setText(
        QString("<font color='red'>No registration selected!</font>"));
  }
  else
  {
    m_Controls.lbRegistrationName->setText(QString::fromStdString(
      this->m_SelectedPreRegNode->GetName()));
  }

  if (this->m_SelectedMovingNode.IsNull())
  {
    m_Controls.lbMovingName->setText(QString("<font color='red'>no moving image selected!</font>"));
  }
  else
  {
    if (this->m_autoMoving)
    {
      m_Controls.lbMovingName->setText(QString("<font color='gray'>") + QString::fromStdString(
        this->m_SelectedMovingNode->GetName()) + QString(" (auto selected)</font>"));
    }
    else
    {
      m_Controls.lbMovingName->setText(QString::fromStdString(this->m_SelectedMovingNode->GetName()));
    }
  }

  if (this->m_SelectedTargetNode.IsNull())
  {
    m_Controls.lbTargetName->setText(QString("<font color='red'>no target image selected!</font>"));
  }
  else
  {
    if (this->m_autoTarget)
    {
      m_Controls.lbTargetName->setText(QString("<font color='gray'>") + QString::fromStdString(
        this->m_SelectedTargetNode->GetName()) + QString(" (auto selected)</font>"));
    }
    else
    {
      m_Controls.lbTargetName->setText(QString::fromStdString(this->m_SelectedTargetNode->GetName()));
    }
  }

  if (!m_activeManipulation)
  {
    QString name = "ManuelRegistration";

    if (m_SelectedPreRegNode.IsNotNull())
    {
      name = QString::fromStdString(m_SelectedPreRegNode->GetName()) + " manual refined";
    }
    this->m_Controls.lbNewRegName->setText(name);
  }

  //config settings widget
  this->m_Controls.groupReg->setEnabled(!m_activeManipulation);

  this->m_Controls.lbMovingName->setEnabled(!m_activeManipulation);
  this->m_Controls.lbTargetName->setEnabled(!m_activeManipulation);

  this->m_Controls.pbStart->setEnabled(this->m_SelectedMovingNode.IsNotNull()
    && this->m_SelectedTargetNode.IsNotNull()
    && !m_activeManipulation
    && (this->m_Controls.radioNewReg->isChecked() || this->m_SelectedPreReg.IsNotNull()));

  this->m_Controls.lbNewRegName->setEnabled(m_activeManipulation);
  this->m_Controls.checkMapEntity->setEnabled(m_activeManipulation);
  this->m_Controls.tabWidget->setEnabled(m_activeManipulation);
  this->m_Controls.pbCancel->setEnabled(m_activeManipulation);
  this->m_Controls.pbStore->setEnabled(m_activeManipulation);
}

void QmitkMatchPointRegistrationManipulator::InitSession()
{
  if (this->m_Controls.radioNewReg->isChecked())
  { //init to map the image centers
    auto movingCenter = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    auto targetCenter = m_SelectedTargetNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    this->m_Controls.manipulationWidget->Initialize(movingCenter, targetCenter);
  }
  else
  { //use selected pre registration as baseline
    m_Controls.manipulationWidget->Initialize(m_SelectedPreReg);
  }

  this->m_CurrentRegistrationWrapper = mitk::MAPRegistrationWrapper::New();
  this->m_CurrentRegistration = m_Controls.manipulationWidget->GetInterimRegistration();
  this->m_CurrentRegistrationWrapper->SetRegistration(m_CurrentRegistration);

  this->m_Controls.comboCenter->setCurrentIndex(0);
  this->OnCenterTypeChanged(0);

  //reinit view
  mitk::RenderingManager::GetInstance()->InitializeViews(m_SelectedTargetNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

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

  this->m_Controls.evalSettings->SetNode(this->m_EvalNode);

  this->m_activeManipulation = true;
};

void QmitkMatchPointRegistrationManipulator::StopSession()
{
  this->m_activeManipulation = false;

  if (this->m_EvalNode.IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_EvalNode);
  }

  this->m_EvalNode = nullptr;

  this->m_CurrentRegistration = nullptr;
  this->m_CurrentRegistrationWrapper = nullptr;
  m_Controls.manipulationWidget->Initialize();
};


void QmitkMatchPointRegistrationManipulator::OnRegistrationChanged()
{
  this->m_EvalNode->Modified();
  this->m_CurrentRegistrationWrapper->Modified();
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::OnSliceChanged()
{
  mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(nullptr);
  unsigned int currentSelectedTimeStep = GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

  if (m_currentSelectedPosition != currentSelectedPosition
    || m_currentSelectedTimeStep != currentSelectedTimeStep
    || m_selectedNodeTime > m_currentPositionTime)
  {
    //the current position has been changed or the selected node has been changed since the last position validation -> check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimeStep = currentSelectedTimeStep;
    m_currentPositionTime.Modified();

    if (this->m_EvalNode.IsNotNull())
    {
      this->m_EvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::GenericProperty<mitk::Point3D>::New(currentSelectedPosition));
    }

    if (m_activeManipulation && m_Controls.comboCenter->currentIndex() == 2)
    { //update transform with the current position.
      m_Controls.manipulationWidget->SetCenterOfRotation(m_currentSelectedPosition);
    }
  }
}

void QmitkMatchPointRegistrationManipulator::OnSettingsChanged(mitk::DataNode*)
{
	this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::OnStartBtnPushed()
{
  this->InitSession();
  this->OnSliceChanged();

  this->GetRenderWindowPart()->RequestUpdate();

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
  mitk::MAPRegistrationWrapper::Pointer newRegWrapper = mitk::MAPRegistrationWrapper::New();
  map::core::RegistrationBase::Pointer newReg = this->m_Controls.manipulationWidget->GenerateRegistration();

  newRegWrapper->SetRegistration(newReg);

  mitk::DataNode::Pointer spResultRegistrationNode = mitk::generateRegistrationResultNode(
    this->m_Controls.lbNewRegName->text().toStdString(), newRegWrapper, "org.mitk::manual_registration",
    mitk::EnsureUID(m_SelectedMovingNode->GetData()), mitk::EnsureUID(m_SelectedTargetNode->GetData()));

  this->GetDataStorage()->Add(spResultRegistrationNode);

  if (m_Controls.checkMapEntity->checkState() == Qt::Checked)
  {
    QmitkMappingJob* pMapJob = new QmitkMappingJob();
    pMapJob->setAutoDelete(true);

    pMapJob->m_spInputData = this->m_SelectedMovingNode->GetData();
    pMapJob->m_InputDataUID = mitk::EnsureUID(m_SelectedMovingNode->GetData());
    pMapJob->m_spRegNode = spResultRegistrationNode;
    pMapJob->m_doGeometryRefinement = false;
    pMapJob->m_spRefGeometry = this->m_SelectedTargetNode->GetData()->GetGeometry()->Clone().GetPointer();

    pMapJob->m_MappedName = this->m_Controls.lbNewRegName->text().toStdString() + std::string(" mapped moving data");
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
  this->GetRenderWindowPart()->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
  const QmitkMappingJob* job)
{
  mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
    spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID,
    job->m_doGeometryRefinement, job->m_InterpolatorLabel);
  this->GetDataStorage()->Add(spMappedNode);
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::OnCenterTypeChanged(int index)
{
  ConfigureTransformCenter(index);

  if (this->m_EvalNode.IsNotNull())
  {
    this->m_EvalNode->Modified();
  }
  this->m_CurrentRegistrationWrapper->Modified();
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::ConfigureTransformCenter(int centerType)
{
  if (centerType == 0)
  { //image center
    auto center = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    m_Controls.manipulationWidget->SetCenterOfRotationIsRelativeToTarget(false);
    m_Controls.manipulationWidget->SetCenterOfRotation(center);
  }
  else if (centerType == 1)
  { //world origin
    mitk::Point3D center;
    center.Fill(0.0);
    m_Controls.manipulationWidget->SetCenterOfRotationIsRelativeToTarget(false);
    m_Controls.manipulationWidget->SetCenterOfRotation(center);
  }
  else
  { //current selected point
    m_Controls.manipulationWidget->SetCenterOfRotationIsRelativeToTarget(true);
    m_Controls.manipulationWidget->SetCenterOfRotation(m_currentSelectedPosition);
  }
};
