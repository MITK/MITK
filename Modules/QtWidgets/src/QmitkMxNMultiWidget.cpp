/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNMultiWidget.h>

// mitk core
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandlerDesynchronized.h>
#include <mitkDisplayActionEventHandlerSynchronized.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// mitk qt widget
#include <QmitkMultiWidgetLayoutManager.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

// qt
#include <QBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSplitter>

#include <algorithm>
#include <functional>
#include <fstream>
#include <vector>

namespace
{
  // Matches the `<editor_name>` segment of the qualified window-id form
  // `<editor_name>__<bare_id>`. Mirrors the schema's window-id pattern (see
  // mxn-layout-v2.schema.json): starts with a letter; no underscores
  // (which would break the first-`__` split rule); only `[A-Za-z0-9.-]`
  // afterwards. Any name accepted at editor construction time is
  // automatically guaranteed to produce schema-valid ids.
  const QRegularExpression kEditorNamePattern(QStringLiteral("^[A-Za-z][A-Za-z0-9.-]*$"));

  // Namespace delimiter between editor name and bare id segment in the
  // canonical qualified-window-id form. Lifted to a constant so the engine
  // and any future caller share one definition.
  const QString kNamespaceDelimiter = QStringLiteral("__");


  // Translation helpers for the v2 layout's `view_direction` enum. Closed
  // mapping (axial/sagittal/coronal/original); throws on anything else.

  mitk::AnatomicalPlane ParseViewDirection(const std::string& s)
  {
    if (s == "axial")    return mitk::AnatomicalPlane::Axial;
    if (s == "sagittal") return mitk::AnatomicalPlane::Sagittal;
    if (s == "coronal")  return mitk::AnatomicalPlane::Coronal;
    if (s == "original") return mitk::AnatomicalPlane::Original;
    mitkThrow() << "Unknown view_direction '" << s
                << "' (expected 'axial', 'sagittal', 'coronal', or 'original').";
  }

  std::string ViewDirectionToV2String(mitk::AnatomicalPlane plane)
  {
    switch (plane)
    {
      case mitk::AnatomicalPlane::Axial:    return "axial";
      case mitk::AnatomicalPlane::Sagittal: return "sagittal";
      case mitk::AnatomicalPlane::Coronal:  return "coronal";
      case mitk::AnatomicalPlane::Original: return "original";
    }
    mitkThrow() << "ViewDirectionToV2String: unsupported AnatomicalPlane enum value.";
  }

  // Pre-walks a v2 'root' subtree to validate structural shape and collect
  // per-window ids + referenced group labels. Throws on missing required
  // field, type mismatch on a known field, or duplicate window id. Does
  // not mutate engine state.
  //
  // 'seedingOrder' captures (bareWindowId, groupName) pairs in pre-order
  // traversal order (splits' children walked in array order), used by
  // ApplyLayout's group-seeding pass to identify each group's seed cell -
  // the cell that appears first in document order whose links.selection
  // names that group.
  void PrewalkValidate(const nlohmann::json& node,
                       std::set<std::string>& seenIds,
                       std::set<std::string>& referencedGroups,
                       std::vector<std::pair<std::string, std::string>>& seedingOrder)
  {
    if (!node.is_object() || !node.contains("type") || !node["type"].is_string())
    {
      mitkThrow() << "Layout node is missing the 'type' string field.";
    }
    const auto type = node["type"].get<std::string>();

    if (type == "split")
    {
      if (!node.contains("orientation") || !node["orientation"].is_string())
      {
        mitkThrow() << "Layout split node is missing the 'orientation' field.";
      }
      const auto orientation = node["orientation"].get<std::string>();
      if (orientation != "horizontal" && orientation != "vertical")
      {
        mitkThrow() << "Layout split node has invalid orientation '" << orientation
                    << "' (expected 'horizontal' or 'vertical').";
      }
      if (!node.contains("children") || !node["children"].is_array() || node["children"].empty())
      {
        mitkThrow() << "Layout split node is missing a non-empty 'children' array.";
      }
      for (const auto& child : node["children"])
      {
        PrewalkValidate(child, seenIds, referencedGroups, seedingOrder);
      }
    }
    else if (type == "window")
    {
      if (!node.contains("id") || !node["id"].is_string())
      {
        mitkThrow() << "Layout window node is missing the 'id' string field.";
      }
      const auto id = node["id"].get<std::string>();
      if (id.empty())
      {
        mitkThrow() << "Layout window node has an empty 'id'.";
      }
      if (!seenIds.insert(id).second)
      {
        mitkThrow() << "Layout document contains duplicate window id '" << id << "'.";
      }
      // Optional display label: free-form, not unique. If present, must be
      // a non-empty string. Anything else (wrong type, empty string) throws.
      if (node.contains("name"))
      {
        if (!node["name"].is_string())
        {
          mitkThrow() << "Layout window '" << id
                      << "' has a 'name' field that is not a string.";
        }
        if (node["name"].get<std::string>().empty())
        {
          mitkThrow() << "Layout window '" << id
                      << "' has an empty 'name'. Omit the field instead of"
                         " emitting an empty string.";
        }
      }
      if (!node.contains("view_direction") || !node["view_direction"].is_string())
      {
        mitkThrow() << "Layout window '" << id
                    << "' is missing the 'view_direction' string field.";
      }
      if (!node.contains("links") || !node["links"].is_object())
      {
        mitkThrow() << "Layout window '" << id << "' is missing the 'links' object.";
      }
      const auto& links = node["links"];
      if (!links.contains("selection") || !links["selection"].is_string())
      {
        mitkThrow() << "Layout window '" << id
                    << "' is missing the required 'links.selection' string.";
      }
      const auto groupName = links["selection"].get<std::string>();
      referencedGroups.insert(groupName);
      seedingOrder.emplace_back(id, groupName);
    }
    else
    {
      mitkThrow() << "Unknown layout node type '" << type
                  << "' (expected 'split' or 'window').";
    }
  }

}  // namespace

QmitkMxNMultiWidget::QmitkMxNMultiWidget(QWidget* parent,
                                         Qt::WindowFlags f/* = 0*/,
                                         const QString& multiWidgetName/* = "mxn"*/)
  : QmitkAbstractMultiWidget(parent, f, multiWidgetName)
  , m_CrosshairVisibility(false)
{
  // Editor-name shape constraint. The qualified-window-id form
  // `<multiWidgetName>__<bare>` is split by the FIRST occurrence of `__`,
  // so the editor-name segment must contain no `_` and start with a letter.
  // Reject malformed names loudly here rather than later when they would
  // produce schema-invalid ids or ambiguous splits at apply time.
  if (!kEditorNamePattern.match(multiWidgetName).hasMatch())
  {
    mitkThrow() << "QmitkMxNMultiWidget: multiWidgetName '"
                << multiWidgetName.toStdString()
                << "' does not match the required pattern '"
                << kEditorNamePattern.pattern().toStdString()
                << "'. Editor names must start with a letter, contain no '_', "
                << "and use only the alphabet [A-Za-z0-9.-].";
  }
}

QmitkMxNMultiWidget::~QmitkMxNMultiWidget()
{
}

void QmitkMxNMultiWidget::InitializeMultiWidget()
{

  AddSynchronizationGroup(1);
  SetLayout(1, 1);
  SetDisplayActionEventHandler(std::make_unique<mitk::DisplayActionEventHandlerDesynchronized>());
  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions(this->GetMultiWidgetName().toStdString());
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

  std::string prefixFilter = synchronized ? "" : this->GetMultiWidgetName().toStdString();
  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions(prefixFilter);
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
    const mitk::TimePointType currentTimePoint = renderingManager->GetTimeNavigationController()->GetSelectedTimePoint();
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
    renderingManager->GetTimeNavigationController()->GetStepper()->SetPos(imageTimeStep);
  }
}

void QmitkMxNMultiWidget::SetInteractionReferenceGeometry(const mitk::TimeGeometry* referenceGeometry)
{
  // Set the interaction reference referenceGeometry for all render windows.
  auto allRenderWindows = this->GetRenderWindows();
  for (auto& renderWindow : allRenderWindows)
  {
    auto* baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow());
    baseRenderer->SetInteractionReferenceGeometry(referenceGeometry);
  }
}

bool QmitkMxNMultiWidget::HasCoupledRenderWindows() const
{
  return false;
}

void QmitkMxNMultiWidget::SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName)
{
  QSet< RenderWindowWidgetPointer > renderWindowWidgets;
  if (widgetName.isNull() || widgetName.isEmpty())
  {
    for (const auto& [windowName, renderWindowWidget] : this->GetRenderWindowWidgets())
    {
      renderWindowWidgets.insert(renderWindowWidget);
    }
  }
  else
  {
    renderWindowWidgets = { GetRenderWindowWidget(widgetName) };
  }

  if (renderWindowWidgets.isEmpty())
  {
    MITK_ERROR << "Position can not be set for an unknown render window widget.";
    return;
  }

  for (auto renderWindowWidget : renderWindowWidgets)
  {
    renderWindowWidget->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
  }
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

  mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(renderWindow->GetVtkRenderWindow(), dataStorage);

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

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidget::SetLayoutImpl()
{
  int requiredRenderWindowWidgets = this->GetRowCount() * this->GetColumnCount();
  int existingRenderWindowWidgets = this->GetRenderWindowWidgets().size();

  int difference = requiredRenderWindowWidgets - existingRenderWindowWidgets;
  while (0 < difference)
  {
    // more render window widgets needed
    this->CreateRenderWindowWidget();
    --difference;
  }

  while (0 > difference)
  {
    // Remove the highest 'widget<i>' that is registered. Routing through
    // the name-keyed overload (rather than the no-arg lex-last variant)
    // guarantees the right cell is removed once the editor crosses 10 cells:
    // {widget0..widget11} sorts as widget0 < widget1 < widget10 < widget11
    // < widget2 < ..., so lex-last would otherwise pick widget9.
    bool removed = false;
    for (std::size_t i = this->GetNumberOfRenderWindowWidgets(); i-- > 0; )
    {
      const auto id = this->GetMultiWidgetName() + kNamespaceDelimiter + QStringLiteral("widget") + QString::number(i);
      if (nullptr != this->GetRenderWindowWidget(id))
      {
        this->RemoveRenderWindowWidget(id);
        removed = true;
        break;
      }
    }
    if (!removed)
    {
      // No 'widget<i>' cell present - mixed with custom-named layout. Bail
      // rather than spinning; the caller is expected to ApplyLayout/
      // RollBackToSingleDefaultCell when entering an inconsistent state.
      MITK_WARN << "SetLayout: cannot shrink to " << requiredRenderWindowWidgets
                << " cells - " << this->GetNumberOfRenderWindowWidgets()
                << " custom-named cells remain. Layout dimensions ("
                << this->GetRowCount() << "x" << this->GetColumnCount()
                << ") are now out of sync with the cell count. Use ApplyLayout "
                   "or RollBackToSingleDefaultCell to recover a consistent state.";
      break;
    }
    ++difference;
  }

  auto firstRenderWindowWidget = this->GetFirstRenderWindowWidget();
  if (nullptr != firstRenderWindowWidget)
  {
    this->SetActiveRenderWindowWidget(firstRenderWindowWidget);
  }

  this->GetMultiWidgetLayoutManager()->SetLayoutDesign(QmitkMultiWidgetLayoutManager::LayoutDesign::DEFAULT);
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::CreateRenderWindowWidget()
{
  // Pick the smallest non-negative 'i' such that
  // '<multiWidgetName>__widget<i>' is not already registered in this editor.
  // Replaces the old 'widget<count>' form, which silently collided when
  // custom-id'd cells already used the same index (e.g. existing
  // {widget0, widget3} + adding a 4th cell would have produced 'widget3'
  // again, which std::map::insert silently rejects).
  const auto prefix = this->GetMultiWidgetName() + kNamespaceDelimiter + QStringLiteral("widget");
  std::size_t i = 0;
  while (this->GetRenderWindowWidget(prefix + QString::number(i)) != nullptr)
  {
    ++i;
  }

  // The positional convenience overload owns the editor's "default group 1"
  // convention: the cell is placed into engine sync-group 1 right after
  // construction. The explicit-id overload stays free of side effects so
  // v2-layout callers can move the cell to its document-declared group
  // without churning through an intermediate group-1 placement.
  auto renderWindowWidget = this->CreateRenderWindowWidget(prefix + QString::number(i));
  this->SetSynchronizationGroup(renderWindowWidget->GetUtilityWidget()->GetNodeSelectionWidget(), 1);
  return renderWindowWidget;
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::CreateRenderWindowWidget(const QString& id)
{
  if (id.isEmpty())
  {
    mitkThrow() << "CreateRenderWindowWidget: id must not be empty.";
  }

  // The id must already be in this editor's canonical qualified form: the
  // engine registers it verbatim, with no prefix concatenation. This guards
  // in-process API misuse (e.g. a caller passing an unqualified bare name);
  // document-driven creation is additionally gated by
  // ValidateIdsForThisEditor() up the stack.
  const auto requiredPrefix = this->GetMultiWidgetName() + kNamespaceDelimiter;
  if (!id.startsWith(requiredPrefix))
  {
    mitkThrow() << "CreateRenderWindowWidget: id '" << id.toStdString()
                << "' does not start with this editor's required prefix '"
                << requiredPrefix.toStdString() << "'.";
  }

  // Use the public lookup rather than reaching into m_RenderWindowWidgets so
  // the canonical accessor stays the single source of truth for what is
  // registered.
  if (this->GetRenderWindowWidget(id) != nullptr)
  {
    mitkThrow() << "CreateRenderWindowWidget: a render window with name '"
                << id.toStdString() << "' already exists in this editor.";
  }

  // create the render window widget and connect signal / slot
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, id, this->GetDataStorage());
  renderWindowWidget->SetCornerAnnotationText(id.toStdString());
  this->AddRenderWindowWidget(id, renderWindowWidget);

  auto renderWindow = renderWindowWidget->GetRenderWindow();

  QmitkRenderWindowUtilityWidget* utilityWidget = new QmitkRenderWindowUtilityWidget(this, renderWindow, this->GetDataStorage());
  renderWindowWidget->AddUtilityWidget(utilityWidget);

  connect(this, &QmitkMxNMultiWidget::UpdateUtilityWidgetViewPlanes,
    utilityWidget, &QmitkRenderWindowUtilityWidget::UpdateViewPlaneSelection);
  // 'SyncGroupChanged' is wired through a lambda that catches 'mitk::Exception',
  // because Qt slots must not let exceptions escape into the event dispatcher.
  // The direct method 'SetSynchronizationGroup' keeps its throwing contract for
  // direct callers; only the slot path is defensive.
  connect(utilityWidget, &QmitkRenderWindowUtilityWidget::SyncGroupChanged, this,
    [this](QmitkSynchronizedNodeSelectionWidget* widget, const GroupSyncIndexType index)
    {
      try
      {
        this->SetSynchronizationGroup(widget, index);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "Ignoring 'SyncGroupChanged(" << index
                  << ")': " << e.GetDescription();
      }
    });
  connect(utilityWidget, &QmitkRenderWindowUtilityWidget::CreateNewSyncGroupRequested,
    this, &QmitkMxNMultiWidget::OnCreateNewSyncGroupRequested);
  connect(this, &QmitkMxNMultiWidget::SyncGroupAdded, utilityWidget, &QmitkRenderWindowUtilityWidget::OnSyncGroupAdded);

  // Replay existing groups so the freshly-created utility widget's combobox
  // reflects the current set of registered groups (rather than relying on a
  // contiguous 1..N seed inside the utility widget's constructor).
  for (const auto& entry : m_SynchronizedWidgetConnectors)
  {
    utilityWidget->OnSyncGroupAdded(entry.first);
  }

  // Initialize the node selection widget with all nodes. The cell is left
  // unattached to any sync group; placement into a group is the caller's
  // responsibility (the positional overload assigns group 1; the v2 layout
  // applier assigns the document-declared target group). This keeps the
  // explicit-name path free of side-effects so it does not auto-create a
  // phantom group 1 for documents that never reference it.
  utilityWidget->GetNodeSelectionWidget()->SelectAll();

  auto layoutManager = this->GetMultiWidgetLayoutManager();
  connect(renderWindow, &QmitkRenderWindow::LayoutDesignChanged, layoutManager, &QmitkMultiWidgetLayoutManager::SetLayoutDesign);
  connect(renderWindow, &QmitkRenderWindow::ResetView, this, &QmitkMxNMultiWidget::ResetCrosshair);
  connect(renderWindow, &QmitkRenderWindow::CrosshairVisibilityChanged, this, &QmitkMxNMultiWidget::SetCrosshairVisibility);
  connect(renderWindow, &QmitkRenderWindow::CrosshairRotationModeChanged, this, &QmitkMxNMultiWidget::SetWidgetPlaneMode);

  return renderWindowWidget;
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::GetWindowFromIndex(size_t index)
{
  if (index >= GetRenderWindowWidgets().size())
  {
    return nullptr;
  }

  auto renderWindowName = this->GetNameFromIndex(index);
  auto renderWindowWidgets = GetRenderWindowWidgets();
  auto it = renderWindowWidgets.find(renderWindowName);
  if (it != renderWindowWidgets.end())
  {
    return it->second;
  }
  else
  {
    MITK_ERROR << "Could not find render window " << renderWindowName.toStdString() << ", although it should be there.";
    return nullptr;
  }
}

//////////////////////////////////////////////////////////////////////////
// V2 LAYOUT FORMAT — Serialize / Apply (see mxn-layout-v2.schema.json)
//////////////////////////////////////////////////////////////////////////

void QmitkMxNMultiWidget::SaveLayout(std::ostream* outStream)
{
  if (outStream == nullptr)
  {
    return;
  }
  *outStream << this->SerializeLayout().dump(4) << std::endl;
}

void QmitkMxNMultiWidget::LoadLayout(const nlohmann::json* jsonData)
{
  if (jsonData == nullptr || jsonData->is_null())
  {
    mitkThrow() << "LoadLayout: jsonData must not be null.";
  }
  this->ApplyLayout(*jsonData);
}

nlohmann::json QmitkMxNMultiWidget::SerializeLayout() const
{
  // Validate layout shape: top-level layout must contain exactly one QSplitter
  // (the canonical post-load shape).
  auto* topLayout = this->layout();
  if (nullptr == topLayout || topLayout->count() == 0)
  {
    mitkThrow() << "SerializeLayout: editor has no top-level layout to serialize.";
  }
  auto* item = topLayout->itemAt(0);
  auto* widget = (item == nullptr) ? nullptr : item->widget();
  auto* rootSplitter = dynamic_cast<QSplitter*>(widget);
  if (nullptr == rootSplitter)
  {
    mitkThrow() << "SerializeLayout: top-level widget is not a QSplitter.";
  }

  // Pre-walk: collect engine-internal sync-group indices encountered in the
  // cell tree, then look up each one's bare name in the engine's group-name
  // registry ('m_GroupNameByIndex'). The registry is populated by every
  // 'AddSynchronizationGroup' call, so every group in the cell tree must
  // have an entry. A missing entry here would indicate engine-state
  // corruption and is surfaced as a throw rather than papered over.
  std::map<GroupSyncIndexType, std::string> groupNames;
  std::function<void(const QSplitter*)> walk = [&](const QSplitter* split)
  {
    for (int i = 0; i < split->count(); ++i)
    {
      auto* child = split->widget(i);
      if (auto* sub = dynamic_cast<QSplitter*>(child))
      {
        walk(sub);
      }
      else if (auto* cell = dynamic_cast<QmitkRenderWindowWidget*>(child))
      {
        const auto idx = cell->GetUtilityWidget()->GetSyncGroup();
        if (groupNames.find(idx) == groupNames.end())
        {
          const auto recorded = m_GroupNameByIndex.find(idx);
          if (recorded == m_GroupNameByIndex.end())
          {
            mitkThrow() << "SerializeLayout: cell sync group " << idx
                        << " has no entry in the group-name registry; "
                        << "engine state is corrupt.";
          }
          groupNames[idx] = recorded->second;
        }
      }
    }
  };
  walk(rootSplitter);

  // Emit the strict-mode 'groups' dict for every group referenced by a cell.
  nlohmann::json groupsJson = nlohmann::json::object();
  for (const auto& [idx, name] : groupNames)
  {
    bool selectAll = true;
    if (auto* connector = this->GetSyncGroupConnector(idx))
    {
      selectAll = connector->GetSelectionMode();
    }
    groupsJson[name] = nlohmann::json{ { "select_all", selectAll } };
  }

  nlohmann::json doc;
  doc["version"] = "2.0";
  doc["name"] = "Custom Layout";
  doc["groups"] = groupsJson;
  // The recurser attaches 'size' to each child inside its parent's loop; the
  // root has no parent loop here, so it never gets a 'size' field. See the
  // matching note inside SerializeSplitter.
  doc["root"] = this->SerializeSplitter(rootSplitter, groupNames);
  return doc;
}

nlohmann::json QmitkMxNMultiWidget::SerializeSplitter(
  const QSplitter* splitter,
  const std::map<GroupSyncIndexType, std::string>& groupNames) const
{
  nlohmann::json node;
  node["type"] = "split";
  node["orientation"] = (splitter->orientation() == Qt::Vertical) ? "vertical" : "horizontal";

  const auto sizes = splitter->sizes();
  auto children = nlohmann::json::array();
  for (int i = 0; i < splitter->count(); ++i)
  {
    auto* child = splitter->widget(i);
    nlohmann::json childJson;
    if (auto* sub = dynamic_cast<QSplitter*>(child))
    {
      childJson = this->SerializeSplitter(sub, groupNames);
    }
    else if (auto* cell = dynamic_cast<QmitkRenderWindowWidget*>(child))
    {
      // Sanity: the pre-walk must have visited every cell and added its
      // sync-group index to 'groupNames'. A miss here would be engine
      // corruption (cell with an index not seen during the pre-walk).
      const auto idx = cell->GetUtilityWidget()->GetSyncGroup();
      if (groupNames.find(idx) == groupNames.end())
      {
        mitkThrow() << "SerializeLayout: cell sync group " << idx
                    << " was not seen during the pre-walk pass.";
      }
      const auto descriptor = this->MakeWindowDescriptor(cell);
      childJson["type"] = "window";
      childJson["id"] = descriptor.id.toStdString();
      // Optional display label: omit the JSON key when the cell has no
      // display name, so empty strings never appear on disk (see schema:
      // `name` requires minLength 1 when present).
      if (!descriptor.displayName.isEmpty())
      {
        childJson["name"] = descriptor.displayName.toStdString();
      }
      childJson["view_direction"] = descriptor.viewDirection.toStdString();
      childJson["links"] = nlohmann::json{ { "selection", descriptor.selectionGroup.toStdString() } };
    }
    else
    {
      mitkThrow() << "SerializeLayout: unknown child widget type at splitter index " << i << ".";
    }
    childJson["size"] = sizes[i];
    children.push_back(childJson);
  }
  node["children"] = children;

  // Note: the root has no 'size' field structurally - the parent loop above
  // attaches 'size' to each child before pushing into the children array,
  // and the root, having no parent loop, never gets one.
  return node;
}

QmitkMxNMultiWidget::WindowDescriptor
QmitkMxNMultiWidget::MakeWindowDescriptor(const QmitkRenderWindowWidget* cell) const
{
  if (nullptr == cell)
  {
    mitkThrow() << "MakeWindowDescriptor: null cell.";
  }

  WindowDescriptor descriptor;
  // The cell's render-window name is the canonical fully-qualified id,
  // used verbatim everywhere (layout JSON, REST URL, scene-file context
  // keys, log lines). No translation step.
  descriptor.id = cell->GetWidgetName();
  descriptor.displayName = cell->GetDisplayName();
  descriptor.viewDirection = QString::fromStdString(
    ViewDirectionToV2String(
      cell->GetSliceNavigationController()->GetDefaultViewDirection()));

  const auto idx = cell->GetUtilityWidget()->GetSyncGroup();
  const auto recorded = m_GroupNameByIndex.find(idx);
  if (recorded == m_GroupNameByIndex.end())
  {
    mitkThrow() << "MakeWindowDescriptor: cell sync group " << idx
                << " has no entry in the group-name registry; "
                << "engine state is corrupt.";
  }
  descriptor.selectionGroup = QString::fromStdString(recorded->second);
  return descriptor;
}

std::vector<QmitkMxNMultiWidget::WindowDescriptor>
QmitkMxNMultiWidget::ListWindowDescriptors() const
{
  // Validate layout shape: same precondition as SerializeLayout.
  auto* topLayout = this->layout();
  if (nullptr == topLayout || topLayout->count() == 0)
  {
    mitkThrow() << "ListWindowDescriptors: editor has no top-level layout.";
  }
  auto* item = topLayout->itemAt(0);
  auto* widget = (item == nullptr) ? nullptr : item->widget();
  auto* rootSplitter = dynamic_cast<QSplitter*>(widget);
  if (nullptr == rootSplitter)
  {
    mitkThrow() << "ListWindowDescriptors: top-level widget is not a QSplitter.";
  }

  std::vector<WindowDescriptor> result;
  std::function<void(const QSplitter*)> walk = [&](const QSplitter* split)
  {
    for (int i = 0; i < split->count(); ++i)
    {
      auto* child = split->widget(i);
      if (auto* sub = dynamic_cast<QSplitter*>(child))
      {
        walk(sub);
      }
      else if (auto* cell = dynamic_cast<QmitkRenderWindowWidget*>(child))
      {
        result.push_back(this->MakeWindowDescriptor(cell));
      }
      else
      {
        mitkThrow() << "ListWindowDescriptors: unknown child widget type at splitter index " << i << ".";
      }
    }
  };
  walk(rootSplitter);
  return result;
}

QSplitter* QmitkMxNMultiWidget::BuildSplitterFromJsonV2(
  const nlohmann::json& splitNode,
  const std::map<std::string, GroupSyncIndexType>& nameToInt,
  QSplitter* parentSplitter)
{
  const auto orientationStr = splitNode["orientation"].get<std::string>();
  const auto orientation = (orientationStr == "vertical") ? Qt::Vertical : Qt::Horizontal;

  // Hold the root via unique_ptr until ownership transfers to the parent
  // splitter (via 'addWidget') or, for the top-level call where
  // 'parentSplitter == nullptr', to the layout in 'ApplyLayout' (which
  // calls 'release()' once the layout has taken ownership). A throw inside
  // the recursive construction loop would otherwise leak the splitter
  // (and its sub-splitters) since the catch in 'ApplyLayout' has no handle
  // on this allocation.
  std::unique_ptr<QSplitter> split(new QSplitter(orientation, parentSplitter));
  QList<int> sizes;

  try
  {
    for (const auto& child : splitNode["children"])
    {
      const auto type = child["type"].get<std::string>();
      // 'size' is optional; default weight 1 matches the schema default.
      // Only the ratio between siblings matters at runtime - QSplitter
      // redistributes weights proportionally on resize. Reject size < 1
      // (the schema also requires this): size 0 would collapse the pane
      // in Qt, but the format has no documented 'hide this cell' semantics.
      const int childSize = child.value("size", 1);
      if (childSize < 1)
      {
        mitkThrow() << "Layout child has invalid 'size' " << childSize
                    << "; size must be >= 1.";
      }
      sizes.append(childSize);

      if (type == "split")
      {
        auto* sub = this->BuildSplitterFromJsonV2(child, nameToInt, split.get());
        split->addWidget(sub);
      }
      else  // "window"
      {
        // Id is used verbatim: the document carries the canonical
        // qualified form (`<multiWidgetName>__<bare>`); ValidateIdsForThisEditor
        // has already ensured the prefix matches this editor instance.
        const auto id = QString::fromStdString(child["id"].get<std::string>());
        const auto viewDirection = ParseViewDirection(child["view_direction"].get<std::string>());
        const auto groupName = child["links"]["selection"].get<std::string>();
        const auto targetIdx = nameToInt.at(groupName);

        auto window = this->CreateRenderWindowWidget(id);
        // The explicit-id overload leaves the cell unattached to any sync
        // group; place it into its document-declared target group here.
        this->SetSynchronizationGroup(window->GetUtilityWidget()->GetNodeSelectionWidget(), targetIdx);
        window->GetSliceNavigationController()->SetDefaultViewDirection(viewDirection);
        window->GetSliceNavigationController()->Update();
        // Optional display label (free-form, non-unique). Pre-walk has
        // already validated type and non-emptiness.
        if (child.contains("name"))
        {
          window->SetDisplayName(QString::fromStdString(child["name"].get<std::string>()));
        }
        split->addWidget(window.get());
        window->show();
      }
    }
  }
  catch (...)
  {
    // Drain every cell currently in 'm_RenderWindowWidgets' through the
    // canonical shared_ptr-drop path. Cells were 'make_shared'-allocated;
    // letting the unique_ptr's destructor cascade-delete them via Qt's
    // 'deleteChildren' would call 'operator delete' on memory that isn't a
    // standalone heap allocation - undefined behaviour, observed as the
    // heap corruption in 'Apply_Failure_RollsBackToDefault'.
    //
    // After the drain, every cell has been destroyed via '~QmitkRenderWindow
    // Widget' (which removes the cell from its splitter's child list); the
    // unique_ptr's cascade then only touches QSplitter objects, which are
    // plain 'new'-allocated and safe to delete. Recursion is naturally
    // handled: the innermost catch drains everything; outer catches find
    // an empty map and become no-ops. 'ApplyLayout' has already run
    // 'TearDownAllCells' at the start, so every cell currently in the map
    // was created by this same call - draining them all here is correct.
    std::vector<QString> names;
    for ([[maybe_unused]] const auto& [name, widget] : this->GetRenderWindowWidgets())
    {
      names.push_back(name);
    }
    for (const auto& name : names)
    {
      this->RemoveRenderWindowWidget(name);
    }
    throw;
  }

  split->setSizes(sizes);
  return split.release();
}

void QmitkMxNMultiWidget::SeedAndNormalizeGroups(
  const std::vector<std::pair<std::string, std::string>>& seedingOrder,
  const std::map<std::string, GroupSyncIndexType>& nameToInt)
{
  // Resolve the seed cell per group: first window in document order whose
  // links.selection names that group. 'seedingOrder' is captured pre-order
  // during validation (cells did not exist yet); resolve to widget pointers
  // now that the cell map is populated.
  std::map<std::string, RenderWindowWidgetPointer> seedCells;
  std::map<std::string, std::vector<RenderWindowWidgetPointer>> groupMembers;
  for (const auto& [id, groupName] : seedingOrder)
  {
    // The id is the canonical fully-qualified name; look the cell up directly.
    const auto cell = this->GetRenderWindowWidget(QString::fromStdString(id));
    if (nullptr == cell)
    {
      // Engine-state corruption: the validation pass said this cell would
      // exist. Surface rather than silently skip.
      mitkThrow() << "SeedAndNormalizeGroups: cell '" << id
                  << "' referenced in seeding order is not registered after build.";
    }
    groupMembers[groupName].push_back(cell);
    seedCells.emplace(groupName, cell);  // emplace: first wins
  }

  // Cap noisy warnings - cross-scene loads where the same data node has
  // diverged across renderers could otherwise emit one warning per node.
  // Document the cap so suppressed warnings are not invisible.
  //
  // Note: on a fresh ApplyLayout, this divergence pass is silent by design.
  // TearDownAllCells() destroys the previous renderers and connector state;
  // the new cells are constructed in lock-step against a fresh connector
  // and therefore have no per-renderer divergence to detect at the moment
  // SeedAndNormalizeGroups runs. The instrumentation surfaces drift if a
  // scene applied AFTER the layout reintroduces divergence between the
  // seed and other group members before normalisation completes.
  constexpr int kWarnCap = 16;
  int warnCount = 0;
  bool warnCapHit = false;
  // TODO(C6): exercise the divergence path via a path-2 integration test once
  // scene-after-layout reseeding is implemented (see plan_mxn_post_rest.md C6).
  // Until then this lambda has no CI coverage by design - fresh layouts have
  // no divergence to detect.
  auto emitDivergence = [&](const std::string& groupName,
                            const std::string& dim,
                            const std::string& nodeLabel,
                            const QString& cellName)
  {
    if (warnCount < kWarnCap)
    {
      MITK_WARN << "ApplyLayout: per-renderer '" << dim << "' divergence in group '"
                << groupName << "' for node '" << nodeLabel << "' between seed and cell '"
                << cellName.toStdString() << "'; normalising to seed value.";
      ++warnCount;
    }
    else
    {
      warnCapHit = true;
    }
  };

  for (const auto& [groupName, seedCell] : seedCells)
  {
    auto* const seedUtility = seedCell->GetUtilityWidget();
    if (nullptr == seedUtility)
    {
      mitkThrow() << "SeedAndNormalizeGroups: seed cell for group '" << groupName
                  << "' has no utility widget.";
    }
    auto* const seedSelectionWidget = seedUtility->GetNodeSelectionWidget();
    auto* const seedRenderer = mitk::BaseRenderer::GetInstance(
      seedCell->GetRenderWindow()->GetVtkRenderWindow());
    if (nullptr == seedRenderer)
    {
      mitkThrow() << "SeedAndNormalizeGroups: seed cell for group '" << groupName
                  << "' has no base renderer.";
    }

    const auto seedSelection = seedSelectionWidget->GetSelectedNodes();

    // Divergence detection (pre-seed): compare every non-seed member's
    // per-renderer 'visible' / 'layer' to the seed's, for each node in the
    // seed's selection. This is best-effort observability for cross-scene
    // loads where the same data node appears in both old and new layouts
    // with divergent per-renderer state. For fresh editor loads, every cell
    // starts in lock-step, so this loop emits nothing.
    const auto memberIt = groupMembers.find(groupName);
    if (memberIt != groupMembers.end())
    {
      for (const auto& member : memberIt->second)
      {
        if (member.get() == seedCell.get())
        {
          continue;
        }
        auto* const memberRenderer = mitk::BaseRenderer::GetInstance(
          member->GetRenderWindow()->GetVtkRenderWindow());
        if (nullptr == memberRenderer)
        {
          continue;
        }

        for (const auto& node : seedSelection)
        {
          if (node.IsNull())
          {
            continue;
          }
          const auto nodeLabel = node->GetName();
          if (node->IsVisible(seedRenderer) != node->IsVisible(memberRenderer))
          {
            emitDivergence(groupName, "visible", nodeLabel, member->GetWidgetName());
          }
          int seedLayer = 0;
          int memberLayer = 0;
          const bool seedHasLayer   = node->GetIntProperty("layer", seedLayer,   seedRenderer);
          const bool memberHasLayer = node->GetIntProperty("layer", memberLayer, memberRenderer);
          if (seedHasLayer && memberHasLayer && seedLayer != memberLayer)
          {
            emitDivergence(groupName, "layer", nodeLabel, member->GetWidgetName());
          }
        }
      }
    }

    // Seed the connector from this group's seed cell, then push the seeded
    // state to every member (including the seed - this is a no-op for it).
    const auto groupIt = nameToInt.find(groupName);
    if (groupIt == nameToInt.end())
    {
      mitkThrow() << "SeedAndNormalizeGroups: group '" << groupName
                  << "' has no engine-internal index assigned.";
    }
    auto* connector = this->GetSyncGroupConnector(groupIt->second);
    if (nullptr == connector)
    {
      mitkThrow() << "SeedAndNormalizeGroups: connector for group '" << groupName
                  << "' is missing.";
    }
    connector->SeedFromMember(seedSelection, seedRenderer);

    if (memberIt != groupMembers.end())
    {
      for (const auto& member : memberIt->second)
      {
        auto* const memberUtility = member->GetUtilityWidget();
        if (nullptr == memberUtility)
        {
          mitkThrow() << "SeedAndNormalizeGroups: member cell '"
                      << member->GetWidgetName().toStdString()
                      << "' in group '" << groupName << "' has no utility widget.";
        }
        connector->SynchronizeWidget(memberUtility->GetNodeSelectionWidget());
      }
    }
  }

  if (warnCapHit)
  {
    MITK_WARN << "ApplyLayout: per-renderer divergence warnings capped at "
              << kWarnCap << "; further occurrences suppressed.";
  }
}

void QmitkMxNMultiWidget::TearDownAllCells()
{
  // ORDER MATTERS — every shared_ptr<QmitkRenderWindowWidget> that outlives
  // the splitter delete causes a double-delete: Qt's deleteChildren on the
  // splitter frees the QObject memory while the dangling shared_ptr later
  // calls 'delete' again. Drop every strong ref we hold BEFORE deleting the
  // splitter:
  //   1. Drop the active-widget pointer (also a shared_ptr).
  //   2. Snapshot only the *keys* of the cell map (a value-copy of the map
  //      itself would copy the shared_ptrs along with it and keep cells
  //      alive past the loop).
  //   3. Remove cells one-by-one through the public name-keyed path. Each
  //      removal disconnects signals and drops the map's shared_ptr; with
  //      no other strong refs left, the cell self-destructs (Qt removes it
  //      from its parent splitter's child list during ~QObject).
  // Only then is the splitter empty and safe to delete.
  this->SetActiveRenderWindowWidget(nullptr);

  std::vector<QString> names;
  for ([[maybe_unused]] const auto& [name, widget] : this->GetRenderWindowWidgets())
  {
    names.push_back(name);
  }
  for (const auto& name : names)
  {
    this->RemoveRenderWindowWidget(name);
  }

  // Drop sync-group connectors. Selection state is not preserved across
  // layout loads; the next ApplyLayout pass re-allocates the groups it
  // needs from the document.
  m_SynchronizedWidgetConnectors.clear();
  m_GroupNameByIndex.clear();

  // Delete the splitter and the layout that held it. The render-window widgets
  // are already gone; the splitter (and any sub-splitters) have no
  // QmitkRenderWindowWidget children left.
  if (auto* oldLayout = this->layout())
  {
    if (oldLayout->count() > 0)
    {
      auto* item = oldLayout->itemAt(0);
      auto* w = (item == nullptr) ? nullptr : item->widget();
      delete w;
    }
    delete oldLayout;
  }
}

void QmitkMxNMultiWidget::RollBackToSingleDefaultCell()
{
  this->TearDownAllCells();
  // SetLayout(1, 1) walks SetLayoutImpl, which auto-creates one cell via the
  // positional CreateRenderWindowWidget path (bare name 'widget0'). That
  // path also assigns the cell to the default sync group 1.
  this->SetLayout(1, 1);
}

void QmitkMxNMultiWidget::ValidateIdsForThisEditor(const nlohmann::json& doc) const
{
  // The schema's id pattern enforces the structural shape `<name>__<bare>`
  // for any consumer that runs JSON-schema validation, but it cannot
  // encode "matches THIS editor instance's `multiWidgetName`": that is a
  // loader-instance-specific constraint. Walk every window node and reject
  // ids that do not start with this editor's required prefix.
  const auto requiredPrefix = (this->GetMultiWidgetName() + kNamespaceDelimiter).toStdString();
  std::function<void(const nlohmann::json&)> walk = [&](const nlohmann::json& node)
  {
    if (!node.is_object() || !node.contains("type") || !node["type"].is_string())
    {
      // Shape errors are surfaced by PrewalkValidate downstream; this pass
      // only checks ids on well-shaped window nodes.
      return;
    }
    const auto type = node["type"].get<std::string>();
    if (type == "split")
    {
      if (node.contains("children") && node["children"].is_array())
      {
        for (const auto& child : node["children"])
        {
          walk(child);
        }
      }
    }
    else if (type == "window")
    {
      if (!node.contains("id") || !node["id"].is_string())
      {
        return;  // Defer to PrewalkValidate for the missing/wrong-type message.
      }
      const auto id = node["id"].get<std::string>();
      if (id.rfind(requiredPrefix, 0) != 0)
      {
        mitkThrow() << "ApplyLayout: window id '" << id
                    << "' does not start with this editor's required prefix '"
                    << requiredPrefix
                    << "'. The layout was written for a different editor instance, "
                    << "or the id is malformed.";
      }
    }
  };

  if (doc.is_object() && doc.contains("root"))
  {
    walk(doc.at("root"));
  }
}

void QmitkMxNMultiWidget::ApplyLayout(const nlohmann::json& doc)
{
  // 'didMutate' guards rollback. As long as we are in the validation phase
  // (no engine state touched yet) a throw must rethrow without rolling back,
  // so a malformed document does not destroy the user's existing layout.
  // Once 'TearDownAllCells()' has run, every subsequent failure must roll
  // back to a usable single-cell state before rethrowing.
  bool didMutate = false;
  try
  {
    // ----- Validation pass (no engine mutation) -----
    if (!doc.is_object())
    {
      mitkThrow() << "Layout document must be a JSON object.";
    }
    if (!doc.contains("version") || !doc["version"].is_string())
    {
      mitkThrow() << "Layout document is missing the 'version' string field; only '2.0' is supported.";
    }
    const auto version = doc["version"].get<std::string>();
    if (version != "2.0")
    {
      // Point v1.x documents at the migration script. This message is read by
      // end users via the QMessageBox load wrapper, so the path it names must
      // match the in-tree script name verbatim - the test 'Version_RejectsAllNonV2'
      // pins this string.
      const bool looksV1 = version.size() >= 2 && version[0] == '1' && version[1] == '.';
      if (looksV1)
      {
        mitkThrow() << "Layout document version is '" << version
                    << "'; only '2.0' is supported. If this is a v1.x layout from "
                    << "before the format change, run "
                    << "'Modules/QtWidgets/resource/migrate-mxn-layout-v1-to-v2.py "
                    << "<file>' to convert it.";
      }
      mitkThrow() << "Layout document version is '" << version
                  << "'; only '2.0' is supported.";
    }
    if (!doc.contains("root"))
    {
      mitkThrow() << "Layout document is missing the 'root' field.";
    }

    // Loader-instance check: every window id must start with this editor's
    // `<multiWidgetName>__` prefix. Run before PrewalkValidate so a misrouted
    // document fails with a precise "wrong editor" message rather than a
    // downstream symptom; both passes run before any engine state is touched.
    this->ValidateIdsForThisEditor(doc);

    // Pre-walk: validate structural shape, collect window ids + referenced
    // group labels, and capture document-order seeding pairs for the
    // post-build group-seeding pass.
    std::set<std::string> seenIds;
    std::set<std::string> referencedGroups;
    std::vector<std::pair<std::string, std::string>> seedingOrder;
    PrewalkValidate(doc.at("root"), seenIds, referencedGroups, seedingOrder);

    // Group resolution. Strict mode = top-level 'groups' present; lazy mode
    // (no 'groups' block) defaults every referenced label to select_all=true.
    const bool strictMode = doc.contains("groups");
    std::map<std::string, bool> groupSelectAll;
    if (strictMode)
    {
      const auto& groupsDict = doc.at("groups");
      if (!groupsDict.is_object())
      {
        mitkThrow() << "Layout 'groups' field must be a JSON object.";
      }
      for (const auto& g : referencedGroups)
      {
        if (!groupsDict.contains(g))
        {
          mitkThrow() << "Layout references group '" << g
                      << "' which is not declared in the 'groups' dict.";
        }
        const auto& entry = groupsDict.at(g);
        if (!entry.is_object())
        {
          mitkThrow() << "Layout 'groups." << g << "' entry must be a JSON object.";
        }
        groupSelectAll[g] = entry.value("select_all", true);
      }
    }
    else
    {
      for (const auto& g : referencedGroups)
      {
        groupSelectAll[g] = true;
      }
    }

    // ----- Tear down existing state -----
    // Invalidate the grid-layout sentinel BEFORE tearing down: from this
    // point on the editor no longer holds a regular grid, so GetRowCount()
    // / GetColumnCount() return 0 to signal callers that the cell set must
    // be enumerated through the cell map. The rollback path also re-runs
    // SetLayout(1, 1), which restores both fields to (1, 1).
    this->ResetGridState();
    this->TearDownAllCells();
    didMutate = true;

    // ----- Allocate engine-internal sync groups -----
    // 'main' (if referenced) pins to engine index 1 to preserve the editor's
    // default-group convention; other names get the next free index in
    // ascending allocation order. Group properties (select_all) are written
    // to each connector before any cell is wired up.
    std::map<std::string, GroupSyncIndexType> nameToInt;
    if (groupSelectAll.find("main") != groupSelectAll.end())
    {
      this->AddSynchronizationGroup(1, "main");
      this->GetSyncGroupConnector(1)->ChangeSelectionMode(groupSelectAll.at("main"));
      nameToInt["main"] = 1;
    }
    else if (!groupSelectAll.empty())
    {
      // No 'main' group declared - the first referenced label takes engine
      // index 1 to preserve the editor's default-group convention. Iterating
      // 'groupSelectAll' (a std::map) walks groups in alphabetical order,
      // matching the assignment order used by the loop below.
      const auto& firstName = groupSelectAll.begin()->first;
      this->AddSynchronizationGroup(1, firstName);
      this->GetSyncGroupConnector(1)->ChangeSelectionMode(groupSelectAll.at(firstName));
      nameToInt[firstName] = 1;
    }
    for (const auto& [groupName, selectAll] : groupSelectAll)
    {
      if (nameToInt.find(groupName) != nameToInt.end())
      {
        continue;
      }
      const auto idx = this->NextFreeSyncGroupIndex();
      this->AddSynchronizationGroup(idx, groupName);
      this->GetSyncGroupConnector(idx)->ChangeSelectionMode(selectAll);
      nameToInt[groupName] = idx;
    }

    // ----- Construct the new cell tree -----
    auto* rootSplitter = this->BuildSplitterFromJsonV2(doc.at("root"), nameToInt, /*parent=*/nullptr);
    auto* hBoxLayout = new QHBoxLayout(this);
    this->setLayout(hBoxLayout);
    hBoxLayout->addWidget(rootSplitter);

    // ----- Group seeding pass + divergence detection -----
    // Make the seed cell of each group authoritative for the group's runtime
    // synchronized state (per the seeding rule documented on the schema's
    // 'groups' field): seed = cell that appears first in document order whose
    // links.selection names that group. After seeding, normalize every other
    // member to the seed's per-renderer values for the keys we fan out
    // (visible, layer). Divergence is reported via MITK_WARN, capped to keep
    // the log usable when many nodes are involved.
    this->SeedAndNormalizeGroups(seedingOrder, nameToInt);

    // Point at the first cell so downstream code that dereferences
    // GetActive... has a sane target after a fresh load.
    auto firstCell = this->GetFirstRenderWindowWidget();
    if (nullptr != firstCell)
    {
      this->SetActiveRenderWindowWidget(firstCell);
    }

    emit UpdateUtilityWidgetViewPlanes();
    this->EnableCrosshair();
    emit LayoutChanged();
  }
  catch (const mitk::Exception&)
  {
    if (didMutate)
    {
      this->RollBackToSingleDefaultCell();
    }
    throw;
  }
  catch (const nlohmann::json::exception& e)
  {
    // Catches the entire 'parse_error / type_error / out_of_range /
    // invalid_iterator / other_error' family - the base class is exactly
    // 'nlohmann::json::exception'.
    if (didMutate)
    {
      this->RollBackToSingleDefaultCell();
    }
    mitkThrow() << "Layout document JSON error: " << e.what();
  }
  catch (const std::exception& e)
  {
    if (didMutate)
    {
      this->RollBackToSingleDefaultCell();
    }
    mitkThrow() << "Layout document load failed: " << e.what();
  }
}

void QmitkMxNMultiWidget::SetDataBasedLayout(const QmitkAbstractNodeSelectionWidget::NodeList& nodes)
{
  // Tear the existing cell tree down first. The previous implementation
  // tried to recycle existing 'widget<i>' cells by positional index, which
  // misses entirely after a v2 layout with custom names is loaded
  // (lookups return null, new cells are appended while the old custom-named
  // ones stay in the map and are then double-deleted by 'delete this->layout()').
  // Mirroring 'ApplyLayout's structure (tear down, allocate groups, build
  // fresh) avoids that hazard regardless of the prior naming scheme.
  this->ResetGridState();
  this->TearDownAllCells();

  auto vSplit = new QSplitter(Qt::Vertical);

  unsigned int rowCounter = 0;
  unsigned int cellCounter = 0;
  for (auto node : nodes)
  {
    rowCounter++;
    // Pre-create the row's synchronization group via the canonical API so that
    // every utility widget's combobox has the entry before SetSyncGroup() runs.
    this->AddSynchronizationGroup(rowCounter);

    auto hSplit = new QSplitter(Qt::Horizontal);
    for (auto viewPlane : { mitk::AnatomicalPlane::Axial, mitk::AnatomicalPlane::Coronal, mitk::AnatomicalPlane::Sagittal })
    {
      // Use the explicit-id overload (which leaves cells unattached) and
      // place each cell directly into its row group via the canonical API,
      // mirroring ApplyLayout. Avoids the churn of the positional overload's
      // initial seeding into group 1 followed by an immediate move.
      const auto id = this->GetMultiWidgetName() + kNamespaceDelimiter
                      + QStringLiteral("widget") + QString::number(cellCounter++);
      auto window = this->CreateRenderWindowWidget(id);
      this->SetSynchronizationGroup(window->GetUtilityWidget()->GetNodeSelectionWidget(), rowCounter);

      window->GetSliceNavigationController()->SetDefaultViewDirection(viewPlane);
      window->GetSliceNavigationController()->Update();
      auto baseRenderer = mitk::BaseRenderer::GetInstance(window->GetRenderWindow()->GetVtkRenderWindow());
      node->SetVisibility(true, baseRenderer);
      mitk::RenderingManager::GetInstance()->InitializeView(baseRenderer->GetRenderWindow(), node->GetData()->GetTimeGeometry());
      hSplit->addWidget(window.get());
      window->show();
    }

    auto* const rowConnector = this->GetSyncGroupConnector(rowCounter);
    rowConnector->ChangeSelectionMode(false);
    rowConnector->ChangeSelection(QList({ node }));

    auto sizes = QList<int>({1, 1, 1});
    hSplit->setSizes(sizes);
    vSplit->addWidget(hSplit);
  }

  auto hBoxLayout = new QHBoxLayout(this);
  this->setLayout(hBoxLayout);
  hBoxLayout->addWidget(vSplit);

  // Deterministic active-cell assignment after rebuild. ResetGridState()
  // nulled out the previous active pointer; without this the editor would
  // be left with a null active cell until the user clicks one.
  if (auto firstCell = this->GetFirstRenderWindowWidget())
  {
    this->SetActiveRenderWindowWidget(firstCell);
  }

  emit UpdateUtilityWidgetViewPlanes();

  this->EnableCrosshair();
  emit LayoutChanged();
}

void QmitkMxNMultiWidget::AddSynchronizationGroup(const GroupSyncIndexType index, const std::string& name)
{
  if (index < 1)
  {
    mitkThrow() << "Invalid synchronization group index '" << index
                << "'. Group index must be >= 1.";
  }

  // Idempotent: an existing group is preserved (no replacement, no extra
  // signal, and the previously recorded name wins over any new one passed in).
  if (m_SynchronizedWidgetConnectors.find(index) != m_SynchronizedWidgetConnectors.end())
  {
    return;
  }

  const auto dataStorage = this->GetDataStorage();
  if (nullptr == dataStorage)
  {
    mitkThrow() << "Cannot create synchronization group '" << index
                << "': no data storage set on the multi widget.";
  }

  const auto noHelperObjects = mitk::NodePredicateAnd::New();
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
  const auto allNodes = dataStorage->GetSubset(noHelperObjects);

  QmitkSynchronizedNodeSelectionWidget::NodeList currentSelection;
  for (const auto& node : *allNodes)
  {
    currentSelection.append(node);
  }

  auto connector = std::make_unique<QmitkSynchronizedWidgetConnector>();
  connector->ChangeSelection(currentSelection);
  m_SynchronizedWidgetConnectors[index] = std::move(connector);

  m_GroupNameByIndex[index] = name.empty()
    ? ((index == 1) ? std::string("main") : ("g_" + std::to_string(index)))
    : name;

  emit SyncGroupAdded(index);
}

void QmitkMxNMultiWidget::SetSynchronizationGroup(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget, const GroupSyncIndexType index)
{
  if (nullptr == synchronizedWidget)
  {
    mitkThrow() << "SetSynchronizationGroup: synchronizedWidget must not be null.";
  }

  // Auto-create on first reference. Add() validates index >= 1 and storage presence.
  if (m_SynchronizedWidgetConnectors.find(index) == m_SynchronizedWidgetConnectors.end())
  {
    this->AddSynchronizationGroup(index);
  }

  const auto old_index = synchronizedWidget->GetSyncGroup();

  if (old_index == index)
  {
    // Already on this group. Connector edge stays (no double-Connect that would
    // double-bump the connection counter); just refresh state from the connector
    // so the freshly attached widget sees the cached selection / select-all mode.
    m_SynchronizedWidgetConnectors[index]->SynchronizeWidget(synchronizedWidget);
    return;
  }

  // For the initial setting of the synchronization, nothing old is there to disconnect.
  if (old_index != -1)
  {
    const auto oldIt = m_SynchronizedWidgetConnectors.find(old_index);
    if (oldIt != m_SynchronizedWidgetConnectors.end())
    {
      oldIt->second->DisconnectWidget(synchronizedWidget);
    }
  }

  synchronizedWidget->SetSyncGroup(index);
  m_SynchronizedWidgetConnectors[index]->ConnectWidget(synchronizedWidget);
  m_SynchronizedWidgetConnectors[index]->SynchronizeWidget(synchronizedWidget);
}

QmitkSynchronizedWidgetConnector* QmitkMxNMultiWidget::GetSyncGroupConnector(const GroupSyncIndexType index) const
{
  const auto it = m_SynchronizedWidgetConnectors.find(index);
  return (it == m_SynchronizedWidgetConnectors.end()) ? nullptr : it->second.get();
}

std::size_t QmitkMxNMultiWidget::GetSyncGroupCount() const
{
  return m_SynchronizedWidgetConnectors.size();
}

QmitkMxNMultiWidget::GroupSyncIndexType QmitkMxNMultiWidget::NextFreeSyncGroupIndex() const
{
  // m_SynchronizedWidgetConnectors is a std::map with int keys, so iteration is
  // in ascending key order. Walk from 1 and return the first gap.
  GroupSyncIndexType candidate = 1;
  for (const auto& entry : m_SynchronizedWidgetConnectors)
  {
    if (entry.first != candidate)
    {
      break;
    }
    ++candidate;
  }
  return candidate;
}

void QmitkMxNMultiWidget::OnCreateNewSyncGroupRequested(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget)
{
  // Guard against the slot firing before the editor has a data storage attached
  // (e.g. during teardown, or if SetDataStorage(nullptr) was called after init).
  // AddSynchronizationGroup would otherwise throw, and Qt slots must not let
  // exceptions escape into the event dispatcher.
  if (nullptr == this->GetDataStorage())
  {
    MITK_WARN << "Ignoring 'create new synchronization group' request: no data storage set on the multi widget.";
    return;
  }

  // 'SetSynchronizationGroup' auto-creates the group via 'AddSynchronizationGroup'
  // on first reference, so a separate Add call here would be redundant.
  const auto next = this->NextFreeSyncGroupIndex();
  this->SetSynchronizationGroup(synchronizedWidget, next);
}
