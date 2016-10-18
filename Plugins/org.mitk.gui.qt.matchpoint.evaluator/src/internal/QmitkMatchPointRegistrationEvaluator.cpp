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
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkMAPRegistrationWrapper.h>
#include "mitkRegVisPropertyTags.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegistrationHelper.h"
#include "mitkRegEvaluationMapper2D.h"
#include <mitkAlgorithmHelper.h>

// Qmitk
#include "QmitkRenderWindow.h"
#include "QmitkMatchPointRegistrationEvaluator.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>


const std::string QmitkMatchPointRegistrationEvaluator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.evaluator";

QmitkMatchPointRegistrationEvaluator::QmitkMatchPointRegistrationEvaluator()
  : m_Parent(NULL), m_activeEvaluation(false), m_autoMoving(false), m_autoTarget(false), m_currentSelectedTimeStep(0), HelperNodeName("RegistrationEvaluationHelper")
{
  m_currentSelectedPosition.Fill(0.0);
}

QmitkMatchPointRegistrationEvaluator::~QmitkMatchPointRegistrationEvaluator()
{
  if (this->m_selectedEvalNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_selectedEvalNode);
  }
}

void QmitkMatchPointRegistrationEvaluator::SetFocus()
{

}

void QmitkMatchPointRegistrationEvaluator::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();
}

void QmitkMatchPointRegistrationEvaluator::CreateQtPartControl(QWidget* parent)
{
	// create GUI widgets from the Qt Designer's .ui file
	m_Controls.setupUi(parent);

	m_Parent = parent;

  connect(m_Controls.pbEval, SIGNAL(clicked()), this, SLOT(OnEvalBtnPushed()));
  connect(m_Controls.pbStop, SIGNAL(clicked()), this, SLOT(OnStopBtnPushed()));
  connect(m_Controls.evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));

  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

  m_selectedEvalNode = this->GetDataStorage()->GetNamedNode(HelperNodeName);

  this->CheckInputs();
	this->ConfigureControls();
}

void QmitkMatchPointRegistrationEvaluator::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkMatchPointRegistrationEvaluator::RenderWindowPartDeactivated(
  mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkMatchPointRegistrationEvaluator::CheckInputs()
{
  if (!m_activeEvaluation)
  {
  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
  this->m_autoMoving = false;
  this->m_autoTarget = false;
  this->m_spSelectedMovingNode = NULL;
  this->m_spSelectedTargetNode = NULL;
  this->m_spSelectedRegNode = NULL;

  if (dataNodes.size() > 0)
  {
    //test if auto select works
    if (mitk::MITKRegistrationHelper::IsRegNode(dataNodes[0]))
    {
      this->m_spSelectedRegNode = dataNodes[0];
      dataNodes.pop_front();

      mitk::BaseProperty* uidProp = m_spSelectedRegNode->GetProperty(mitk::nodeProp_RegAlgMovingData);

      if (uidProp)
      {
        //search for the moving node
        mitk::NodePredicateProperty::Pointer predicate = mitk::NodePredicateProperty::New(mitk::nodeProp_UID,
          uidProp);
        this->m_spSelectedMovingNode = this->GetDataStorage()->GetNode(predicate);
        this->m_autoMoving = this->m_spSelectedMovingNode.IsNotNull();
      }

      uidProp = m_spSelectedRegNode->GetProperty(mitk::nodeProp_RegAlgTargetData);

      if (uidProp)
      {
        //search for the target node
        mitk::NodePredicateProperty::Pointer predicate = mitk::NodePredicateProperty::New(mitk::nodeProp_UID,
          uidProp);
        this->m_spSelectedTargetNode = this->GetDataStorage()->GetNode(predicate);
        this->m_autoTarget = this->m_spSelectedTargetNode.IsNotNull();
      }
    }

    //if still nodes are selected -> ignore possible auto select
    if (!dataNodes.empty())
    {
      mitk::Image* inputImage = dynamic_cast<mitk::Image*>(dataNodes[0]->GetData());

      if (inputImage)
      {
        this->m_spSelectedMovingNode = dataNodes[0];
        this->m_autoMoving = false;
        dataNodes.pop_front();
      }
    }

    if (!dataNodes.empty())
    {
      mitk::Image* inputImage = dynamic_cast<mitk::Image*>(dataNodes[0]->GetData());

      if (inputImage)
      {
        this->m_spSelectedTargetNode = dataNodes[0];
        this->m_autoTarget = false;
        dataNodes.pop_front();
      }
    }
  }
  }
}


void QmitkMatchPointRegistrationEvaluator::OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
        const QList<mitk::DataNode::Pointer>& nodes)
{
  this->CheckInputs();
	this->ConfigureControls();
};

void QmitkMatchPointRegistrationEvaluator::ConfigureControls()
{
  //configure input data widgets
  if (this->m_spSelectedRegNode.IsNull())
  {
    if (this->m_spSelectedMovingNode.IsNotNull() && this->m_spSelectedTargetNode.IsNotNull())
    {
      m_Controls.lbRegistrationName->setText(
        QString("<font color='gray'>No registration selected! Direct comparison</font>"));
    }
    else
    {
      m_Controls.lbRegistrationName->setText(
        QString("<font color='red'>No registration selected!</font>"));
    }
  }
  else
  {
    m_Controls.lbRegistrationName->setText(QString::fromStdString(
      this->m_spSelectedRegNode->GetName()));
  }

  if (this->m_spSelectedMovingNode.IsNull())
  {
    m_Controls.lbMovingName->setText(QString("<font color='red'>no moving image selected!</font>"));
  }
  else
  {
    if (this->m_autoMoving)
    {
      m_Controls.lbMovingName->setText(QString("<font color='gray'>") + QString::fromStdString(
        this->m_spSelectedMovingNode->GetName()) + QString(" (auto selected)</font>"));
    }
    else
    {
      m_Controls.lbMovingName->setText(QString::fromStdString(this->m_spSelectedMovingNode->GetName()));
    }
  }

  if (this->m_spSelectedTargetNode.IsNull())
  {
    m_Controls.lbTargetName->setText(QString("<font color='red'>no target image selected!</font>"));
  }
  else
  {
    if (this->m_autoTarget)
    {
      m_Controls.lbTargetName->setText(QString("<font color='gray'>") + QString::fromStdString(
        this->m_spSelectedTargetNode->GetName()) + QString(" (auto selected)</font>"));
    }
    else
    {
      m_Controls.lbTargetName->setText(QString::fromStdString(this->m_spSelectedTargetNode->GetName()));
    }
  }

  //config settings widget
  this->m_Controls.evalSettings->setVisible(m_activeEvaluation);
  this->m_Controls.pbEval->setEnabled(this->m_spSelectedMovingNode.IsNotNull()
    && this->m_spSelectedTargetNode.IsNotNull());
  this->m_Controls.pbEval->setVisible(!m_activeEvaluation);
  this->m_Controls.pbStop->setVisible(m_activeEvaluation);
  this->m_Controls.lbMovingName->setEnabled(!m_activeEvaluation);
  this->m_Controls.lbRegistrationName->setEnabled(!m_activeEvaluation);
  this->m_Controls.lbTargetName->setEnabled(!m_activeEvaluation);
}


void QmitkMatchPointRegistrationEvaluator::OnSliceChanged()
{
  mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(NULL);
  unsigned int currentSelectedTimeStep = GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

  if (m_currentSelectedPosition != currentSelectedPosition
    || m_currentSelectedTimeStep != currentSelectedTimeStep
    || m_selectedNodeTime > m_currentPositionTime)
  {
    //the current position has been changed or the selected node has been changed since the last position validation -> check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimeStep = currentSelectedTimeStep;
    m_currentPositionTime.Modified();

    if (this->m_selectedEvalNode.IsNotNull())
    {
      this->m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::GenericProperty<mitk::Point3D>::New(currentSelectedPosition));
    }
  }
}

void QmitkMatchPointRegistrationEvaluator::OnSettingsChanged(mitk::DataNode*)
{
	this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationEvaluator::OnEvalBtnPushed()
{
  //reinit view
  mitk::RenderingManager::GetInstance()->InitializeViews(m_spSelectedTargetNode->GetData()->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

  mitk::RegEvaluationObject::Pointer regEval = mitk::RegEvaluationObject::New();

  mitk::MAPRegistrationWrapper::Pointer reg;

  if (m_spSelectedRegNode.IsNotNull())
  {
    reg = dynamic_cast<mitk::MAPRegistrationWrapper*>(this->m_spSelectedRegNode->GetData());
  }
  else
  {
    //generate a dymme reg to use
    reg = mitk::GenerateIdentityRegistration3D();
  }

  regEval->SetRegistration(reg);
  regEval->SetTargetNode(this->m_spSelectedTargetNode);
  regEval->SetMovingNode(this->m_spSelectedMovingNode);

  if (this->m_selectedEvalNode.IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_selectedEvalNode);
  }

  this->m_selectedEvalNode = mitk::DataNode::New();
  this->m_selectedEvalNode->SetData(regEval);
  
  mitk::RegEvaluationMapper2D::SetDefaultProperties(this->m_selectedEvalNode);
  this->m_selectedEvalNode->SetName(HelperNodeName);
  this->m_selectedEvalNode->SetBoolProperty("helper object", true);
  this->GetDataStorage()->Add(this->m_selectedEvalNode);

  this->m_Controls.evalSettings->SetNode(this->m_selectedEvalNode);
  this->OnSliceChanged();

  this->GetRenderWindowPart()->RequestUpdate();

  this->m_activeEvaluation = true;
  this->CheckInputs();
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationEvaluator::OnStopBtnPushed()
{
  this->m_activeEvaluation = false;

  this->GetDataStorage()->Remove(this->m_selectedEvalNode);
  this->m_selectedEvalNode = nullptr;
  this->m_Controls.evalSettings->SetNode(this->m_selectedEvalNode);

  this->CheckInputs();
  this->ConfigureControls();
  this->GetRenderWindowPart()->RequestUpdate();
}