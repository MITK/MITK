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
#include "QmitkAbstractMultiWidget.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkDisplayActionEvents.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandler.h>

// qt
#include <QMouseEvent>

// c++
#include <iomanip>

struct QmitkAbstractMultiWidget::Impl final
{
  Impl(mitk::RenderingManager* renderingManager,
       mitk::BaseRenderer::RenderingMode::Type renderingMode,
       const QString& multiWidgetName);

  void SetDataStorage(mitk::DataStorage* dataStorage)
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

  void Synchronize(bool synchronized)
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

  void InitializeDisplayActionEventHandling()
  {
    m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
    m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
    m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigPACS.xml");

    m_DisplayActionEventHandler = std::make_unique<mitk::DisplayActionEventHandler>();
    m_DisplayActionEventHandler->SetObservableBroadcast(m_DisplayActionEventBroadcast);

    Synchronize(true);
  }
  
  mitk::DataStorage::Pointer m_DataStorage;

  mitk::RenderingManager* m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;
  QString m_MultiWidgetName;

  RenderWindowWidgetMap m_RenderWindowWidgets;
  RenderWindowWidgetPointer m_ActiveRenderWindowWidget;

  int m_MultiWidgetRows;
  int m_MultiWidgetColumns;

  // interaction
  mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast;
  std::unique_ptr<mitk::DisplayActionEventHandler> m_DisplayActionEventHandler;
};

QmitkAbstractMultiWidget::Impl::Impl(mitk::RenderingManager* renderingManager,
                                     mitk::BaseRenderer::RenderingMode::Type renderingMode,
                                     const QString& multiWidgetName)
  : m_DataStorage(nullptr)
  , m_RenderingManager(renderingManager)
  , m_RenderingMode(renderingMode)
  , m_MultiWidgetName(multiWidgetName)
  , m_MultiWidgetRows(0)
  , m_MultiWidgetColumns(0)
  , m_DisplayActionEventBroadcast(nullptr)
  , m_DisplayActionEventHandler(nullptr)
{
  InitializeDisplayActionEventHandling();
}

QmitkAbstractMultiWidget::QmitkAbstractMultiWidget(QWidget* parent,
                                                   Qt::WindowFlags f/* = 0*/,
                                                   mitk::RenderingManager* renderingManager/* = nullptr*/,
                                                   mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/,
                                                   const QString& multiWidgetName/* = "multiwidget"*/)
  : QWidget(parent, f)
  , m_Impl(std::make_unique<Impl>(renderingManager, renderingMode, multiWidgetName))
{
  // nothing here
}

QmitkAbstractMultiWidget::~QmitkAbstractMultiWidget() { }

void QmitkAbstractMultiWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_Impl->SetDataStorage(dataStorage);
}

mitk::DataStorage* QmitkAbstractMultiWidget::GetDataStorage() const
{
  return m_Impl->m_DataStorage;
}

int QmitkAbstractMultiWidget::GetRowCount() const
{
  return m_Impl->m_MultiWidgetRows;
}

int QmitkAbstractMultiWidget::GetColumnCount() const
{
  return m_Impl->m_MultiWidgetColumns;
}

void QmitkAbstractMultiWidget::SetLayout(int row, int column)
{
  m_Impl->m_MultiWidgetRows = row;
  m_Impl->m_MultiWidgetColumns = column;
  SetLayoutImpl();
}

void QmitkAbstractMultiWidget::Synchronize(bool synchronized)
{
  m_Impl->Synchronize(synchronized);
  SynchronizeImpl();
}

void QmitkAbstractMultiWidget::SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  auto interactionSchemeSwitcher = mitk::InteractionSchemeSwitcher::New();
  auto interactionEventHandler = GetInteractionEventHandler();
  try
  {
    interactionSchemeSwitcher->SetInteractionScheme(interactionEventHandler, scheme);
  }
  catch (const mitk::Exception&)
  {
    return;
  }

  SetInteractionSchemeImpl();
}

mitk::InteractionEventHandler* QmitkAbstractMultiWidget::GetInteractionEventHandler()
{
  return m_Impl->m_DisplayActionEventBroadcast.GetPointer();
}

QmitkAbstractMultiWidget::RenderWindowWidgetMap QmitkAbstractMultiWidget::GetRenderWindowWidgets() const
{
  return m_Impl->m_RenderWindowWidgets;
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkAbstractMultiWidget::GetRenderWindowWidget(int row, int column) const
{
  return GetRenderWindowWidget(GetNameFromIndex(row, column));
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkAbstractMultiWidget::GetRenderWindowWidget(const QString& widgetName) const
{
  RenderWindowWidgetMap::const_iterator it = m_Impl->m_RenderWindowWidgets.find(widgetName);
  if (it != m_Impl->m_RenderWindowWidgets.end())
  {
    return it->second;
  }

  return nullptr;
}

QmitkAbstractMultiWidget::RenderWindowHash QmitkAbstractMultiWidget::GetRenderWindows() const
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

QmitkRenderWindow* QmitkAbstractMultiWidget::GetRenderWindow(int row, int column) const
{
  return GetRenderWindow(GetNameFromIndex(row, column));
}

QmitkRenderWindow* QmitkAbstractMultiWidget::GetRenderWindow(const QString& widgetName) const
{
  RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(widgetName);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->GetRenderWindow();
  }

  return nullptr;
}

void QmitkAbstractMultiWidget::SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget)
{
  m_Impl->m_ActiveRenderWindowWidget = activeRenderWindowWidget;
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkAbstractMultiWidget::GetActiveRenderWindowWidget() const
{
  return m_Impl->m_ActiveRenderWindowWidget;
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkAbstractMultiWidget::GetFirstRenderWindowWidget() const
{
  if (!m_Impl->m_RenderWindowWidgets.empty())
  {
    return m_Impl->m_RenderWindowWidgets.begin()->second;
  }
  else
  {
    return nullptr;
  }
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkAbstractMultiWidget::GetLastRenderWindowWidget() const
{
  if (!m_Impl->m_RenderWindowWidgets.empty())
  {
    return m_Impl->m_RenderWindowWidgets.rbegin()->second;
  }
  else
  {
    return nullptr;
  }
}

QString QmitkAbstractMultiWidget::GetNameFromIndex(int row, int column) const
{
  if (0 <= row && m_Impl->m_MultiWidgetRows > row && 0 <= column && m_Impl->m_MultiWidgetColumns > column)
  {
    return GetNameFromIndex(row * m_Impl->m_MultiWidgetColumns + column);
  }

  return QString();
}

QString QmitkAbstractMultiWidget::GetNameFromIndex(size_t index) const
{
  if (index <= m_Impl->m_RenderWindowWidgets.size())
  {
    return m_Impl->m_MultiWidgetName + ".widget" + QString::number(index);
  }

  return QString();
}

unsigned int QmitkAbstractMultiWidget::GetNumberOfRenderWindowWidgets() const
{
  return m_Impl->m_RenderWindowWidgets.size();
}

void QmitkAbstractMultiWidget::RequestUpdate(const QString& widgetName)
{
  RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(widgetName);
  if (nullptr != renderWindowWidget)
  {
    return renderWindowWidget->RequestUpdate();
  }
}

void QmitkAbstractMultiWidget::RequestUpdateAll()
{
  for (const auto& renderWindowWidget : m_Impl->m_RenderWindowWidgets)
  {
    renderWindowWidget.second->RequestUpdate();
  }
}

void QmitkAbstractMultiWidget::ForceImmediateUpdate(const QString& widgetName)
{
  RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(widgetName);
  if (nullptr != renderWindowWidget)
  {
    renderWindowWidget->ForceImmediateUpdate();
  }
}

void QmitkAbstractMultiWidget::ForceImmediateUpdateAll()
{
  for (const auto& renderWindowWidget : m_Impl->m_RenderWindowWidgets)
  {
    renderWindowWidget.second->ForceImmediateUpdate();
  }
}

void QmitkAbstractMultiWidget::AddRenderWindowWidget(const QString& widgetName, RenderWindowWidgetPointer renderWindowWidget)
{
  m_Impl->m_RenderWindowWidgets.insert(std::make_pair(widgetName, renderWindowWidget));
}

void QmitkAbstractMultiWidget::RemoveRenderWindowWidget()
{
  auto iterator = m_Impl->m_RenderWindowWidgets.find(GetNameFromIndex(GetRenderWindowWidgets().size() - 1));
  if (iterator == m_Impl->m_RenderWindowWidgets.end())
  {
    return;
  }

  // disconnect each signal of this render window widget
  RenderWindowWidgetPointer renderWindowWidgetToRemove = iterator->second;
  disconnect(renderWindowWidgetToRemove.get(), 0, 0, 0);

  // erase the render window from the map
  m_Impl->m_RenderWindowWidgets.erase(iterator);
}
