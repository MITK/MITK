/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkImageNavigatorView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "mitkDataStorageEditorInput.h"

// berry Includes
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>
#include <berryConstants.h>

#include <QMessageBox>



const std::string QmitkImageNavigatorView::VIEW_ID = "org.mitk.views.imagenavigator";


class ImageNavigatorPartListener : public berry::IPartListener
{
public:

  ImageNavigatorPartListener(QmitkImageNavigatorView* view)
    : m_View(view)
  {}

  berry::IPartListener::Events::Types GetPartEventTypes() const
  {
    return berry::IPartListener::Events::OPENED |
        berry::IPartListener::Events::CLOSED;
  }

  void PartClosed(berry::IWorkbenchPartReference::Pointer partRef)
  {
    if((partRef->GetId() == QmitkImageNavigatorView::VIEW_ID) || (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID))
    {
      m_View->SetMultiWidget(0);
    }
  }

  void PartOpened(berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
    {
      if (QmitkStdMultiWidgetEditor::Pointer multiWidgetPart =
          partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>())
      {
        m_View->SetMultiWidget(multiWidgetPart->GetStdMultiWidget());
      }
      else
      {
        m_View->SetMultiWidget(0);
      }
    }
  }

private:

  QmitkImageNavigatorView* m_View;
};


QmitkImageNavigatorView::QmitkImageNavigatorView()
  : m_MultiWidget(NULL)
{
  multiWidgetListener = new ImageNavigatorPartListener(this);
}

QmitkImageNavigatorView::~QmitkImageNavigatorView()
{
  this->GetSite()->GetPage()->RemovePartListener(multiWidgetListener);
}

void QmitkImageNavigatorView::CreateQtPartControl(QWidget *parent)
{

  // create GUI widgets
  m_Controls.setupUi(parent);
  m_MultiWidget = this->GetActiveStdMultiWidget();
  m_Controls.m_SliceNavigatorTransversal->SetInverseDirection(true);
  m_TransversalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTransversal, m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice() , "sliceNavigatorTransversalFromSimpleExample");
  m_SagittalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorSagittal, m_MultiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
  m_FrontalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorFrontal, m_MultiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
  m_TimeStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTime, m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample");

  this->GetSite()->GetPage()->AddPartListener(multiWidgetListener);

  bool worldCoordinatesAreVisible = false;
  m_Controls.m_ShowWorldCoordinatesCheckBox->setChecked(worldCoordinatesAreVisible);
  this->OnShowWorldCoordinatesToggled(worldCoordinatesAreVisible);

  connect(m_Controls.m_ShowWorldCoordinatesCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnShowWorldCoordinatesToggled(bool)));
  connect(m_Controls.m_XWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateChanged()));
  connect(m_Controls.m_YWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateChanged()));
  connect(m_Controls.m_ZWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateChanged()));
  connect(m_Controls.m_XIndexCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnIndexCoordinateChanged()));
  connect(m_Controls.m_YIndexCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnIndexCoordinateChanged()));
  connect(m_Controls.m_ZIndexCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnIndexCoordinateChanged()));
  connect(m_TransversalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
  connect(m_SagittalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
  connect(m_FrontalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));

}

void QmitkImageNavigatorView::SetFocus ()
{

}

void QmitkImageNavigatorView::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
  m_MultiWidget = multiWidget;
  if (m_MultiWidget)
  {
    m_TransversalStepper->deleteLater();
    m_SagittalStepper->deleteLater();
    m_FrontalStepper->deleteLater();
    m_TimeStepper->deleteLater();

    m_TransversalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTransversal, m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice() , "sliceNavigatorTransversalFromSimpleExample");
    m_SagittalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorSagittal, m_MultiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
    m_FrontalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorFrontal, m_MultiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
    m_TimeStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTime, m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample");
  }
  else
  {
    m_TransversalStepper->SetStepper(0);
    m_SagittalStepper->SetStepper(0);
    m_FrontalStepper->SetStepper(0);
    m_TimeStepper->SetStepper(0);
  }
}

QmitkStdMultiWidget* QmitkImageNavigatorView::GetActiveStdMultiWidget()
{
  QmitkStdMultiWidget* activeStdMultiWidget = 0;
  berry::IEditorPart::Pointer editor =
      this->GetSite()->GetPage()->GetActiveEditor();

  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }
  else
  {
    mitk::DataStorageEditorInput::Pointer editorInput;
    editorInput = new mitk::DataStorageEditorInput();
    berry::IEditorPart::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }

  return activeStdMultiWidget;
}

int QmitkImageNavigatorView::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkImageNavigatorView::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
{
  if(width==false)
  {
    return 160;
  }
  else
  {
    return preferredResult;
  }
}

void QmitkImageNavigatorView::OnShowWorldCoordinatesToggled(bool isOn)
{
  m_Controls.m_WorldLabel->setVisible(isOn);
  m_Controls.m_XWorldCoordinateSpinBox->setVisible(isOn);
  m_Controls.m_YWorldCoordinateSpinBox->setVisible(isOn);
  m_Controls.m_ZWorldCoordinateSpinBox->setVisible(isOn);

  m_Controls.m_IndexLabel->setVisible(isOn);
  m_Controls.m_XIndexCoordinateSpinBox->setVisible(isOn);
  m_Controls.m_YIndexCoordinateSpinBox->setVisible(isOn);
  m_Controls.m_ZIndexCoordinateSpinBox->setVisible(isOn);

  m_Controls.m_XLabel->setVisible(isOn);
  m_Controls.m_YLabel->setVisible(isOn);
  m_Controls.m_ZLabel->setVisible(isOn);
}

void QmitkImageNavigatorView::OnMillimetreCoordinateChanged()
{
  if (m_MultiWidget)
  {
    mitk::Geometry3D::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetCreatedWorldGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Point3D positionInWorldCoordinates;
      positionInWorldCoordinates[0] = m_Controls.m_XWorldCoordinateSpinBox->value();
      positionInWorldCoordinates[1] = m_Controls.m_YWorldCoordinateSpinBox->value();
      positionInWorldCoordinates[2] = m_Controls.m_ZWorldCoordinateSpinBox->value();

      m_MultiWidget->MoveCrossToPosition(positionInWorldCoordinates);
    }
  }
}

void QmitkImageNavigatorView::OnIndexCoordinateChanged()
{
  if (m_MultiWidget)
  {
    mitk::Geometry3D::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetCreatedWorldGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Point3D positionInIndexCoordinates;
      positionInIndexCoordinates[0] = m_Controls.m_XIndexCoordinateSpinBox->value();
      positionInIndexCoordinates[1] = m_Controls.m_YIndexCoordinateSpinBox->value();
      positionInIndexCoordinates[2] = m_Controls.m_ZIndexCoordinateSpinBox->value();

      mitk::Point3D positionInWorldCoordinates;
      geometry->IndexToWorld(positionInIndexCoordinates, positionInWorldCoordinates);

      m_MultiWidget->MoveCrossToPosition(positionInWorldCoordinates);
    }
  }
}

void QmitkImageNavigatorView::OnRefetch()
{
  if (m_MultiWidget)
  {
    mitk::Geometry3D::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetCreatedWorldGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Point3D crossPositionInWorldCoordinates = m_MultiWidget->GetCrossPosition();
      mitk::Point3D cornerPoint1InWorldCoordinates = geometry->GetCornerPoint(true, true, true);
      mitk::Point3D cornerPoint2InWorldCoordinates = geometry->GetCornerPoint(false, false, false);

      mitk::Point3D crossPositionInIndexCoordinates;
      mitk::Point3D cornerPoint1InIndexCoordinates;
      mitk::Point3D cornerPoint2InIndexCoordinates;

      geometry->WorldToIndex(crossPositionInWorldCoordinates, crossPositionInIndexCoordinates);
      geometry->WorldToIndex(cornerPoint1InWorldCoordinates, cornerPoint1InIndexCoordinates);
      geometry->WorldToIndex(cornerPoint2InWorldCoordinates, cornerPoint2InIndexCoordinates);

      m_Controls.m_XWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
      m_Controls.m_YWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
      m_Controls.m_ZWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));
      m_Controls.m_XWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
      m_Controls.m_YWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
      m_Controls.m_ZWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));
      m_Controls.m_XWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[0]);
      m_Controls.m_YWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[1]);
      m_Controls.m_ZWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[2]);

      m_Controls.m_XIndexCoordinateSpinBox->setMinimum(std::min(cornerPoint1InIndexCoordinates[0], cornerPoint2InIndexCoordinates[0]));
      m_Controls.m_YIndexCoordinateSpinBox->setMinimum(std::min(cornerPoint1InIndexCoordinates[1], cornerPoint2InIndexCoordinates[1]));
      m_Controls.m_ZIndexCoordinateSpinBox->setMinimum(std::min(cornerPoint1InIndexCoordinates[2], cornerPoint2InIndexCoordinates[2]));
      m_Controls.m_XIndexCoordinateSpinBox->setMaximum(std::max(cornerPoint1InIndexCoordinates[0], cornerPoint2InIndexCoordinates[0]) -1);
      m_Controls.m_YIndexCoordinateSpinBox->setMaximum(std::max(cornerPoint1InIndexCoordinates[1], cornerPoint2InIndexCoordinates[1]) -1);
      m_Controls.m_ZIndexCoordinateSpinBox->setMaximum(std::max(cornerPoint1InIndexCoordinates[2], cornerPoint2InIndexCoordinates[2]) -1);
      m_Controls.m_XIndexCoordinateSpinBox->setValue(crossPositionInIndexCoordinates[0]);
      m_Controls.m_YIndexCoordinateSpinBox->setValue(crossPositionInIndexCoordinates[1]);
      m_Controls.m_ZIndexCoordinateSpinBox->setValue(crossPositionInIndexCoordinates[2]);
    }
  }
}
