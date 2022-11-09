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
#include <QMessageBox>

QmitkMxNMultiWidget::QmitkMxNMultiWidget(QWidget* parent,
                                         Qt::WindowFlags f/* = 0*/,
                                         const QString& multiWidgetName/* = "mxnmulti"*/)
  : QmitkAbstractMultiWidget(parent, f, multiWidgetName)
  , m_TimeNavigationController(nullptr)
  , m_CrosshairVisibility(false)
{
  m_TimeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
}

QmitkMxNMultiWidget::~QmitkMxNMultiWidget()
{
  auto allRenderWindows = this->GetRenderWindows();
  for (auto& renderWindow : allRenderWindows)
  {
    m_TimeNavigationController->Disconnect(renderWindow->GetSliceNavigationController());
  }
}

void QmitkMxNMultiWidget::InitializeMultiWidget()
{
  SetLayout(1, 1);
  SetDisplayActionEventHandler(std::make_unique<mitk::DisplayActionEventHandlerDesynchronized>());
  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions();
  }
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

QmitkRenderWindow* QmitkMxNMultiWidget::GetRenderWindow(const mitk::AnatomicalPlane& /*orientation*/) const
{
  // currently no mapping between plane orientation and render windows
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
    currentActiveRenderWindowWidget->setStyleSheet("QmitkRenderWindowWidget { border: 2px solid " +
                                                   hexColor.name(QColor::HexRgb) + "; }");
  }

  // set the new decoration color of the currently active render window widget
  if (nullptr != activeRenderWindowWidget)
  {
    activeRenderWindowWidget->setStyleSheet("QmitkRenderWindowWidget { border: 2px solid #FF6464; }");
  }

  QmitkAbstractMultiWidget::SetActiveRenderWindowWidget(activeRenderWindowWidget);
}

void QmitkMxNMultiWidget::SetReferenceGeometry(const mitk::TimeGeometry* referenceGeometry, bool resetCamera)
{
  auto* renderingManager = mitk::RenderingManager::GetInstance();
  mitk::Point3D currentPosition = mitk::Point3D();
  unsigned int imageTimeStep = 0;
  if (!resetCamera)
  {
    // store the current position to set it again later, if the camera should not be reset
    currentPosition = this->GetSelectedPosition("");

    // store the current time step to set it again later, if the camera should not be reset
    const auto currentTimePoint = renderingManager->GetTimeNavigationController()->GetSelectedTimePoint();
    if (referenceGeometry->IsValidTimePoint(currentTimePoint))
    {
      imageTimeStep = referenceGeometry->TimePointToTimeStep(currentTimePoint);
    }
  }

  // initialize active render window
  renderingManager->InitializeView(
    this->GetActiveRenderWindowWidget()->GetRenderWindow()->GetVtkRenderWindow(), referenceGeometry, resetCamera);

  if (!resetCamera)
  {
    this->SetSelectedPosition(currentPosition, "");
    renderingManager->GetTimeNavigationController()->GetTime()->SetPos(imageTimeStep);
  }
}

bool QmitkMxNMultiWidget::HasCoupledRenderWindows() const
{
  return false;
}

void QmitkMxNMultiWidget::SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName)
{
  RenderWindowWidgetPointer renderWindowWidget;
  if (widgetName.isNull() || widgetName.isEmpty())
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
    return;
  }

  MITK_ERROR << "Position can not be set for an unknown render window widget.";
}

const mitk::Point3D QmitkMxNMultiWidget::GetSelectedPosition(const QString& widgetName) const
{
  RenderWindowWidgetPointer renderWindowWidget;
  if (widgetName.isNull() || widgetName.isEmpty())
  {
    renderWindowWidget = GetActiveRenderWindowWidget();
  }
  else
  {
    renderWindowWidget = GetRenderWindowWidget(widgetName);
  }

  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->GetCrosshairPosition();
  }

  MITK_ERROR << "Crosshair position can not be retrieved.";
  return mitk::Point3D(0.0);
}

void QmitkMxNMultiWidget::SetCrosshairVisibility(bool visible)
{
  // get the specific render window that sent the signal
  QmitkRenderWindow* renderWindow = qobject_cast<QmitkRenderWindow*>(sender());
  if (nullptr == renderWindow)
  {
    return;
  }

  auto renderWindowWidget = this->GetRenderWindowWidget(renderWindow);
  renderWindowWidget->SetCrosshairVisibility(visible);
}

bool QmitkMxNMultiWidget::GetCrosshairVisibility() const
{
  // get the specific render window that sent the signal
  QmitkRenderWindow* renderWindow = qobject_cast<QmitkRenderWindow*>(sender());
  if (nullptr == renderWindow)
  {
    return false;
  }

  auto renderWindowWidget = this->GetRenderWindowWidget(renderWindow);
  return renderWindowWidget->GetCrosshairVisibility();
}

void QmitkMxNMultiWidget::SetCrosshairGap(unsigned int gapSize)
{
  auto renderWindowWidgets = this->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget.second->SetCrosshairGap(gapSize);
  }
}

void QmitkMxNMultiWidget::ResetCrosshair()
{
  auto dataStorage = GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  // get the specific render window that sent the signal
  QmitkRenderWindow* renderWindow = qobject_cast<QmitkRenderWindow*>(sender());
  if (nullptr == renderWindow)
  {
    return;
  }

  mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(renderWindow->GetRenderWindow(), dataStorage);

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

mitk::SliceNavigationController* QmitkMxNMultiWidget::GetTimeNavigationController()
{
  return m_TimeNavigationController;
}

void QmitkMxNMultiWidget::AddPlanesToDataStorage()
{
  auto renderWindowWidgets = this->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget.second->AddPlanesToDataStorage();
  }
}

void QmitkMxNMultiWidget::RemovePlanesFromDataStorage()
{
  auto renderWindowWidgets = this->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget.second->RemovePlanesFromDataStorage();
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

void QmitkMxNMultiWidget::mousePressEvent(QMouseEvent*)
{
  // nothing here, but necessary for mouse interactions (.xml-configuration files)
}

void QmitkMxNMultiWidget::moveEvent(QMoveEvent* e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the overlays as the MultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

void QmitkMxNMultiWidget::RemoveRenderWindowWidget()
{
  auto renderWindowWidgets = this->GetRenderWindowWidgets();
  auto iterator = renderWindowWidgets.find(this->GetNameFromIndex(this->GetNumberOfRenderWindowWidgets() - 1));
  if (iterator == renderWindowWidgets.end())
  {
    return;
  }

  // disconnect each signal of this render window widget
  RenderWindowWidgetPointer renderWindowWidgetToRemove = iterator->second;
  m_TimeNavigationController->Disconnect(renderWindowWidgetToRemove->GetSliceNavigationController());

  QmitkAbstractMultiWidget::RemoveRenderWindowWidget();
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
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage(), true);
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString());
  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget);

  auto renderWindow = renderWindowWidget->GetRenderWindow();
  auto layoutManager = GetMultiWidgetLayoutManager();
  connect(renderWindow, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);
  connect(renderWindow, &QmitkRenderWindow::ResetView, this, &QmitkMxNMultiWidget::ResetCrosshair);
  connect(renderWindow, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkMxNMultiWidget::SetCrosshairVisibility);
  connect(renderWindow, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkMxNMultiWidget::SetWidgetPlaneMode);

  // connect time navigation controller to react on geometry time events with the render window's slice naviation controller
  m_TimeNavigationController->ConnectGeometryTimeEvent(renderWindow->GetSliceNavigationController());
  // reverse connection between the render window's slice navigation controller and the time navigation controller
  renderWindow->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController);
}
