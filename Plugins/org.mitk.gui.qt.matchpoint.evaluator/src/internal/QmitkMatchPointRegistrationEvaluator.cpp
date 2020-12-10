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
#include "mitkRegVisPropertyTags.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegistrationHelper.h"
#include "mitkRegEvaluationMapper2D.h"
#include <mitkMAPAlgorithmHelper.h>
#include "mitkProperties.h"

// Qmitk
#include "QmitkRenderWindow.h"
#include "QmitkMatchPointRegistrationEvaluator.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>


const std::string QmitkMatchPointRegistrationEvaluator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.evaluator";

const std::string QmitkMatchPointRegistrationEvaluator::HelperNodeName =
    "RegistrationEvaluationHelper";

QmitkMatchPointRegistrationEvaluator::QmitkMatchPointRegistrationEvaluator()
  : m_Parent(nullptr), m_activeEvaluation(false), m_currentSelectedTimePoint(0.)
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

  this->m_Controls.registrationNodeSelector->SetDataStorage(this->GetDataStorage());
  this->m_Controls.registrationNodeSelector->SetSelectionIsOptional(true);
  this->m_Controls.movingNodeSelector->SetDataStorage(this->GetDataStorage());
  this->m_Controls.movingNodeSelector->SetSelectionIsOptional(false);
  this->m_Controls.targetNodeSelector->SetDataStorage(this->GetDataStorage());
  this->m_Controls.targetNodeSelector->SetSelectionIsOptional(false);

  this->m_Controls.registrationNodeSelector->SetInvalidInfo("Select valid registration.");
  this->m_Controls.registrationNodeSelector->SetEmptyInfo("Assuming identity. Select registration to change.");
  this->m_Controls.registrationNodeSelector->SetPopUpTitel("Select registration.");
  this->m_Controls.registrationNodeSelector->SetPopUpHint("Select a registration object that should be evaluated. If no registration is selected, identity will be assumed for evaluation.");

  this->m_Controls.movingNodeSelector->SetInvalidInfo("Select moving image.");
  this->m_Controls.movingNodeSelector->SetPopUpTitel("Select moving image.");
  this->m_Controls.movingNodeSelector->SetPopUpHint("Select the moving image for the evaluation. This is the image that will be mapped by the registration.");
  this->m_Controls.targetNodeSelector->SetInvalidInfo("Select target image.");
  this->m_Controls.targetNodeSelector->SetPopUpTitel("Select target image.");
  this->m_Controls.targetNodeSelector->SetPopUpHint("Select the target image for the evaluation.");
  this->m_Controls.checkAutoSelect->setChecked(true);

  this->ConfigureNodePredicates();

  connect(m_Controls.pbEval, SIGNAL(clicked()), this, SLOT(OnEvalBtnPushed()));
  connect(m_Controls.pbStop, SIGNAL(clicked()), this, SLOT(OnStopBtnPushed()));
  connect(m_Controls.evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));

  connect(m_Controls.registrationNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationEvaluator::OnNodeSelectionChanged);
  connect(m_Controls.movingNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationEvaluator::OnNodeSelectionChanged);
  connect(m_Controls.targetNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationEvaluator::OnNodeSelectionChanged);

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

void QmitkMatchPointRegistrationEvaluator::ConfigureNodePredicates()
{
  this->m_Controls.registrationNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::RegNodePredicate());

  this->m_Controls.movingNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::ImageNodePredicate());
  this->m_Controls.targetNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::ImageNodePredicate());
}

void QmitkMatchPointRegistrationEvaluator::CheckInputs()
{
  if (!m_activeEvaluation)
  {
    bool autoSelectInput = m_Controls.checkAutoSelect->isChecked() && this->m_spSelectedRegNode != this->m_Controls.registrationNodeSelector->GetSelectedNode();
    this->m_spSelectedRegNode = this->m_Controls.registrationNodeSelector->GetSelectedNode();
    this->m_spSelectedMovingNode = this->m_Controls.movingNodeSelector->GetSelectedNode();
    this->m_spSelectedTargetNode = this->m_Controls.targetNodeSelector->GetSelectedNode();

    if (this->m_spSelectedRegNode.IsNotNull() && (this->m_spSelectedMovingNode.IsNull() || autoSelectInput))
    {
      mitk::BaseProperty* uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);

      if (uidProp)
      {
        //search for the moving node
        mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
          uidProp);
        mitk::DataNode::Pointer movingNode = this->GetDataStorage()->GetNode(predicate);
        if (movingNode.IsNotNull())
        {
          this->m_spSelectedMovingNode = movingNode;
          QmitkSingleNodeSelectionWidget::NodeList selection({ movingNode });
          this->m_Controls.movingNodeSelector->SetCurrentSelection(selection);
        }
      }
    }

    if (this->m_spSelectedRegNode.IsNotNull() && (this->m_spSelectedTargetNode.IsNull() || autoSelectInput))
    {
      mitk::BaseProperty* uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);

      if (uidProp)
      {
        //search for the target node
        mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
          uidProp);
        mitk::DataNode::Pointer targetNode = this->GetDataStorage()->GetNode(predicate);
        if (targetNode.IsNotNull())
        {
          this->m_spSelectedTargetNode = targetNode;
          QmitkSingleNodeSelectionWidget::NodeList selection({ targetNode });
          this->m_Controls.targetNodeSelector->SetCurrentSelection(selection);
        }
      }
    }
  }
}

void QmitkMatchPointRegistrationEvaluator::OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  this->CheckInputs();
	this->ConfigureControls();
}

void QmitkMatchPointRegistrationEvaluator::NodeRemoved(const mitk::DataNode* node)
{
  if (node == this->m_spSelectedMovingNode
    || node == this->m_spSelectedRegNode
    || node == this->m_spSelectedTargetNode
    || node == this->m_selectedEvalNode)
  {
    if (node == this->m_selectedEvalNode)
    {
      this->m_selectedEvalNode = nullptr;
    }
    this->OnStopBtnPushed();
    MITK_INFO << "Stopped current MatchPoint evaluation session, because at least one relevant node was removed from storage.";
  }
}

void QmitkMatchPointRegistrationEvaluator::ConfigureControls()
{
  //config settings widget
  this->m_Controls.evalSettings->setVisible(m_activeEvaluation);
  this->m_Controls.pbEval->setEnabled(this->m_spSelectedMovingNode.IsNotNull()
    && this->m_spSelectedTargetNode.IsNotNull());
  this->m_Controls.pbEval->setVisible(!m_activeEvaluation);
  this->m_Controls.pbStop->setVisible(m_activeEvaluation);
  this->m_Controls.registrationNodeSelector->setEnabled(!m_activeEvaluation);
  this->m_Controls.movingNodeSelector->setEnabled(!m_activeEvaluation);
  this->m_Controls.targetNodeSelector->setEnabled(!m_activeEvaluation);
}


void QmitkMatchPointRegistrationEvaluator::OnSliceChanged()
{
  mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(nullptr);
  auto currentTimePoint = GetRenderWindowPart()->GetSelectedTimePoint();

  if (m_currentSelectedPosition != currentSelectedPosition
    || m_currentSelectedTimePoint != currentTimePoint
    || m_selectedNodeTime > m_currentPositionTime)
  {
    //the current position has been changed or the selected node has been changed since the last position validation -> check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimePoint = currentTimePoint;
    m_currentPositionTime.Modified();

    if (this->m_selectedEvalNode.IsNotNull())
    {
      this->m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::Point3dProperty::New(currentSelectedPosition));
    }
  }
}

void QmitkMatchPointRegistrationEvaluator::OnSettingsChanged(mitk::DataNode*)
{
	this->GetRenderWindowPart()->RequestUpdate();
}

void QmitkMatchPointRegistrationEvaluator::OnEvalBtnPushed()
{
  //reinit view
  mitk::RenderingManager::GetInstance()->InitializeViews(m_spSelectedTargetNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

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

  if (this->m_selectedEvalNode.IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_selectedEvalNode);
  }
  this->m_selectedEvalNode = nullptr;

  this->m_Controls.evalSettings->SetNode(this->m_selectedEvalNode);

  this->CheckInputs();
  this->ConfigureControls();
  if (nullptr != this->GetRenderWindowPart())
  {
    this->GetRenderWindowPart()->RequestUpdate();
  }
}
