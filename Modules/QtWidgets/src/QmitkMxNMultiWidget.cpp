/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidget.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandlerDesynchronized.h>
#include <mitkDisplayActionEventHandlerSynchronized.h>

// qt
#include <QGridLayout>

QmitkMxNMultiWidget::QmitkMxNMultiWidget(QWidget* parent,
                                         Qt::WindowFlags f/* = 0*/,
                                         const QString& multiWidgetName/* = "mxnmulti"*/)
  : QmitkAbstractMultiWidget(parent, f, multiWidgetName)
  , m_CrosshairVisibility(false)
{
  // nothing here
}

void QmitkMxNMultiWidget::InitializeMultiWidget()
{
  SetLayout(1, 1);
  ActivateMenuWidget(true);
  SetDisplayActionEventHandler(std::make_unique<mitk::DisplayActionEventHandlerDesynchronized>());
  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions();
  }
}

void QmitkMxNMultiWidget::MultiWidgetOpened()
{
  SetCrosshairVisibility(true);
}

void QmitkMxNMultiWidget::MultiWidgetClosed()
{
  SetCrosshairVisibility(false);
}

void QmitkMxNMultiWidget::Synchronize(bool synchronized)
{
  if (synchronized)
  {
    SetDisplayActionEventHandler(std::make_unique<mitk::DisplayActionEventHandlerSynchronized>());
  }
  else
  {
    SetDisplayActionEventHandler(std::make_unique<mitk::DisplayActionEventHandlerDesynchronized>());
  }

  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions();
  }
}

QmitkRenderWindow* QmitkMxNMultiWidget::GetRenderWindow(const QString& widgetName) const
{
  if ("axial" == widgetName || "sagittal" == widgetName || "coronal" == widgetName || "3d" == widgetName)
  {
    return GetActiveRenderWindowWidget()->GetRenderWindow();
  }

  return QmitkAbstractMultiWidget::GetRenderWindow(widgetName);
}

QmitkRenderWindow* QmitkMxNMultiWidget::GetRenderWindow(const mitk::BaseRenderer::ViewDirection& /*viewDirection*/) const
{
  // currently no mapping between view directions and render windows
  // simply return the currently active render window
  return GetActiveRenderWindowWidget()->GetRenderWindow();
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
    auto decorationColor = currentActiveRenderWindowWidget->GetDecorationColor();
    QColor hexColor(decorationColor[0] * 255, decorationColor[1] * 255, decorationColor[2] * 255);
    currentActiveRenderWindowWidget->setStyleSheet("border: 2px solid " + hexColor.name(QColor::HexRgb));
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

void QmitkMxNMultiWidget::ResetCrosshair()
{
  auto dataStorage = GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);

  SetWidgetPlaneMode(mitk::InteractionSchemeSwitcher::MITKStandard);
}

void QmitkMxNMultiWidget::SetWidgetPlaneMode(int userMode)
{
  MITK_DEBUG << "Changing crosshair mode to " << userMode;

  switch (userMode)
  {
    case 0:
      SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKStandard);
      break;
    case 1:
      SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKRotationUncoupled);
      break;
    case 2:
      SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKRotationCoupled);
      break;
    case 3:
      SetInteractionScheme(mitk::InteractionSchemeSwitcher::MITKSwivel);
      break;
  }
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

  auto firstRenderWindowWidget = GetFirstRenderWindowWidget();
  if (nullptr != firstRenderWindowWidget)
  {
    SetActiveRenderWindowWidget(firstRenderWindowWidget);
  }

  GetMultiWidgetLayoutManager()->SetLayoutDesign(QmitkMultiWidgetLayoutManager::LayoutDesign::DEFAULT);
}

void QmitkMxNMultiWidget::CreateRenderWindowWidget()
{
  // create the render window widget and connect signal / slot
  QString renderWindowWidgetName = GetNameFromIndex(GetNumberOfRenderWindowWidgets());
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString());

  connect(renderWindowWidget.get(), &QmitkRenderWindowWidget::MouseEvent, this, &QmitkMxNMultiWidget::mousePressEvent);

  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget);

  auto renderWindow = renderWindowWidget->GetRenderWindow();
  auto layoutManager = GetMultiWidgetLayoutManager();
  connect(renderWindow, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);
  connect(renderWindow, &QmitkRenderWindow::ResetView, this, &QmitkMxNMultiWidget::ResetCrosshair);
  connect(renderWindow, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkMxNMultiWidget::SetCrosshairVisibility);
  connect(renderWindow, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkMxNMultiWidget::SetWidgetPlaneMode);
}
