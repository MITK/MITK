#include <mitkLine.h>
#include <mitkPointSet.h>

#include "mitkCrosshairManager.h"

CrosshairModeController* CrosshairModeController::instance = nullptr;

mitkCrosshairManager::mitkCrosshairManager(const QString& parentWidget) :
  m_Selected(nullptr),
  m_ShowSelected(false),
  checkWindowsShareCrosshair(nullptr),
  m_ShowPlanesIn3d(true),
  m_ShowPlanesIn3dWithoutCursor(true),
  m_SingleDataStorage(false),
  m_ParentWidget(parentWidget)
{
  m_CrosshairPredicate = mitk::NodePredicateProperty::New("crosshair", mitk::BoolProperty::New(true));

  m_SwivelColor.Set(0., 1., 1.);
  m_SwivelSelectedColor.Set(1., 1., 0.);

  auto crosshairModeController = CrosshairModeController::getInstance();
  connect(crosshairModeController, &CrosshairModeController::crosshairModeChanged, this, &mitkCrosshairManager::setCrosshairMode);
  m_CrosshairMode = crosshairModeController->getMode();
}

mitkCrosshairManager::~mitkCrosshairManager()
{
  emit savePlaneVisibilityIn3D(m_ShowPlanesIn3d);
}

void mitkCrosshairManager::setCrosshairMode(CrosshairMode mode)
{
  if (mode == m_CrosshairMode) {
    return;
  }

  m_CrosshairMode = mode;
  updateAllWindows();

  emit crosshairModeChanged(m_CrosshairMode);

  CrosshairModeController::getInstance()->setMode(mode);
}

CrosshairMode mitkCrosshairManager::getCrosshairMode()
{
  return m_CrosshairMode;
}

void mitkCrosshairManager::addWindow(QmitkRenderWindow* window)
{
  if (hasWindow(window)) {
    return;
  }

  m_ManagedWindows.push_back(window);
}

void mitkCrosshairManager::removeWindow(QmitkRenderWindow* window)
{
  if (!hasWindow(window)) {
    return;
  }

  m_ManagedWindows.erase(std::find(m_ManagedWindows.begin(), m_ManagedWindows.end(), window));
}

void mitkCrosshairManager::removeAll() {
  m_ManagedWindows.clear();
}

bool mitkCrosshairManager::hasWindow(QmitkRenderWindow* window)
{
  return std::find(m_ManagedWindows.begin(), m_ManagedWindows.end(), window) != m_ManagedWindows.end();
}

void mitkCrosshairManager::updateWindow(QmitkRenderWindow* window) {
  if (!hasWindow(window)) {
    return;
  }

  recreateCrosshair(window);
}

void mitkCrosshairManager::updateAllWindows()
{
  if (m_SingleDataStorage && m_ManagedWindows.size() > 0) {
    recreateCrosshair(m_ManagedWindows[0]);
    for (auto& window : m_ManagedWindows) {
      window->GetRenderer()->RequestUpdate();
    }
  } else {
    for (auto& window : m_ManagedWindows) {
      recreateCrosshair(window);
    }
  }
}

void mitkCrosshairManager::setDefaultProperties(mitk::DataNode::Pointer crosshair)
{
  crosshair->SetIntProperty("layer", 1000);
  crosshair->SetBoolProperty("crosshair", true);
  crosshair->SetBoolProperty("includeInBoundingBox", false);
  crosshair->SetBoolProperty("helper object", true);
  crosshair->SetProperty("parentWidget", mitk::StringProperty::New(m_ParentWidget.toStdString()));
  crosshair->SetBoolProperty("draw edges", true);
}

void mitkCrosshairManager::addPlaneCrosshair(QmitkRenderWindow* window, bool render2d, bool render3d)
{
  auto dataStorage = window->GetRenderer()->GetDataStorage();
  // Viewer mode
  if (m_ShowSelected) {
    if (m_Selected == nullptr || window == m_Selected) {
      return;
    }
    mitk::DataNode::Pointer crosshair = m_Selected->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
    setDefaultProperties(crosshair);
    crosshair->SetColor(0, 1, 0);
    crosshair->SetIntProperty("Crosshair.Gap Size", m_UseCrosshairGap ? m_CrosshairGap : 0);
    crosshair->SetBoolProperty("Crosshair.Render 2D", render2d);
    crosshair->SetBoolProperty("Crosshair.Render 3D", render3d);
    crosshair->SetBoolProperty("draw edges", render3d);
    dataStorage->Add(crosshair);
  } else {
    // Display all planes from other windows
    for (int i = 0; i < m_ManagedWindows.size(); i++) {
      auto otherWindow = m_ManagedWindows[i];
      // Do not add planes from 3d windows
      if (otherWindow->isWindow3d()) {
        continue;
      }

      mitk::DataNode::Pointer crosshair = otherWindow->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
      setDefaultProperties(crosshair);
      if (m_UseWindowsColors && i < m_WindowsColors.size()) {
        crosshair->SetColor(m_WindowsColors[i]);
      } else {
        crosshair->SetColor(otherWindow->windowPlaneIsSelected() ? m_SwivelSelectedColor : m_SwivelColor);
      }
      crosshair->SetIntProperty("Crosshair.Gap Size", m_UseCrosshairGap ? m_CrosshairGap : 0);
      crosshair->SetBoolProperty("Crosshair.Render 2D", render2d);
      crosshair->SetBoolProperty("Crosshair.Render 3D", render3d);
      crosshair->SetBoolProperty("draw edges", render3d);
      crosshair->SetName(otherWindow->GetSliceNavigationController()->GetViewDirectionAsString());
      dataStorage->Add(crosshair);
    }
  }
}

void mitkCrosshairManager::addPointCrosshair(QmitkRenderWindow* window)
{
  auto dataStorage = window->GetRenderer()->GetDataStorage();
  mitk::DataNode::Pointer crosshair;

  // Viewer mode
  if (m_ShowSelected || window->isTwoDimensionalViewState()) {
    if (m_Selected == nullptr || window == m_Selected) {
      return;
    }
    // Get intersection point with selected window
    const mitk::PlaneGeometry* selectedPlane = m_Selected->GetRenderer()->GetCurrentWorldGeometry2D();
    const mitk::PlaneGeometry* curPlane = window->GetRenderer()->GetCurrentWorldGeometry2D();

    mitk::Point2D pointFrom, pointTo;
    if (curPlane->IntersectWithPlane2D(selectedPlane, pointFrom, pointTo)) {
      // Generate point
      mitk::Point3D intersectionPoint, temp;
      curPlane->Map(pointFrom, intersectionPoint);
      curPlane->Map(pointTo, temp);
      for (int i = 0; i < 3; i++) {
        intersectionPoint[i] = (intersectionPoint[i] + temp[i]) / 2.;
      }

      mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
      pointSet->SetPoint(0, intersectionPoint);

      crosshair = mitk::DataNode::New();
      crosshair->SetData(pointSet);
      setDefaultProperties(crosshair);
      crosshair->SetFloatProperty("pointsize", 10.0f);
      crosshair->SetFloatProperty("opacity", 0.8f);
      crosshair->SetColor(0, 1, 0);
    }
  } else { // Clipping plane mode
    // Find intersection point for render windows. Expected count is 2
    std::vector<QmitkRenderWindow*> renderWindows;
    for (auto& candidate : m_ManagedWindows) {
      if (candidate->isWindow3d()) {
        continue;
      }

      if (candidate == window) {
        continue;
      }

      if (checkWindowsShareCrosshair != nullptr && !(*checkWindowsShareCrosshair)(candidate, window)) {
        continue;
      }

      renderWindows.push_back(candidate);
    }

    // Find intersection line between 2 planes
    const mitk::PlaneGeometry* plane0 = renderWindows[0]->GetRenderer()->GetCurrentWorldGeometry2D();
    const mitk::PlaneGeometry* plane1 = renderWindows[1]->GetRenderer()->GetCurrentWorldGeometry2D();
    mitk::Line3D intersectionLine;
    if (plane0->IntersectionLine(plane1, intersectionLine)) {
      // Intersect line with current geometry
      mitk::Point3D intersectionPoint;
      const mitk::PlaneGeometry* curPlane;
      if (window->isWindow3d()) {
        curPlane = renderWindows[2]->GetRenderer()->GetCurrentWorldGeometry2D();
      } else {
        curPlane = window->GetRenderer()->GetCurrentWorldGeometry2D();
      }
      if (curPlane->IntersectionPoint(intersectionLine, intersectionPoint)) {
        mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
        pointSet->SetPoint(0, intersectionPoint);

        crosshair = mitk::DataNode::New();
        crosshair->SetData(pointSet);
        setDefaultProperties(crosshair);
        crosshair->SetFloatProperty("pointsize", 10.0f);
        crosshair->SetFloatProperty("opacity", 0.8f);
        crosshair->SetColor(m_SwivelColor);
      }
    }
  }

  if (crosshair) {
    dataStorage->Add(crosshair);
  }
}

void mitkCrosshairManager::addCrosshair(QmitkRenderWindow* window)
{
  switch (m_CrosshairMode)
  {
    case CrosshairMode::PLANE:
      addPlaneCrosshair(window, true, m_ShowPlanesIn3d);
      break;
    case CrosshairMode::POINT:
      addPointCrosshair(window);
      if (m_ShowPlanesIn3d) {
        addPlaneCrosshair(window, false, m_ShowPlanesIn3d);
      }
      break;
    case CrosshairMode::NONE:
      if (m_ShowPlanesIn3d) {
        addPlaneCrosshair(window, false, m_ShowPlanesIn3d);
      }
    default:
      if (m_ShowPlanesIn3dWithoutCursor) {
        addPlaneCrosshair(window, false, true);
      }
      break;
  }
}

void mitkCrosshairManager::removeCrosshair(QmitkRenderWindow* window)
{
  auto dataStorage = window->GetRenderer()->GetDataStorage();
  auto crosshairsToRemove = dataStorage->GetSubset(m_CrosshairPredicate);
  for (auto node : *crosshairsToRemove) {
    node->SetBoolProperty("Crosshair.Render 2D", false);
    dataStorage->Remove(node);
  }
}

void mitkCrosshairManager::recreateCrosshair(QmitkRenderWindow* window)
{
  removeCrosshair(window);
  addCrosshair(window);
  window->GetRenderer()->RequestUpdate();
}

void mitkCrosshairManager::selectWindow(QmitkRenderWindow* window)
{
  if (!hasWindow(window) && window != nullptr) {
    return;
  }

  if (m_Selected == window) {
    return;
  }

  m_Selected = window;

  // Recreate crosshairs
  if (m_ShowSelected && m_Selected != nullptr) {
    for (auto& window : m_ManagedWindows) {
      recreateCrosshair(window);
    }
  }
}

void mitkCrosshairManager::setShowSelectedOnly(bool show)
{
  if (m_ShowSelected == show) {
    return;
  }

  m_ShowSelected = show;
  updateAllWindows();
}

void mitkCrosshairManager::setShowPlanesIn3d(bool show)
{
  m_ShowPlanesIn3d = show;
}

void mitkCrosshairManager::setShowPlanesIn3dWithoutCursor(bool show)
{
  m_ShowPlanesIn3dWithoutCursor = show;
}

void mitkCrosshairManager::setCrosshairGap(int gap)
{
  m_CrosshairGap = gap;
}

void mitkCrosshairManager::setUseCrosshairGap(bool use)
{
  m_UseCrosshairGap = use;
}

void mitkCrosshairManager::setWindowsColors(std::vector<mitk::Color> colors)
{
  m_WindowsColors = colors;
}

void mitkCrosshairManager::setUseWindowsColors(bool use)
{
  m_UseWindowsColors = use;
}

void mitkCrosshairManager::setSingleDataStorage(bool single)
{
  m_SingleDataStorage = single;
}

QToolButton* mitkCrosshairManager::createUiModeController()
{
  auto controller = new QToolButton();
  controller->setIcon(QIcon(":/Qmitk/CrosshairModes/crosshair.png"));
  controller->setToolTip(tr("Crosshair modes"));
  controller->setPopupMode(QToolButton::InstantPopup);

  QMap<CrosshairMode, QString> crosshairModesNames;
  crosshairModesNames[CrosshairMode::PLANE] = tr("Planes mode");
  crosshairModesNames[CrosshairMode::POINT] = tr("Point mode");
  crosshairModesNames[CrosshairMode::NONE] = tr("No crosshair");

  QMap<CrosshairMode, QString> crosshairModesIcons;
  crosshairModesIcons[CrosshairMode::PLANE] = "crosshair.png";
  crosshairModesIcons[CrosshairMode::POINT] = "point.png";
  crosshairModesIcons[CrosshairMode::NONE] = "no_crosshair.png";

  auto crosshairModeController = CrosshairModeController::getInstance();
  for (auto& mode : crosshairModesNames.keys()) {
    QAction* crosshairModeAction = new QAction(crosshairModesNames[mode], controller);
    crosshairModeAction->setIcon(QIcon(":/Qmitk/CrosshairModes/" + crosshairModesIcons[mode]));
    connect(crosshairModeAction, &QAction::triggered, controller, [controller, crosshairModeAction]() {
      controller->setIcon(crosshairModeAction->icon());
    });
    connect(crosshairModeAction, &QAction::triggered, crosshairModeController, [crosshairModeController, mode]() {
      crosshairModeController->setMode(mode);
    });

    controller->addAction(crosshairModeAction);
  }

  connect(crosshairModeController, &CrosshairModeController::crosshairModeChanged, controller, [controller](CrosshairMode mode) {
    controller->setIcon(controller->actions().at((int)mode)->icon());
  });

  return controller;
}

void mitkCrosshairManager::updateCrosshairsPositions()
{
  // Only recreate point mode
  if (m_CrosshairMode != CrosshairMode::POINT) {
    for (auto& window : m_ManagedWindows) {
      window->GetRenderer()->RequestUpdate();
    }
    return;
  }

  updateAllWindows();
}

void mitkCrosshairManager::updateSwivelColors(QmitkRenderWindow* updatedWindow)
{
  mitk::DataNode::Pointer updatedNode = updatedWindow->GetRenderer()->GetCurrentWorldGeometry2DNode();
  mitk::Color updatedColor = updatedWindow->windowPlaneIsSelected() ? m_SwivelSelectedColor : m_SwivelColor;

  // Update all crosshairs
  for (auto& window : m_ManagedWindows) {
    auto dataStorage = window->GetRenderer()->GetDataStorage();
    auto crosshairs = dataStorage->GetSubset(m_CrosshairPredicate);
    for (auto crosshair : *crosshairs) {
      if (crosshair == updatedNode) {
        crosshair->SetColor(updatedColor);
      }
    }
    window->GetRenderer()->RequestUpdate();
  }
}

bool mitkCrosshairManager::getShowPlanesIn3D()
{
  return m_ShowPlanesIn3d;
}
