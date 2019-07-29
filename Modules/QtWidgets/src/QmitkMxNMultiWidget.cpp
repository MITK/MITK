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

#include "QmitkMxNMultiWidget.h"
#include "QmitkRenderWindowWidget.h"

// qt
#include <QGridLayout>

QmitkMxNMultiWidget::QmitkMxNMultiWidget(QWidget* parent,
                                         Qt::WindowFlags f/* = 0*/,
                                         mitk::RenderingManager* renderingManager/* = nullptr*/,
                                         mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/,
                                         const QString& multiWidgetName/* = "mxnmulti"*/)
  : QmitkAbstractMultiWidget(parent, f, renderingManager, renderingMode, multiWidgetName)
  , m_GridLayout(nullptr)
  , m_CrosshairVisibility(false)
{
  // nothing here
}

void QmitkMxNMultiWidget::InitializeMultiWidget()
{
  SetLayout(1, 1);
}

void QmitkMxNMultiWidget::MultiWidgetOpened()
{
  SetCrosshairVisibility(true);
}

void QmitkMxNMultiWidget::MultiWidgetClosed()
{
  SetCrosshairVisibility(false);
}

void QmitkMxNMultiWidget::SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget)
{
  auto currentActiveRenderWindowWidget = GetActiveRenderWindowWidget();
  if (currentActiveRenderWindowWidget == activeRenderWindowWidget)
  {
    return;
  }

  // reset the decoration color of the previously active render window widget
  if (nullptr != currentActiveRenderWindowWidget)
  {
    currentActiveRenderWindowWidget->setStyleSheet("border: 2px solid white");
  }

  // set the new decoration color of the currently active render window widget
  if (nullptr != activeRenderWindowWidget)
  {
    activeRenderWindowWidget->setStyleSheet("border: 2px solid #FF6464");
  }

  QmitkAbstractMultiWidget::SetActiveRenderWindowWidget(activeRenderWindowWidget);
}

void QmitkMxNMultiWidget::SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName)
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

const mitk::Point3D QmitkMxNMultiWidget::GetSelectedPosition(const QString& /*widgetName*/) const
{
  // see T26208
  return mitk::Point3D();
}

void QmitkMxNMultiWidget::SetCrosshairVisibility(bool activate)
{
  auto renderWindowWidgets = GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget.second->ActivateCrosshair(activate);
  }

  m_CrosshairVisibility = activate;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidget::wheelEvent(QWheelEvent* e)
{
  emit WheelMoved(e);
}

void QmitkMxNMultiWidget::mousePressEvent(QMouseEvent* e)
{
  if (QEvent::MouseButtonPress != e->type())
  {
    return;
  }

  auto renderWindowWidget = dynamic_cast<QmitkRenderWindowWidget*>(this->sender());
  if (nullptr == renderWindowWidget)
  {
    return;
  }

  auto renderWindowWidgetPointer = GetRenderWindowWidget(renderWindowWidget->GetWidgetName());
  SetActiveRenderWindowWidget(renderWindowWidgetPointer);
}

void QmitkMxNMultiWidget::moveEvent(QMoveEvent* e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the overlays as the MultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidget::SetLayoutImpl()
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

void QmitkMxNMultiWidget::InitializeLayout()
{
  delete m_GridLayout;
  m_GridLayout = new QGridLayout(this);
  m_GridLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_GridLayout);

  FillMultiWidgetLayout();
  resize(QSize(364, 477).expandedTo(minimumSizeHint()));

  auto firstRenderWindowWidget = GetFirstRenderWindowWidget();
  if (nullptr != firstRenderWindowWidget)
  {
    SetActiveRenderWindowWidget(firstRenderWindowWidget);
  }
}

void QmitkMxNMultiWidget::CreateRenderWindowWidget()
{
  // create the render window widget and connect signal / slot
  QString renderWindowWidgetName = GetNameFromIndex(GetNumberOfRenderWindowWidgets());
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString());

  connect(renderWindowWidget.get(), &QmitkRenderWindowWidget::MouseEvent, this, &QmitkMxNMultiWidget::mousePressEvent);

  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget);
}

void QmitkMxNMultiWidget::FillMultiWidgetLayout()
{
  for (int row = 0; row < GetRowCount(); ++row)
  {
    for (int column = 0; column < GetColumnCount(); ++column)
    {
      RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(row, column);
      if (nullptr != renderWindowWidget)
      {
        m_GridLayout->addWidget(renderWindowWidget.get(), row, column);
      }
    }
  }
}
