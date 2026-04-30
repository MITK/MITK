/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk qt widgets module
#include <QmitkAbstractMultiWidget.h>
#include <QmitkMultiWidgetLayoutManager.h>
#include <QmitkRenderWindowWidget.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkDisplayActionEventBroadcast.h>

// qt
#include <QMouseEvent>

// c++
#include <iomanip>

struct QmitkAbstractMultiWidget::Impl final
{
  Impl(QmitkAbstractMultiWidget* multiWidget, const QString& multiWidgetName);
  ~Impl();

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
  }

  mitk::DataStorage::Pointer m_DataStorage;

  QString m_MultiWidgetName;

  // INVARIANT: cells stored here are owned by these shared_ptrs and MUST be
  // destroyed by dropping the shared_ptr (via 'RemoveRenderWindowWidget' or
  // 'TearDownAllCells'). They MUST NOT be cascade-deleted via Qt's parent /
  // child mechanics: cells are created with 'std::make_shared', so the
  // QObject lives inside the shared_ptr's combined control block and
  // calling 'operator delete' on it - which Qt's 'deleteChildren' does -
  // is undefined behaviour. Any path that destroys a layout/splitter that
  // contains cells must drop the shared_ptrs first; see 'TearDownAllCells'
  // for the canonical sequence. (Architectural debt: dual ownership model
  // is brittle and worth replacing with a single ownership domain.)
  RenderWindowWidgetMap m_RenderWindowWidgets;
  RenderWindowWidgetPointer m_ActiveRenderWindowWidget;

  int m_MultiWidgetRows;
  int m_MultiWidgetColumns;

  // interaction
  unsigned long m_RenderWindowFocusObserverTag;
  mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast;
  std::unique_ptr<mitk::DisplayActionEventHandler> m_DisplayActionEventHandler;
  QmitkMultiWidgetLayoutManager* m_LayoutManager;
};

QmitkAbstractMultiWidget::Impl::Impl(QmitkAbstractMultiWidget* multiWidget, const QString& multiWidgetName)
  : m_DataStorage(nullptr)
  , m_MultiWidgetName(multiWidgetName)
  , m_MultiWidgetRows(0)
  , m_MultiWidgetColumns(0)
  , m_RenderWindowFocusObserverTag(0)
  , m_DisplayActionEventBroadcast(nullptr)
  , m_DisplayActionEventHandler(nullptr)
  , m_LayoutManager(new QmitkMultiWidgetLayoutManager(multiWidget))
{
  auto command = itk::MemberCommand<QmitkAbstractMultiWidget>::New();
  command->SetCallbackFunction(multiWidget, &QmitkAbstractMultiWidget::OnFocusChanged);
  m_RenderWindowFocusObserverTag =
    mitk::RenderingManager::GetInstance()->AddObserver(mitk::FocusChangedEvent(), command);

  InitializeDisplayActionEventHandling();
}

QmitkAbstractMultiWidget::Impl::~Impl()
{
  mitk::RenderingManager::GetInstance()->RemoveObserver(m_RenderWindowFocusObserverTag);
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

QString QmitkAbstractMultiWidget::GetMultiWidgetName() const
{
  return this->m_Impl->m_MultiWidgetName;
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
  // Look-ahead: if 'index' equals the current cell count, return the legacy
  // positional name a hypothetical next cell would have received. This keeps
  // the slot prediction the toolbar uses working unchanged.
  if (index == m_Impl->m_RenderWindowWidgets.size())
  {
    return m_Impl->m_MultiWidgetName + ".widget" + QString::number(index);
  }

  // Indexed lookup: walk the registered map (sorted by qualified name) and
  // return the i-th key. This keeps the positional accessor functional under
  // any naming scheme, including v2 layouts with custom names that no longer
  // match 'widget<index>'.
  if (index < m_Impl->m_RenderWindowWidgets.size())
  {
    auto iterator = m_Impl->m_RenderWindowWidgets.begin();
    std::advance(iterator, index);
    return iterator->first;
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

void QmitkAbstractMultiWidget::OnFocusChanged(itk::Object*, const itk::EventObject& event)
{
  auto focusEvent = dynamic_cast<const mitk::FocusChangedEvent*>(&event);
  if (nullptr == focusEvent)
  {
    return;
  }

  auto focusedRenderWindow = mitk::RenderingManager::GetInstance()->GetFocusedRenderWindow();
  RenderWindowWidgetMap renderWindowWidgets = this->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    const auto vtkRenderWindow = renderWindowWidget.second->GetRenderWindow()->GetVtkRenderWindow();
    if (vtkRenderWindow == focusedRenderWindow)
    {
      this->SetActiveRenderWindowWidget(renderWindowWidget.second);
      break;
    }
  }
}

void QmitkAbstractMultiWidget::AddRenderWindowWidget(const QString& widgetName, RenderWindowWidgetPointer renderWindowWidget)
{
  m_Impl->m_RenderWindowWidgets.insert(std::make_pair(widgetName, renderWindowWidget));
}

void QmitkAbstractMultiWidget::ResetGridState()
{
  m_Impl->m_MultiWidgetRows = 0;
  m_Impl->m_MultiWidgetColumns = 0;
  m_Impl->m_ActiveRenderWindowWidget = nullptr;
}

void QmitkAbstractMultiWidget::RemoveRenderWindowWidget()
{
  // Walk the map in reverse and remove the lexicographically last entry.
  // Using the map directly (rather than 'GetNameFromIndex(count-1)') keeps
  // this safe for layouts whose cell names are not the legacy positional
  // 'widget<i>' form, e.g. v2 layouts with custom names.
  if (m_Impl->m_RenderWindowWidgets.empty())
  {
    return;
  }

  auto last = std::prev(m_Impl->m_RenderWindowWidgets.end());
  RenderWindowWidgetPointer renderWindowWidgetToRemove = last->second;
  disconnect(renderWindowWidgetToRemove.get(), 0, 0, 0);
  m_Impl->m_RenderWindowWidgets.erase(last);
}

void QmitkAbstractMultiWidget::RemoveRenderWindowWidget(const QString& widgetName)
{
  auto iterator = m_Impl->m_RenderWindowWidgets.find(widgetName);
  if (iterator == m_Impl->m_RenderWindowWidgets.end())
  {
    return;
  }

  RenderWindowWidgetPointer renderWindowWidgetToRemove = iterator->second;
  disconnect(renderWindowWidgetToRemove.get(), 0, 0, 0);
  m_Impl->m_RenderWindowWidgets.erase(iterator);
}
