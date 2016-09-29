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
#include <mitkNodePredicateDataType.h>
#include <mitkMAPRegistrationWrapper.h>
#include "mitkRegVisPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegEvalStyleProperty.h"
#include "mitkRegEvalWipeStyleProperty.h"

// Qmitk
#include "QmitkMatchPointRegistrationEvaluator.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>


const std::string QmitkMatchPointRegistrationEvaluator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.evaluator";

QmitkMatchPointRegistrationEvaluator::QmitkMatchPointRegistrationEvaluator()
	: m_Parent(NULL), m_internalBlendUpdate(false)
{
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

	mitk::RegEvalStyleProperty::Pointer sampleProp = mitk::RegEvalStyleProperty::New();

	for (unsigned int pos = 0; pos < sampleProp->Size(); ++pos)
	{
		this->m_Controls.comboStyle->insertItem(pos,
		                                        QString::fromStdString(sampleProp->GetEnumString(pos)));
	}

	connect(m_Controls.comboStyle, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(OnComboStyleChanged(int)));

	connect(m_Controls.pbBlend50, SIGNAL(clicked()), this, SLOT(OnBlend50Pushed()));
	connect(m_Controls.pbBlendTarget, SIGNAL(clicked()), this, SLOT(OnBlendTargetPushed()));
	connect(m_Controls.pbBlendMoving, SIGNAL(clicked()), this, SLOT(OnBlendMovingPushed()));
	connect(m_Controls.pbBlendToggle, SIGNAL(clicked()), this, SLOT(OnBlendTogglePushed()));
	connect(m_Controls.slideBlend, SIGNAL(valueChanged(int)), this, SLOT(OnSlideBlendChanged(int)));
	connect(m_Controls.sbBlend, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBlendChanged(int)));

	connect(m_Controls.sbChecker, SIGNAL(valueChanged(int)), this, SLOT(OnSpinCheckerChanged(int)));

	connect(m_Controls.radioWipeCross, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));
	connect(m_Controls.radioWipeH, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));
	connect(m_Controls.radioWipeV, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));

	connect(m_Controls.radioTargetContour, SIGNAL(toggled(bool)), this, SLOT(OnContourStyleChanged()));

	this->ConfigureControls();
}

void QmitkMatchPointRegistrationEvaluator::OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
        const QList<mitk::DataNode::Pointer>& nodes)
{
	m_selectedEvalNode = NULL;

	if (nodes.size() > 0)
	{
		mitk::RegEvaluationObject* evalObj = dynamic_cast<mitk::RegEvaluationObject*>(nodes[0]->GetData());

		if (evalObj)
		{
			this->m_selectedEvalNode = nodes[0];
		}
	}

	ConfigureControls();
};

void QmitkMatchPointRegistrationEvaluator::ConfigureControls()
{
	this->m_Controls.comboStyle->setEnabled(this->m_selectedEvalNode.IsNotNull());
	this->m_Controls.labelNoSelect->setVisible(this->m_selectedEvalNode.IsNull());

	if (this->m_selectedEvalNode.IsNotNull())
	{
		mitk::RegEvalStyleProperty* evalProp = NULL;

		if (this->m_selectedEvalNode->GetProperty(evalProp, mitk::nodeProp_RegEvalStyle))
		{
			OnComboStyleChanged(evalProp->GetValueAsId());
			this->m_Controls.comboStyle->setCurrentIndex(evalProp->GetValueAsId());
		}
		else
		{
			this->Error(QString("Cannot configure plugin controlls correctly. Node property ") + QString(
			                mitk::nodeProp_RegEvalStyle) + QString(" has not the assumed type."));
		}

		int factor = 50;
		this->m_selectedEvalNode->GetIntProperty(mitk::nodeProp_RegEvalBlendFactor, factor);
		this->m_Controls.sbBlend->setValue(factor);

		int count = 3;
		this->m_selectedEvalNode->GetIntProperty(mitk::nodeProp_RegEvalCheckerCount, count);
		this->m_Controls.sbChecker->setValue(count);

		bool targetContour = true;
		this->m_selectedEvalNode->GetBoolProperty(mitk::nodeProp_RegEvalTargetContour, targetContour);
		this->m_Controls.radioTargetContour->setChecked(targetContour);
	}
	else
	{
		this->m_Controls.groupBlend->setVisible(false);
		this->m_Controls.groupCheck->setVisible(false);
		this->m_Controls.groupWipe->setVisible(false);
		this->m_Controls.groupContour->setVisible(false);
	}
}

void QmitkMatchPointRegistrationEvaluator::OnComboStyleChanged(int index)
{
	m_Controls.groupBlend->setVisible(index == 0);
	m_Controls.groupCheck->setVisible(index == 2);
	m_Controls.groupWipe->setVisible(index == 3);
	m_Controls.groupContour->setVisible(index == 5);

	if (m_selectedEvalNode.IsNotNull())
	{
		m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalStyle, mitk::RegEvalStyleProperty::New(index));
		this->GetRenderWindowPart()->RequestUpdate();
	}
};

void QmitkMatchPointRegistrationEvaluator::OnBlend50Pushed()
{
	m_Controls.sbBlend->setValue(50);
};

void QmitkMatchPointRegistrationEvaluator::OnBlendTargetPushed()
{
	m_Controls.sbBlend->setValue(0);
};

void QmitkMatchPointRegistrationEvaluator::OnBlendMovingPushed()
{
	m_Controls.sbBlend->setValue(100);
};

void QmitkMatchPointRegistrationEvaluator::OnBlendTogglePushed()
{
	m_Controls.sbBlend->setValue(100 - m_Controls.sbBlend->value());
};

void QmitkMatchPointRegistrationEvaluator::OnSlideBlendChanged(int factor)
{
	m_internalBlendUpdate = true;
	m_Controls.sbBlend->setValue(factor);
	m_internalBlendUpdate = false;
};

void QmitkMatchPointRegistrationEvaluator::OnSpinBlendChanged(int factor)
{
	if (m_selectedEvalNode.IsNotNull())
	{
		m_selectedEvalNode->SetIntProperty(mitk::nodeProp_RegEvalBlendFactor, factor);
		this->GetRenderWindowPart()->RequestUpdate();

		if (!m_internalBlendUpdate)
		{
			this->m_Controls.slideBlend->setValue(factor);
		}
	}
};

void QmitkMatchPointRegistrationEvaluator::OnSpinCheckerChanged(int count)
{
	if (m_selectedEvalNode.IsNotNull())
	{
		m_selectedEvalNode->SetIntProperty(mitk::nodeProp_RegEvalCheckerCount, count);
		this->GetRenderWindowPart()->RequestUpdate();
	}
};

void QmitkMatchPointRegistrationEvaluator::OnWipeStyleChanged()
{
	if (m_selectedEvalNode.IsNotNull())
	{
		if (this->m_Controls.radioWipeCross->isChecked())
		{
			m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle,
			                                mitk::RegEvalWipeStyleProperty::New(0));
		}
		else if (this->m_Controls.radioWipeH->isChecked())
		{
			m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle,
			                                mitk::RegEvalWipeStyleProperty::New(1));
		}
		else
		{
			m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle,
			                                mitk::RegEvalWipeStyleProperty::New(2));
		}

		this->GetRenderWindowPart()->RequestUpdate();
	}
};


void QmitkMatchPointRegistrationEvaluator::OnContourStyleChanged()
{
	if (m_selectedEvalNode.IsNotNull())
	{
		m_selectedEvalNode->SetBoolProperty(mitk::nodeProp_RegEvalTargetContour,
		                                    m_Controls.radioTargetContour->isChecked());
		this->GetRenderWindowPart()->RequestUpdate();
	}
};