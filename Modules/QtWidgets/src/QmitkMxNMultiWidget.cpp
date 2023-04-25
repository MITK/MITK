/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidget.h"

// mitk core
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandlerDesynchronized.h>
#include <mitkDisplayActionEventHandlerSynchronized.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// mitk qt widget
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

// qt
#include <QGridLayout>
#include <QMessageBox>
#include <QSplitter>

#include <fstream>

QmitkMxNMultiWidget::QmitkMxNMultiWidget(QWidget* parent,
                                         Qt::WindowFlags f/* = 0*/,
                                         const QString& multiWidgetName/* = "mxn"*/)
  : QmitkAbstractMultiWidget(parent, f, multiWidgetName)
  , m_TimeNavigationController(nullptr)
  , m_SynchronizedWidgetConnector(std::make_unique<QmitkSynchronizedWidgetConnector>())
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

  this->SetInitialSelection();
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

void QmitkMxNMultiWidget::InitializeViews(const mitk::TimeGeometry* geometry, bool resetCamera)
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
    if (geometry->IsValidTimePoint(currentTimePoint))
    {
      imageTimeStep = geometry->TimePointToTimeStep(currentTimePoint);
    }
  }

  // initialize active render window
  renderingManager->InitializeView(
    this->GetActiveRenderWindowWidget()->GetRenderWindow()->GetVtkRenderWindow(), geometry, resetCamera);

  if (!resetCamera)
  {
    this->SetSelectedPosition(currentPosition, "");
    renderingManager->GetTimeNavigationController()->GetTime()->SetPos(imageTimeStep);
  }
}

void QmitkMxNMultiWidget::SetInteractionReferenceGeometry(const mitk::TimeGeometry* referenceGeometry)
{
  // Set the interaction reference referenceGeometry for all render windows.
  auto allRenderWindows = this->GetRenderWindows();
  for (auto& renderWindow : allRenderWindows)
  {
    auto* baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetRenderWindow());
    baseRenderer->SetInteractionReferenceGeometry(referenceGeometry);
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

void QmitkMxNMultiWidget::EnableCrosshair()
{
  auto renderWindowWidgets = this->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget.second->EnableCrosshair();
  }
}

void QmitkMxNMultiWidget::DisableCrosshair()
{
  auto renderWindowWidgets = this->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget.second->DisableCrosshair();
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

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::CreateRenderWindowWidget()
{
  // create the render window widget and connect signal / slot
  QString renderWindowWidgetName = GetNameFromIndex(GetNumberOfRenderWindowWidgets());
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString());
  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget);

  auto renderWindow = renderWindowWidget->GetRenderWindow();

  QmitkRenderWindowUtilityWidget* utilityWidget = new QmitkRenderWindowUtilityWidget(this, renderWindow, GetDataStorage());
  renderWindowWidget->AddUtilityWidget(utilityWidget);

  connect(utilityWidget, &QmitkRenderWindowUtilityWidget::SynchronizationToggled,
    this, &QmitkMxNMultiWidget::ToggleSynchronization);
  connect(this, &QmitkMxNMultiWidget::UpdateUtilityWidgetViewPlanes,
    utilityWidget, &QmitkRenderWindowUtilityWidget::UpdateViewPlaneSelection);

  // needs to be done after 'QmitkRenderWindowUtilityWidget::ToggleSynchronization' has been connected
  // initially synchronize the node selection widget
  utilityWidget->ToggleSynchronization(true);

  auto layoutManager = GetMultiWidgetLayoutManager();
  connect(renderWindow, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);
  connect(renderWindow, &QmitkRenderWindow::ResetView, this, &QmitkMxNMultiWidget::ResetCrosshair);
  connect(renderWindow, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkMxNMultiWidget::SetCrosshairVisibility);
  connect(renderWindow, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkMxNMultiWidget::SetWidgetPlaneMode);

  // connect time navigation controller to react on referenceGeometry time events with the render window's slice naviation controller
  m_TimeNavigationController->ConnectGeometryTimeEvent(renderWindow->GetSliceNavigationController());
  // reverse connection between the render window's slice navigation controller and the time navigation controller
  renderWindow->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController);

  return renderWindowWidget;
}

void QmitkMxNMultiWidget::LoadLayout(const nlohmann::json* jsonData)
{
  if ((*jsonData).is_null())
  {
    QMessageBox::warning(this, "Load layout", "Could not read window layout");
    return;
  }

  unsigned int windowCounter = 0;

  try
  {
    auto version = jsonData->at("version").get<std::string>();
    if (version != "1.0")
    {
      QMessageBox::warning(this, "Load layout", "Unknown layout version, could not load");
      return;
    }

    delete this->layout();
    auto content = BuildLayoutFromJSON(jsonData, &windowCounter);
    auto hBoxLayout = new QHBoxLayout(this);
    this->setLayout(hBoxLayout);
    hBoxLayout->addWidget(content);
    emit UpdateUtilityWidgetViewPlanes();
  }
  catch (nlohmann::json::out_of_range& e)
  {
    MITK_ERROR << "Error in loading window layout from JSON: " << e.what();
    return;
  }

  while (GetNumberOfRenderWindowWidgets() > windowCounter)
  {
    RemoveRenderWindowWidget();
  }

  EnableCrosshair();
  emit LayoutChanged();
}

void QmitkMxNMultiWidget::SaveLayout(std::ostream* outStream)
{
  if (outStream == nullptr)
  {
    return;
  }

  auto layout = this->layout();
  if (layout == nullptr)
    return;

  // There should only ever be one item: a splitter
  auto widget = layout->itemAt(0)->widget();
  auto splitter = dynamic_cast<QSplitter*>(widget);
  if (!splitter)
  {
    MITK_ERROR << "Tried to save unexpected layout format. Make sure the layout of this instance contains a single QSplitter.";
    return;
  }

  auto layoutJSON = BuildJSONFromLayout(splitter);
  layoutJSON["version"] = "1.0";
  layoutJSON["name"] = "Custom Layout";

  *outStream << std::setw(4) << layoutJSON << std::endl;

}

nlohmann::json QmitkMxNMultiWidget::BuildJSONFromLayout(const QSplitter* splitter)
{
  nlohmann::json resultJSON;
  resultJSON["isWindow"] = false;
  resultJSON["vertical"] = (splitter->orientation() == Qt::Vertical) ? true : false;
  auto sizes = splitter->sizes();

  auto content = nlohmann::json::array();

  auto countSplitter = splitter->count();
  for (int i = 0; i < countSplitter; ++i)
  {
    auto widget = splitter->widget(i);
    nlohmann::json widgetJSON;
    if (auto widgetSplitter = dynamic_cast<QSplitter*>(widget); widgetSplitter)
    {
      widgetJSON = BuildJSONFromLayout(widgetSplitter);
    }
    else if (auto widgetWindow = dynamic_cast<QmitkRenderWindowWidget*>(widget); widgetWindow)
    {
      widgetJSON["isWindow"] = true;
      widgetJSON["viewDirection"] = widgetWindow->GetSliceNavigationController()->GetViewDirectionAsString();
    }
    widgetJSON["size"] = sizes[i];
    content.push_back(widgetJSON);
  }
  resultJSON["content"] = content;
  return resultJSON;
}

QSplitter* QmitkMxNMultiWidget::BuildLayoutFromJSON(const nlohmann::json* jsonData, unsigned int* windowCounter, QSplitter* parentSplitter)
{

  bool vertical = jsonData->at("vertical").get<bool>();
  auto orientation = vertical ? Qt::Vertical : Qt::Horizontal;

  auto split = new QSplitter(orientation, parentSplitter);
  QList<int> sizes;

  for (auto object : jsonData->at("content"))
  {
    bool isWindow = object["isWindow"].get<bool>();
    int size = object["size"].get<int>();
    sizes.append(size);

    if (isWindow)
    {
      auto viewDirection = object["viewDirection"].get<std::string>();
      mitk::AnatomicalPlane viewPlane = mitk::AnatomicalPlane::Sagittal;
      if (viewDirection == "Axial")
      {
        viewPlane = mitk::AnatomicalPlane::Axial;
      }
      else if (viewDirection == "Coronal")
      {
        viewPlane = mitk::AnatomicalPlane::Coronal;
      }
      else if (viewDirection == "Original")
      {
        viewPlane = mitk::AnatomicalPlane::Original;
      }
      else if (viewDirection == "Sagittal")
      {
        viewPlane = mitk::AnatomicalPlane::Sagittal;
      }

      QmitkAbstractMultiWidget::RenderWindowWidgetPointer window = nullptr;
      QString renderWindowName;
      QmitkAbstractMultiWidget::RenderWindowWidgetMap::iterator it;

      // repurpose existing render windows as far as they already exist
      if (*windowCounter < GetRenderWindowWidgets().size())
      {
        renderWindowName = this->GetNameFromIndex(*windowCounter);
        auto renderWindowWidgets = GetRenderWindowWidgets();
        it = renderWindowWidgets.find(renderWindowName);
        if (it != renderWindowWidgets.end())
        {
          window = it->second;
        }
        else
        {
          MITK_ERROR << "Could not find render window " << renderWindowName.toStdString() << ", although it should be there.";
        }
      }

      if (window == nullptr)
      {
        window = CreateRenderWindowWidget();
      }

      window->GetSliceNavigationController()->SetDefaultViewDirection(viewPlane);
      window->GetSliceNavigationController()->Update();
      split->addWidget(window.get());
      window->show();
      (*windowCounter)++;
    }
    else
    {
      auto subSplitter = BuildLayoutFromJSON(&object, windowCounter, split);
      split->addWidget(subSplitter);
    }
  }
  split->setSizes(sizes);

  return split;

}

void QmitkMxNMultiWidget::SetInitialSelection()
{
  auto dataStorage = this->GetDataStorage();
  if (nullptr == dataStorage)
  {
    return;
  }

  mitk::NodePredicateAnd::Pointer noHelperObjects = mitk::NodePredicateAnd::New();
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
  auto allNodes = dataStorage->GetSubset(noHelperObjects);
  QmitkSynchronizedNodeSelectionWidget::NodeList currentSelection;
  for (auto& node : *allNodes)
  {
    currentSelection.append(node);
  }

  m_SynchronizedWidgetConnector->ChangeSelection(currentSelection);
}

void QmitkMxNMultiWidget::ToggleSynchronization(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget)
{
  bool synchronized = synchronizedWidget->IsSynchronized();

  if (synchronized)
  {
    m_SynchronizedWidgetConnector->ConnectWidget(synchronizedWidget);
    m_SynchronizedWidgetConnector->SynchronizeWidget(synchronizedWidget);
  }
  else
  {
    m_SynchronizedWidgetConnector->DisconnectWidget(synchronizedWidget);
  }
}
