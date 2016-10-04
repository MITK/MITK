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
#include "QmitkRenderWindow.h"
#include "QmitkMatchPointRegistrationEvaluator.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>



///**********************************************
QmitkSliceChangedListener::QmitkSliceChangedListener(): m_renderWindowPart(NULL),
m_PendingSliceChangedEvent(false),
m_internalUpdateFlag(false)
{
}

QmitkSliceChangedListener::~QmitkSliceChangedListener()
{
  this->RemoveAllObservers();
};

void QmitkSliceChangedListener::OnSliceChangedDelayed()
{
  m_PendingSliceChangedEvent = false;
  emit SliceChanged();
};

void
QmitkSliceChangedListener::OnSliceChangedInternal(const itk::EventObject& e)
{
  // Taken from QmitkStdMultiWidget::HandleCrosshairPositionEvent().
  // Since there are always 3 events arriving (one for each render window) every time the slice
  // or time changes, the slot OnSliceChangedDelayed is triggered - and only if it hasn't been
  // triggered yet - so it is only executed once for every slice/time change.
  if (!m_PendingSliceChangedEvent)
  {
    m_PendingSliceChangedEvent = true;

    QTimer::singleShot(0, this, SLOT(OnSliceChangedDelayed()));
  }
};

void QmitkSliceChangedListener::OnSliceNavigationControllerDeleted(const itk::Object* sender, const itk::EventObject& /*e*/)
{
  const mitk::SliceNavigationController* sendingSlicer =
    dynamic_cast<const mitk::SliceNavigationController*>(sender);

  this->RemoveObservers(sendingSlicer);
};

void QmitkSliceChangedListener::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_renderWindowPart != renderWindowPart)
  {
    m_renderWindowPart = renderWindowPart;

    if (!InitObservers())
    {
      QMessageBox::information(NULL, "Error", "Unable to set up the event observers. The " \
        "plot will not be triggered on changing the crosshair, " \
        "position or time step.");
    }
  }
};

void QmitkSliceChangedListener::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  m_renderWindowPart = NULL;
  this->RemoveAllObservers(renderWindowPart);
};

bool QmitkSliceChangedListener::InitObservers()
{
  bool result = true;

  typedef QHash<QString, QmitkRenderWindow*> WindowMapType;
  WindowMapType windowMap = m_renderWindowPart->GetQmitkRenderWindows();

  auto i = windowMap.begin();

  while (i != windowMap.end())
  {
    mitk::SliceNavigationController* sliceNavController =
      i.value()->GetSliceNavigationController();

    if (sliceNavController)
    {
      itk::ReceptorMemberCommand<QmitkSliceChangedListener>::Pointer cmdSliceEvent =
        itk::ReceptorMemberCommand<QmitkSliceChangedListener>::New();
      cmdSliceEvent->SetCallbackFunction(this, &QmitkSliceChangedListener::OnSliceChangedInternal);
      int tag = sliceNavController->AddObserver(
        mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0),
        cmdSliceEvent);

      m_ObserverMap.insert(std::make_pair(sliceNavController, ObserverInfo(sliceNavController, tag,
        i.key().toStdString(), m_renderWindowPart)));

      itk::ReceptorMemberCommand<QmitkSliceChangedListener>::Pointer cmdTimeEvent =
        itk::ReceptorMemberCommand<QmitkSliceChangedListener>::New();
      cmdTimeEvent->SetCallbackFunction(this, &QmitkSliceChangedListener::OnSliceChangedInternal);
      tag = sliceNavController->AddObserver(
        mitk::SliceNavigationController::GeometryTimeEvent(NULL, 0),
        cmdTimeEvent);

      m_ObserverMap.insert(std::make_pair(sliceNavController, ObserverInfo(sliceNavController, tag,
        i.key().toStdString(), m_renderWindowPart)));

      itk::MemberCommand<QmitkSliceChangedListener>::Pointer cmdDelEvent =
        itk::MemberCommand<QmitkSliceChangedListener>::New();
      cmdDelEvent->SetCallbackFunction(this,
        &QmitkSliceChangedListener::OnSliceNavigationControllerDeleted);
      tag = sliceNavController->AddObserver(
        itk::DeleteEvent(), cmdDelEvent);

      m_ObserverMap.insert(std::make_pair(sliceNavController, ObserverInfo(sliceNavController, tag,
        i.key().toStdString(), m_renderWindowPart)));
    }

    ++i;

    result = result && sliceNavController;
  }

  return result;
};

void QmitkSliceChangedListener::RemoveObservers(const mitk::SliceNavigationController* deletedSlicer)
{
  std::pair < ObserverMapType::const_iterator, ObserverMapType::const_iterator> obsRange =
    m_ObserverMap.equal_range(deletedSlicer);

  for (ObserverMapType::const_iterator pos = obsRange.first; pos != obsRange.second; ++pos)
  {
    pos->second.controller->RemoveObserver(pos->second.observerTag);
  }

  m_ObserverMap.erase(deletedSlicer);
};

void QmitkSliceChangedListener::RemoveAllObservers(mitk::IRenderWindowPart* deletedPart)
{
  for (ObserverMapType::const_iterator pos = m_ObserverMap.begin(); pos != m_ObserverMap.end();)
  {
    ObserverMapType::const_iterator delPos = pos++;

    if (deletedPart == NULL || deletedPart == delPos->second.renderWindowPart)
    {
      delPos->second.controller->RemoveObserver(delPos->second.observerTag);
      m_ObserverMap.erase(delPos);
    }
  }
};

QmitkSliceChangedListener::ObserverInfo::ObserverInfo(mitk::SliceNavigationController* controller, int observerTag,
  const std::string& renderWindowName, mitk::IRenderWindowPart* part) : controller(controller), observerTag(observerTag),
  renderWindowName(renderWindowName), renderWindowPart(part)
{
};

///**********************************************












const std::string QmitkMatchPointRegistrationEvaluator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.evaluator";

QmitkMatchPointRegistrationEvaluator::QmitkMatchPointRegistrationEvaluator()
	: m_Parent(NULL), m_internalBlendUpdate(false), m_currentSelectedTimeStep(0)
{
  m_currentSelectedPosition.Fill(0.0);
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

  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

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
      m_selectedNodeTime.Modified();
      OnSliceChanged();
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