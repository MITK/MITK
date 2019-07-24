/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// mitk qt widgets module
#include "QmitkCustomMultiWidget.h"
#include "QmitkRenderWindowWidget.h"

// qt
#include <QGridLayout>

QmitkCustomMultiWidget::QmitkCustomMultiWidget(QWidget* parent,
                                               Qt::WindowFlags f/* = 0*/,
                                               mitk::RenderingManager* renderingManager/* = nullptr*/,
                                               mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/,
                                               const QString& multiWidgetName/* = "custommulti"*/)
  : QmitkAbstractMultiWidget(parent, f, renderingManager, renderingMode, multiWidgetName)
  , m_GridLayout(nullptr)
{
  // nothing here
}

QmitkCustomMultiWidget::~QmitkCustomMultiWidget()
{
  // nothing here
}

void QmitkCustomMultiWidget::InitializeMultiWidget()
{
  SetLayout(1, 1);
}

void QmitkCustomMultiWidget::SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName)
{
  RenderWindowWidgetPointer renderWindowWidget;
  if (widgetName.isNull())
  {
    renderWindowWidget = GetActiveRenderWindowWidget();
  }
  else
  {
    renderWindowWidget = GetRenderWindowWidget(widgetName);
  }

  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
    renderWindowWidget->RequestUpdate();
    return;
  }

  MITK_ERROR << "Position can not be set for an unknown render window widget.";
}

const mitk::Point3D QmitkCustomMultiWidget::GetSelectedPosition(const QString& /*widgetName*/) const
{
  // see T26208
  return mitk::Point3D();
}

void QmitkCustomMultiWidget::wheelEvent(QWheelEvent* e)
{
  emit WheelMoved(e);
}

void QmitkCustomMultiWidget::moveEvent(QMoveEvent* e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the overlays as the MultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidget::SetLayoutImpl()
{
  int requiredRenderWindowWidgets = GetRowCount() * GetColumnCount();
  int existingRenderWindowWidgets = GetRenderWindowWidgets().size();

  int difference = requiredRenderWindowWidgets - existingRenderWindowWidgets;
  while (0 < difference)
  {
    // more render window widgets needed
    CreateRenderWindowWidget();
    --difference;
  }
  while (0 > difference)
  {
    // less render window widgets needed
    RemoveRenderWindowWidget();
    ++difference;
  }

  InitializeLayout();
}

void QmitkCustomMultiWidget::InitializeLayout()
{
  delete m_GridLayout;
  m_GridLayout = new QGridLayout(this);
  m_GridLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_GridLayout);

  FillMultiWidgetLayout();
  resize(QSize(364, 477).expandedTo(minimumSizeHint()));
}

void QmitkCustomMultiWidget::CreateRenderWindowWidget()
{
  // create the render window widget and connect signals / slots
  QString renderWindowWidgetName = GetNameFromIndex(GetNumberOfRenderWindowWidgets());
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString());

  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget);
}

void QmitkCustomMultiWidget::FillMultiWidgetLayout()
{
  for (int row = 0; row < GetRowCount(); ++row)
  {
    for (int column = 0; column < GetColumnCount(); ++column)
    {
      RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(row, column);
      if (nullptr != renderWindowWidget)
      {
        m_GridLayout->addWidget(renderWindowWidget.get(), row, column);
        SetActiveRenderWindowWidget(renderWindowWidget);
      }
    }
  }
}
