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

  connect(m_Controls.m_XWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));
  connect(m_Controls.m_YWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));
  connect(m_Controls.m_ZWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));

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

int QmitkImageNavigatorView::GetClosestAxisIndex(mitk::Vector3D normal)
{
  // cos(theta) = normal . axis
  // cos(theta) = (a, b, c) . (d, e, f)
  // cos(theta) = (a, b, c) . (1, 0, 0) = a
  // cos(theta) = (a, b, c) . (0, 1, 0) = b
  // cos(theta) = (a, b, c) . (0, 0, 1) = c
  double absCosThetaWithAxis[3];

  for (int i = 0; i < 3; i++)
  {
    absCosThetaWithAxis[i] = fabs(normal[i]);
  }
  int largestIndex = 0;
  double largestValue = absCosThetaWithAxis[0];
  for (int i = 1; i < 3; i++)
  {
    if (absCosThetaWithAxis[i] > largestValue)
    {
      largestValue = absCosThetaWithAxis[i];
      largestIndex = i;
    }
  }
  return largestIndex;
}

void QmitkImageNavigatorView::SetBorderColors()
{
  if (m_MultiWidget)
  {
    mitk::PlaneGeometry::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetCurrentPlaneGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Vector3D normal = geometry->GetNormal();
      int axis = this->GetClosestAxisIndex(normal);
      this->SetBorderColor(axis, QString("red"));
    }

    geometry = m_MultiWidget->mitkWidget2->GetSliceNavigationController()->GetCurrentPlaneGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Vector3D normal = geometry->GetNormal();
      int axis = this->GetClosestAxisIndex(normal);
      this->SetBorderColor(axis, QString("green"));
    }

    geometry = m_MultiWidget->mitkWidget3->GetSliceNavigationController()->GetCurrentPlaneGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Vector3D normal = geometry->GetNormal();
      int axis = this->GetClosestAxisIndex(normal);
      this->SetBorderColor(axis, QString("blue"));
    }

  }
}

void QmitkImageNavigatorView::SetBorderColor(int axis, QString colorAsStyleSheetString)
{
  if (axis == 0)
  {
    this->SetBorderColor(m_Controls.m_XWorldCoordinateSpinBox, colorAsStyleSheetString);
  }
  else if (axis == 1)
  {
    this->SetBorderColor(m_Controls.m_YWorldCoordinateSpinBox, colorAsStyleSheetString);
  }
  else if (axis == 2)
  {
    this->SetBorderColor(m_Controls.m_ZWorldCoordinateSpinBox, colorAsStyleSheetString);
  }
}

void QmitkImageNavigatorView::SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString)
{
  assert(spinBox);
  spinBox->setStyleSheet(QString("border: 2px solid ") + colorAsStyleSheetString + ";");
}

void QmitkImageNavigatorView::SetStepSizes()
{
  this->SetStepSize(0);
  this->SetStepSize(1);
  this->SetStepSize(2);
}

void QmitkImageNavigatorView::SetStepSize(int axis)
{
  if (m_MultiWidget)
  {
    mitk::Geometry3D::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetInputWorldGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Point3D crossPositionInIndexCoordinates;
      mitk::Point3D crossPositionInIndexCoordinatesPlus1;
      mitk::Point3D crossPositionInMillimetresPlus1;
      mitk::Vector3D transformedAxisDirection;

      mitk::Point3D crossPositionInMillimetres = m_MultiWidget->GetCrossPosition();
      geometry->WorldToIndex(crossPositionInMillimetres, crossPositionInIndexCoordinates);

      crossPositionInIndexCoordinatesPlus1 = crossPositionInIndexCoordinates;
      crossPositionInIndexCoordinatesPlus1[axis] += 1;

      geometry->IndexToWorld(crossPositionInIndexCoordinatesPlus1, crossPositionInMillimetresPlus1);

      transformedAxisDirection = crossPositionInMillimetresPlus1 - crossPositionInMillimetres;

      int closestAxisInMillimetreSpace = this->GetClosestAxisIndex(transformedAxisDirection);
      double stepSize = transformedAxisDirection.GetNorm();
      this->SetStepSize(closestAxisInMillimetreSpace, stepSize);
    }
  }
}

void QmitkImageNavigatorView::SetStepSize(int axis, double stepSize)
{
  if (axis == 0)
  {
    m_Controls.m_XWorldCoordinateSpinBox->setSingleStep(stepSize);
  }
  else if (axis == 1)
  {
    m_Controls.m_YWorldCoordinateSpinBox->setSingleStep(stepSize);
  }
  else if (axis == 2)
  {
    m_Controls.m_ZWorldCoordinateSpinBox->setSingleStep(stepSize);
  }
}

void QmitkImageNavigatorView::OnMillimetreCoordinateValueChanged()
{
  if (m_MultiWidget)
  {
    mitk::Geometry3D::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetInputWorldGeometry();

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

void QmitkImageNavigatorView::OnRefetch()
{
  if (m_MultiWidget)
  {
    mitk::Geometry3D::ConstPointer geometry = m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetInputWorldGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();

      mitk::Point3D cornerPoint1InIndexCoordinates;
      cornerPoint1InIndexCoordinates[0] = bounds[0];
      cornerPoint1InIndexCoordinates[1] = bounds[2];
      cornerPoint1InIndexCoordinates[2] = bounds[4];

      mitk::Point3D cornerPoint2InIndexCoordinates;
      cornerPoint2InIndexCoordinates[0] = bounds[1];
      cornerPoint2InIndexCoordinates[1] = bounds[3];
      cornerPoint2InIndexCoordinates[2] = bounds[5];

      if (!geometry->GetImageGeometry())
      {
        cornerPoint1InIndexCoordinates[0] += 0.5;
        cornerPoint1InIndexCoordinates[1] += 0.5;
        cornerPoint1InIndexCoordinates[2] += 0.5;
        cornerPoint2InIndexCoordinates[0] -= 0.5;
        cornerPoint2InIndexCoordinates[1] -= 0.5;
        cornerPoint2InIndexCoordinates[2] -= 0.5;
      }

      mitk::Point3D crossPositionInWorldCoordinates = m_MultiWidget->GetCrossPosition();

      mitk::Point3D cornerPoint1InWorldCoordinates;
      mitk::Point3D cornerPoint2InWorldCoordinates;

      geometry->IndexToWorld(cornerPoint1InIndexCoordinates, cornerPoint1InWorldCoordinates);
      geometry->IndexToWorld(cornerPoint2InIndexCoordinates, cornerPoint2InWorldCoordinates);

      m_Controls.m_XWorldCoordinateSpinBox->blockSignals(true);
      m_Controls.m_YWorldCoordinateSpinBox->blockSignals(true);
      m_Controls.m_ZWorldCoordinateSpinBox->blockSignals(true);

      m_Controls.m_XWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
      m_Controls.m_YWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
      m_Controls.m_ZWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));
      m_Controls.m_XWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
      m_Controls.m_YWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
      m_Controls.m_ZWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));

      m_Controls.m_XWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[0]);
      m_Controls.m_YWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[1]);
      m_Controls.m_ZWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[2]);

      m_Controls.m_XWorldCoordinateSpinBox->blockSignals(false);
      m_Controls.m_YWorldCoordinateSpinBox->blockSignals(false);
      m_Controls.m_ZWorldCoordinateSpinBox->blockSignals(false);
    }

    this->SetBorderColors();

  }
}
