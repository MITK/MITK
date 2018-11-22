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
#include <mitkImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkUIDGenerator.h>

// qt
#include <QGridLayout>
#include <QMouseEvent>
#include <QTimer>

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

  QGridLayout* m_CustomMultiWidgetLayout;
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
  , m_CustomMultiWidgetLayout(nullptr)
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
  , m_PlaneMode(0)
  , m_PendingCrosshairPositionEvent(false)
  , m_CrosshairNavigationEnabled(false)
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

void QmitkAbstractMultiWidget::SetRowCount(int row)
{
  m_Impl->m_MultiWidgetRows = row;
}

int QmitkAbstractMultiWidget::GetRowCount() const
{
  return m_Impl->m_MultiWidgetRows;
}

void QmitkAbstractMultiWidget::SetColumnCount(int column)
{
  m_Impl->m_MultiWidgetColumns = column ;
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
  QmitkRenderWindowWidget* renderWindowWidgetToRemove = iterator->second.get();
  disconnect(renderWindowWidgetToRemove, 0, 0, 0);

  // erase the render window from the map
  m_Impl->m_RenderWindowWidgets.erase(iterator);
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

const mitk::Point3D QmitkAbstractMultiWidget::GetSelectedPosition(const QString& widgetName) const
{
  /*
  const mitk::PlaneGeometry *plane1 = mitkWidget1->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane2 = mitkWidget2->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane3 = mitkWidget3->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ((plane1 != NULL) && (plane2 != NULL) && (plane1->IntersectionLine(plane2, line)))
  {
  mitk::Point3D point;
  if ((plane3 != NULL) && (plane3->IntersectionPoint(line, point)))
  {
  return point;
  }
  }
  // TODO BUG POSITIONTRACKER;
  mitk::Point3D p;
  return p;
  // return m_LastLeftClickPositionSupplier->GetCurrentPoint();
  */
  return mitk::Point3D();
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
//////////////////////////////////////////////////////////////////////////
void QmitkAbstractMultiWidget::HandleCrosshairPositionEvent()
{
  /*
  if (!m_PendingCrosshairPositionEvent)
  {
    m_PendingCrosshairPositionEvent = true;
    QTimer::singleShot(0, this, SLOT(HandleCrosshairPositionEventDelayed()));
  }
  */
}

void QmitkAbstractMultiWidget::HandleCrosshairPositionEventDelayed()
{
  /*
  m_PendingCrosshairPositionEvent = false;

  // find image with highest layer
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->m_DataStorage->GetSubset(isImageData).GetPointer();

  mitk::DataNode::Pointer node;
  mitk::DataNode::Pointer topSourceNode;
  mitk::Image::Pointer image;
  bool isBinary = false;
  node = this->GetTopLayerNode(nodes);
  int component = 0;
  if (node.IsNotNull())
  {
    node->GetBoolProperty("binary", isBinary);
    if (isBinary)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = m_DataStorage->GetSources(node, NULL, true);
      if (!sourcenodes->empty())
      {
        topSourceNode = this->GetTopLayerNode(sourcenodes);
      }
      if (topSourceNode.IsNotNull())
      {
        image = dynamic_cast<mitk::Image *>(topSourceNode->GetData());
        topSourceNode->GetIntProperty("Image.Displayed Component", component);
      }
      else
      {
        image = dynamic_cast<mitk::Image *>(node->GetData());
        node->GetIntProperty("Image.Displayed Component", component);
      }
    }
    else
    {
      image = dynamic_cast<mitk::Image *>(node->GetData());
      node->GetIntProperty("Image.Displayed Component", component);
    }
  }

  mitk::Point3D crosshairPos = this->GetCrossPosition();
  std::string statusText;
  std::stringstream stream;
  itk::Index<3> p;
  mitk::BaseRenderer *baseRenderer = GetRenderWindow()->GetSliceNavigationController()->GetRenderer();
  unsigned int timestep = baseRenderer->GetTimeStep();

  if (image.IsNotNull() && (image->GetTimeSteps() > timestep))
  {
    image->GetGeometry()->WorldToIndex(crosshairPos, p);
    stream.precision(2);
    stream << "Position: <" << std::fixed << crosshairPos[0] << ", " << std::fixed << crosshairPos[1] << ", "
      << std::fixed << crosshairPos[2] << "> mm";
    stream << "; Index: <" << p[0] << ", " << p[1] << ", " << p[2] << "> ";

    mitk::ScalarType pixelValue;

    mitkPixelTypeMultiplex5(mitk::FastSinglePixelAccess,
      image->GetChannelDescriptor().GetPixelType(),
      image,
      image->GetVolumeData(baseRenderer->GetTimeStep()),
      p,
      pixelValue,
      component);

    if (fabs(pixelValue) > 1000000 || fabs(pixelValue) < 0.01)
    {
      stream << "; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: " << std::scientific << pixelValue << "  ";
    }
    else
    {
      stream << "; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: " << pixelValue << "  ";
    }
  }
  else
  {
    stream << "No image information at this position!";
  }

  statusText = stream.str();
  mitk::StatusBar::GetInstance()->DisplayGreyValueText(statusText.c_str());
  */
}

void QmitkAbstractMultiWidget::SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName)
{
  // #TODO: check parameter and see if this should be implemented here
  /*
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
  */
}

void QmitkAbstractMultiWidget::ResetCrosshair()
{
  // #TODO: new concept: we do not want to initialize all views;
  //        we do not want to rely on the geometry planes
  /*
  if (m_DataStorage.IsNotNull())
  {
    m_RenderingManager->InitializeViewsByBoundingObjects(m_DataStorage);
    // m_RenderingManager->InitializeViews( m_DataStorage->ComputeVisibleBoundingGeometry3D() );
    // reset interactor to normal slicing
    SetWidgetPlaneMode(PLANE_MODE_SLICING);
  }
  */
}

//////////////////////////////////////////////////////////////////////////
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////
void QmitkAbstractMultiWidget::wheelEvent(QWheelEvent* e)
{
  emit WheelMoved(e);
}

void QmitkAbstractMultiWidget::mousePressEvent(QMouseEvent* e)
{
  // nothing here
}

void QmitkAbstractMultiWidget::moveEvent(QMoveEvent* e)
{
  QWidget::moveEvent(e);

  // it is necessary to readjust the position of the overlays as the MultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}
