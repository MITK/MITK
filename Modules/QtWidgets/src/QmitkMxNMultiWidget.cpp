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

#include <QList>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>

// mitk core
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEvents.h>

#include <mitkUIDGenerator.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkCameraController.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>
#include <mitkNodePredicateBase.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkStatusBar.h>
#include <mitkVtkLayerController.h>
#include <vtkSmartPointer.h>

#include <iomanip>

// qt
#include <QGridLayout>

QmitkMxNMultiWidget::QmitkMxNMultiWidget(QWidget* parent,
                                               Qt::WindowFlags f/* = 0*/,
                                               mitk::RenderingManager* renderingManager/* = nullptr*/,
                                               mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/,
                                               const QString& multiWidgetName/* = "mxnmulti"*/)
  : QWidget(parent, f)
  , m_MxNMultiWidgetLayout(nullptr)
  , m_MultiWidgetRows(0)
  , m_MultiWidgetColumns(0)
  , m_PlaneMode(0)
  , m_RenderingManager(renderingManager)
  , m_RenderingMode(renderingMode)
  , m_MultiWidgetName(multiWidgetName)
  , m_DisplayActionEventBroadcast(nullptr)
  , m_DisplayActionEventHandler(nullptr)
  , m_DataStorage(nullptr)
{
  InitializeGUI();
  InitializeDisplayActionEventHandling();
  resize(QSize(364, 477).expandedTo(minimumSizeHint()));
}

void QmitkMxNMultiWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (dataStorage == m_DataStorage)
  {
    return;
  }

  m_DataStorage = dataStorage;
  // set new data storage for the render window widgets
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->SetDataStorage(m_DataStorage);
  }
}

void QmitkMxNMultiWidget::InitializeRenderWindowWidgets()
{
  m_MultiWidgetRows = 1;
  m_MultiWidgetColumns = 1;
  CreateRenderWindowWidget();
  InitializeGUI();
}

void QmitkMxNMultiWidget::ResetLayout(int row, int column)
{
  m_MultiWidgetRows = row;
  m_MultiWidgetColumns = column;

  int requiredRenderWindowWidgets = m_MultiWidgetRows * m_MultiWidgetColumns;
  int existingRenderWindowWidgets = m_RenderWindowWidgets.size();

  int difference = requiredRenderWindowWidgets - existingRenderWindowWidgets;
  while(0 < difference)
  {
    // more render window widgets needed
    CreateRenderWindowWidget();
    --difference;
  }
  while(0 > difference)
  {
    // less render window widgets needed
    DestroyRenderWindowWidget();
    ++difference;
  }

  InitializeGUI();
}

void QmitkMxNMultiWidget::Synchronize(bool synchronized)
{
  auto allObserverTags = m_DisplayActionEventHandler->GetAllObserverTags();
  for (auto observerTag : allObserverTags)
  {
    m_DisplayActionEventHandler->DisconnectObserver(observerTag);
  }

  if (synchronized)
  {
    mitk::StdFunctionCommand::ActionFunction actionFunction = mitk::DisplayActionEventFunctions::MoveCameraSynchronizedAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayMoveEvent(nullptr, mitk::Vector2D()), actionFunction);

    actionFunction = mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplaySetCrosshairEvent(nullptr, mitk::Point3D()), actionFunction);

    actionFunction = mitk::DisplayActionEventFunctions::ZoomCameraSynchronizedAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayZoomEvent(nullptr, 0.0, mitk::Point2D()), actionFunction);

    actionFunction = mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayScrollEvent(nullptr, 0), actionFunction);
  }
  else
  {
    mitk::StdFunctionCommand::ActionFunction actionFunction = mitk::DisplayActionEventFunctions::MoveSenderCameraAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayMoveEvent(nullptr, mitk::Vector2D()), actionFunction);

    actionFunction = mitk::DisplayActionEventFunctions::SetCrosshairAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplaySetCrosshairEvent(nullptr, mitk::Point3D()), actionFunction);

    actionFunction = mitk::DisplayActionEventFunctions::ZoomSenderCameraAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayZoomEvent(nullptr, 0.0, mitk::Point2D()), actionFunction);

    actionFunction = mitk::DisplayActionEventFunctions::ScrollSliceStepperAction();
    m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplayScrollEvent(nullptr, 0), actionFunction);
  }

  // use the standard 'set level window' action for both modes
  mitk::StdFunctionCommand::ActionFunction actionFunction = mitk::DisplayActionEventFunctions::SetLevelWindowAction();
  m_DisplayActionEventHandler->ConnectDisplayActionEvent(mitk::DisplaySetLevelWindowEvent(nullptr, mitk::ScalarType(), mitk::ScalarType()), actionFunction);
}

QmitkMxNMultiWidget::RenderWindowWidgetMap QmitkMxNMultiWidget::GetRenderWindowWidgets() const
{
  return m_RenderWindowWidgets;
}

QmitkMxNMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::GetRenderWindowWidget(int row, int column) const
{
  return GetRenderWindowWidget(GetNameFromIndex(row, column));
}

QmitkMxNMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::GetRenderWindowWidget(const QString& widgetName) const
{
  RenderWindowWidgetMap::const_iterator it = m_RenderWindowWidgets.find(widgetName);
  if (it != m_RenderWindowWidgets.end())
  {
    return it->second;
  }

  return nullptr;
}

QmitkMxNMultiWidget::RenderWindowHash QmitkMxNMultiWidget::GetRenderWindows() const
{
  RenderWindowHash result;
  // create QHash on demand
  auto renderWindowWidgets = GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    result.insert(renderWindowWidget.first, renderWindowWidget.second->GetRenderWindow());
  }

  return result;
}

QmitkRenderWindow* QmitkMxNMultiWidget::GetRenderWindow(int row, int column) const
{
  return GetRenderWindow(GetNameFromIndex(row, column));
}

QmitkRenderWindow* QmitkMxNMultiWidget::GetRenderWindow(const QString& widgetName) const
{
  RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(widgetName);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->GetRenderWindow();
  }

  return nullptr;
}

void QmitkMxNMultiWidget::SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget)
{
  m_ActiveRenderWindowWidget = activeRenderWindowWidget;
}

QmitkMxNMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::GetActiveRenderWindowWidget() const
{
  return m_ActiveRenderWindowWidget;
}

QmitkMxNMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::GetFirstRenderWindowWidget() const
{
  if (!m_RenderWindowWidgets.empty())
  {
    return m_RenderWindowWidgets.begin()->second;
  }
  else
  {
    return nullptr;
  }
}

QmitkMxNMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::GetLastRenderWindowWidget() const
{
  if (!m_RenderWindowWidgets.empty())
  {
    return m_RenderWindowWidgets.rbegin()->second;
  }
  else
  {
    return nullptr;
  }
}

unsigned int QmitkMxNMultiWidget::GetNumberOfRenderWindowWidgets() const
{
  return m_RenderWindowWidgets.size();
}

void QmitkMxNMultiWidget::RequestUpdate(const QString& widgetName)
{
  RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(widgetName);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->RequestUpdate();
  }
}

void QmitkMxNMultiWidget::RequestUpdateAll()
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->RequestUpdate();
  }
}

void QmitkMxNMultiWidget::ForceImmediateUpdate(const QString& widgetName)
{
  RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(widgetName);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ForceImmediateUpdate();
  }
}

void QmitkMxNMultiWidget::ForceImmediateUpdateAll()
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ForceImmediateUpdate();
  }
}

void QmitkMxNMultiWidget::ActivateAllCrosshairs(bool activate)
{
  for (const auto& renderWindowWidget : m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ActivateCrosshair(activate);
  }
}

const mitk::Point3D QmitkMxNMultiWidget::GetSelectedPosition(const QString& /*widgetName*/) const
{
  // see T26208
  return mitk::Point3D();
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidget::SetSelectedPosition(const QString& widgetName, const mitk::Point3D& newPosition)
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

//////////////////////////////////////////////////////////////////////////
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidget::wheelEvent(QWheelEvent* e)
{
  emit WheelMoved(e);
}

void QmitkMxNMultiWidget::mousePressEvent(QMouseEvent* /*e*/)
{
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
void QmitkMxNMultiWidget::InitializeGUI()
{
  delete m_MxNMultiWidgetLayout;
  m_MxNMultiWidgetLayout = new QGridLayout(this);
  m_MxNMultiWidgetLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_MxNMultiWidgetLayout);

  FillMultiWidgetLayout();
}

void QmitkMxNMultiWidget::InitializeDisplayActionEventHandling()
{
  m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
  m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
  m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigPACS.xml");

  m_DisplayActionEventHandler = std::make_unique<mitk::DisplayActionEventHandler>();
  m_DisplayActionEventHandler->SetObservableBroadcast(m_DisplayActionEventBroadcast);

  Synchronize(true);
}

void QmitkMxNMultiWidget::CreateRenderWindowWidget()
{
  // create the render window widget and connect signals / slots
  QString renderWindowWidgetName = GetNameFromIndex(m_RenderWindowWidgets.size());
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, m_DataStorage);
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString());

  // store the newly created render window widget with the UID
  m_RenderWindowWidgets.insert(std::make_pair(renderWindowWidgetName, renderWindowWidget));
}

void QmitkMxNMultiWidget::DestroyRenderWindowWidget()
{
  auto iterator = m_RenderWindowWidgets.find(GetNameFromIndex(m_RenderWindowWidgets.size() - 1));
  if (iterator == m_RenderWindowWidgets.end())
  {
    return;
  }

  // disconnect each signal of this render window widget
  RenderWindowWidgetPointer renderWindowWidgetToRemove = iterator->second;
  disconnect(renderWindowWidgetToRemove.get(), 0, 0, 0);

  // erase the render window from the map
  m_RenderWindowWidgets.erase(iterator);
}

void QmitkMxNMultiWidget::FillMultiWidgetLayout()
{
  for (int row = 0; row < m_MultiWidgetRows; ++row)
  {
    for (int column = 0; column < m_MultiWidgetColumns; ++column)
    {
      RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(row, column);
      if (nullptr != renderWindowWidget)
      {
        m_MxNMultiWidgetLayout->addWidget(renderWindowWidget.get(), row, column);
        SetActiveRenderWindowWidget(renderWindowWidget);
      }
    }
  }
}

QString QmitkMxNMultiWidget::GetNameFromIndex(int row, int column) const
{
  if (0 <= row && m_MultiWidgetRows > row && 0 <= column && m_MultiWidgetColumns > column)
  {
    return GetNameFromIndex(row * m_MultiWidgetColumns + column);
  }

  return QString();
}

QString QmitkMxNMultiWidget::GetNameFromIndex(size_t index) const
{
  if (index <= m_RenderWindowWidgets.size())
  {
    return m_MultiWidgetName + ".widget" + QString::number(index);
  }

  return QString();
}
