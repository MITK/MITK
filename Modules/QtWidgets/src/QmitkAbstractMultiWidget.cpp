/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk qt widgets module
#include "QmitkAbstractMultiWidget.h"
#include "QmitkLevelWindowWidget.h"
#include "QmitkMultiWidgetLayoutManager.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkDisplayActionEventBroadcast.h>

// qt
#include <QMouseEvent>

// c++
#include <iomanip>

struct QmitkAbstractMultiWidget::Impl final
{
  Impl(QmitkAbstractMultiWidget* multiWidget,
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

  void InitializeDisplayActionEventHandling()
  {
    m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
    m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
    m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigPACS.xml");
  }

  mitk::DataStorage::Pointer m_DataStorage;

  QString m_MultiWidgetName;

  RenderWindowWidgetMap m_RenderWindowWidgets;
  RenderWindowWidgetPointer m_ActiveRenderWindowWidget;

  int m_MultiWidgetRows;
  int m_MultiWidgetColumns;

  // interaction
  mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast;
  std::unique_ptr<mitk::DisplayActionEventHandler> m_DisplayActionEventHandler;
  QmitkMultiWidgetLayoutManager* m_LayoutManager;
};

QmitkAbstractMultiWidget::Impl::Impl(QmitkAbstractMultiWidget* multiWidget,
                                     const QString& multiWidgetName)
  : m_DataStorage(nullptr)
  , m_MultiWidgetName(multiWidgetName)
  , m_MultiWidgetRows(0)
  , m_MultiWidgetColumns(0)
  , m_DisplayActionEventBroadcast(nullptr)
  , m_DisplayActionEventHandler(nullptr)
  , m_LayoutManager(new QmitkMultiWidgetLayoutManager(multiWidget))
{
  InitializeDisplayActionEventHandling();
}

QmitkAbstractMultiWidget::QmitkAbstractMultiWidget(QWidget* parent,
                                                   Qt::WindowFlags f/* = 0*/,
                                                   const QString& multiWidgetName/* = "multiwidget"*/)
  : QWidget(parent, f)
  , m_Impl(std::make_unique<Impl>(this, multiWidgetName))
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

void QmitkAbstractMultiWidget::SetDisplayActionEventHandler(std::unique_ptr<mitk::DisplayActionEventHandler> displayActionEventHandler)
{
  m_Impl->m_DisplayActionEventHandler = std::move(displayActionEventHandler);
  m_Impl->m_DisplayActionEventHandler->SetObservableBroadcast(m_Impl->m_DisplayActionEventBroadcast);
}

mitk::DisplayActionEventHandler* QmitkAbstractMultiWidget::GetDisplayActionEventHandler()
{
  return m_Impl->m_DisplayActionEventHandler.get();
}

QmitkAbstractMultiWidget::RenderWindowWidgetMap QmitkAbstractMultiWidget::GetRenderWindowWidgets() const
{
  return m_Impl->m_RenderWindowWidgets;
}

QmitkAbstractMultiWidget::RenderWindowWidgetMap QmitkAbstractMultiWidget::Get2DRenderWindowWidgets() const
{
  RenderWindowWidgetMap renderWindowWidgets2D;

  auto renderWindowWidgets = GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    auto renderWindow = renderWindowWidget.second->GetRenderWindow();
    if(mitk::BaseRenderer::Standard2D == mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow())->GetMapperID())
    {
      renderWindowWidgets2D.insert(std::make_pair(renderWindowWidget.first, renderWindowWidget.second));
    }
  }

  return renderWindowWidgets2D;
}

QmitkAbstractMultiWidget::RenderWindowWidgetMap QmitkAbstractMultiWidget::Get3DRenderWindowWidgets() const
{
  RenderWindowWidgetMap renderWindowWidgets3D;

  auto renderWindowWidgets = GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    auto renderWindow = renderWindowWidget.second->GetRenderWindow();
    if (mitk::BaseRenderer::Standard3D == mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow())->GetMapperID())
    {
      renderWindowWidgets3D.insert(std::make_pair(renderWindowWidget.first, renderWindowWidget.second));
    }
  }

  return renderWindowWidgets3D;
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

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkAbstractMultiWidget::GetRenderWindowWidget(const QmitkRenderWindow* renderWindow) const
{
  auto renderWindowWidgets = GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    if (renderWindowWidget.second->GetRenderWindow() == renderWindow)
    {
      return renderWindowWidget.second;
    }
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
  emit ActiveRenderWindowChanged();
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

void QmitkAbstractMultiWidget::ActivateMenuWidget(bool state)
{
  for (const auto& renderWindowWidget : m_Impl->m_RenderWindowWidgets)
  {
    auto renderWindow = renderWindowWidget.second->GetRenderWindow();
    renderWindow->ActivateMenuWidget(state);
  }
}

bool QmitkAbstractMultiWidget::IsMenuWidgetEnabled() const
{
  return m_Impl->m_ActiveRenderWindowWidget->GetRenderWindow()->GetActivateMenuWidgetFlag();
}

QmitkMultiWidgetLayoutManager* QmitkAbstractMultiWidget::GetMultiWidgetLayoutManager() const
{
  return m_Impl->m_LayoutManager;
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
