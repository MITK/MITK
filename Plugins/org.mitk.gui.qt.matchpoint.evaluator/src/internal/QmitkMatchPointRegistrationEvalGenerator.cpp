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
#include <mitkImageAccessByItk.h>
#include <mitkStatusBar.h>
#include <mitkNodePredicateProperty.h>
#include "mitkMAPRegistrationWrapper.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegistrationHelper.h"
#include "mitkRegEvaluationMapper2D.h"
#include "mitkAlgorithmHelper.h"

// Qmitk
#include "QmitkMatchPointRegistrationEvalGenerator.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>

const std::string QmitkMatchPointRegistrationEvalGenerator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.evaluation.generator";

QmitkMatchPointRegistrationEvalGenerator::QmitkMatchPointRegistrationEvalGenerator()
	: m_Parent(NULL)
{
}

void QmitkMatchPointRegistrationEvalGenerator::SetFocus()
{
	//m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkMatchPointRegistrationEvalGenerator::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();
}

void QmitkMatchPointRegistrationEvalGenerator::CreateQtPartControl(QWidget* parent)
{
	// create GUI widgets from the Qt Designer's .ui file
	m_Controls.setupUi(parent);

	m_Parent = parent;

	connect(m_Controls.pbEval, SIGNAL(clicked()), this, SLOT(OnEvalBtnPushed()));

	this->CheckInputs();
	this->ConfigureMappingControls();
}

void QmitkMatchPointRegistrationEvalGenerator::CheckInputs()
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
}

void QmitkMatchPointRegistrationEvalGenerator::ConfigureMappingControls()
{
	this->m_Controls.pbEval->setEnabled(this->m_spSelectedMovingNode.IsNotNull()
	                                    && this->m_spSelectedTargetNode.IsNotNull());
}

void QmitkMatchPointRegistrationEvalGenerator::OnSelectionChanged(
    berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes)
{
	this->CheckInputs();
	this->ConfigureMappingControls();
}

void QmitkMatchPointRegistrationEvalGenerator::OnEvalBtnPushed()
{
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

	mitk::DataNode::Pointer regEvalNode = mitk::DataNode::New();
	regEvalNode->SetData(regEval);
	regEvalNode->SetName("RegistrationEvaluationHelper");

	mitk::RegEvaluationMapper2D::SetDefaultProperties(regEvalNode);

	this->GetDataStorage()->Add(regEvalNode);

	this->GetRenderWindowPart()->RequestUpdate();

	this->CheckInputs();
	this->ConfigureMappingControls();
}