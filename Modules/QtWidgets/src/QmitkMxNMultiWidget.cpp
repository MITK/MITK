/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNMultiWidget.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandlerSynchronized.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

// vtk
#include <vtkCamera.h>
#include <vtkRenderer.h>

// mitk qt widget
#include <QmitkMultiWidgetLayoutManager.h>
#include <QmitkMxNCellOverlay.h>
#include <QmitkRenderWindowProximity.h>
#include <QmitkMxNSyncBarcodeWidget.h>
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
  // The no-underscore rule on the editor-name segment is what makes the
  // first-`__` split into editor-name and bare-id unambiguous.
  const QRegularExpression EDITOR_NAME_PATTERN(QStringLiteral("^[A-Za-z][A-Za-z0-9.-]*$"));

  const QString NAMESPACE_DELIMITER = QStringLiteral("__");

  // Window-id and group-name patterns mirror mxn-layout-v2.schema.json.
  // Enforced by PrewalkValidate so callers that do not run a JSON-schema
  // validator still fail loudly at load rather than letting unsafe
  // characters reach REST URLs or property-context keys downstream.
  const QRegularExpression WINDOW_ID_PATTERN(
    QStringLiteral("^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$"));
  const QRegularExpression GROUP_NAME_PATTERN(
    QStringLiteral("^[A-Za-z0-9_.-]+$"));

  // Persisted group hue (`groups.<id>.color`). A cosmetic field: a value
  // failing this pattern is ignored with a warning, never a load failure.
  const QRegularExpression GROUP_COLOR_PATTERN(
    QStringLiteral("^#[0-9A-Fa-f]{6}$"));


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

  // Group name each Synchronize(true) macro link uses. An ordinary group in
  // every respect (shared namespace, serialized like any other); the constant
  // only pins the label the macro rewrites.
  const std::string SYNCHRONIZE_MACRO_GROUP = "sync";

  // Default group hues, handed out by first-registration order (wrapping).
  // Colorblind-aware and desaturated enough to stay legible on the dark
  // Workbench style; the only saturated colors the viewport furniture uses.
  constexpr std::array<const char*, 8> GROUP_HUE_PALETTE{
    "#E1707A",  // rose
    "#6FA8DC",  // sky
    "#93C47D",  // moss
    "#F0B26B",  // amber
    "#B08FD9",  // violet
    "#76C7C0",  // teal
    "#C9C97A",  // olive
    "#D08FB8"   // orchid
  };

  std::size_t DimensionIndex(QmitkMxNSyncDimension dimension)
  {
    return static_cast<std::size_t>(dimension);
  }

  // One parsed `links.<dim>` entry of a window node (v3 object or string form).
  struct NavLinkSpec
  {
    QmitkMxNSyncDimension dimension;
    std::string group;
    QmitkMxNMultiWidget::SyncOffset offset;
  };

  // Everything the validation pre-walk collects for the later engine passes.
  struct PrewalkResult
  {
    std::set<std::string> windowIds;
    std::set<std::string> selectionGroups;
    // (windowId, selectionGroup) in pre-order traversal order (splits'
    // children walked in array order); the selection seeding pass uses the
    // first cell per group as that group's seed.
    std::vector<std::pair<std::string, std::string>> seedingOrder;
    // Per-window navigation links in the same pre-order; document order
    // defines each navigation group's seed cell as well.
    std::vector<std::pair<std::string, std::vector<NavLinkSpec>>> navLinks;
    std::set<std::string> navGroups;
  };

  void ValidateGroupNamePattern(const std::string& windowId, const std::string& groupName)
  {
    if (!GROUP_NAME_PATTERN.match(QString::fromStdString(groupName)).hasMatch())
    {
      mitkThrow() << "Layout window '" << windowId
                  << "' references group name '" << groupName
                  << "' which does not match the required pattern '"
                  << GROUP_NAME_PATTERN.pattern().toStdString()
                  << "' (URL-segment-safe).";
    }
  }

  // Group-name target of a link value in either the string shorthand or the
  // object form. Only call on values whose shape has been validated.
  std::string LinkTarget(const nlohmann::json& linkValue)
  {
    return linkValue.is_string() ? linkValue.get<std::string>()
                                 : linkValue.at("target").get<std::string>();
  }

  // Validates one v3 link value and extracts (group, offset). Enforces the
  // closed modifier set: `target` everywhere; `offset` only on slice / zoom /
  // pan, each with its dimension-typed shape. `dimension` is empty for
  // `selection`, which accepts no modifiers at all.
  std::pair<std::string, QmitkMxNMultiWidget::SyncOffset> ParseV3LinkValue(
    const std::string& windowId,
    const std::string& key,
    std::optional<QmitkMxNSyncDimension> dimension,
    const nlohmann::json& value)
  {
    if (value.is_string())
    {
      const auto group = value.get<std::string>();
      ValidateGroupNamePattern(windowId, group);
      return { group, {} };
    }
    if (!value.is_object())
    {
      mitkThrow() << "Layout window '" << windowId << "' has a 'links." << key
                  << "' value that is neither a group-name string nor a link object.";
    }
    if (!value.contains("target") || !value["target"].is_string())
    {
      mitkThrow() << "Layout window '" << windowId << "' has a 'links." << key
                  << "' object without the required 'target' string.";
    }
    const auto group = value["target"].get<std::string>();
    ValidateGroupNamePattern(windowId, group);

    QmitkMxNMultiWidget::SyncOffset offset;
    for (auto it = value.begin(); it != value.end(); ++it)
    {
      if (it.key() == "target")
      {
        continue;
      }
      if (it.key() != "offset")
      {
        mitkThrow() << "Layout window '" << windowId << "' has unknown modifier '"
                    << it.key() << "' on 'links." << key << "'.";
      }
      if (!dimension.has_value())
      {
        mitkThrow() << "Layout window '" << windowId
                    << "': 'links.selection' does not accept an 'offset' modifier.";
      }
      switch (*dimension)
      {
        case QmitkMxNSyncDimension::Slice:
          if (!it->is_number_integer())
          {
            mitkThrow() << "Layout window '" << windowId
                        << "': 'links.slice' offset must be an integer (slice steps).";
          }
          offset = it->get<int>();
          break;
        case QmitkMxNSyncDimension::Zoom:
          if (!it->is_number() || it->get<double>() <= 0.0)
          {
            mitkThrow() << "Layout window '" << windowId
                        << "': 'links.zoom' offset must be a number > 0 (multiplicative factor).";
          }
          offset = it->get<double>();
          break;
        case QmitkMxNSyncDimension::Pan:
          if (!it->is_array() || it->size() != 2 || !(*it)[0].is_number() || !(*it)[1].is_number())
          {
            mitkThrow() << "Layout window '" << windowId
                        << "': 'links.pan' offset must be an array of two numbers (in-plane world-mm).";
          }
          {
            mitk::Vector2D panOffset;
            panOffset[0] = (*it)[0].get<double>();
            panOffset[1] = (*it)[1].get<double>();
            offset = panOffset;
          }
          break;
        default:
          mitkThrow() << "Layout window '" << windowId << "': dimension 'links." << key
                      << "' does not accept an 'offset' modifier.";
      }
    }
    return { group, offset };
  }

  // Pre-walks a 'root' subtree to validate structural shape and collect
  // per-window ids, referenced group labels, and (v3) navigation links.
  // Throws on missing required field, type mismatch on a known field, or
  // duplicate window id. Does not mutate engine state.
  //
  // Version scope: for a "3.0" document the `links` object is closed (unknown
  // keys, unknown modifiers, and misplaced or mistyped offsets throw). A
  // "2.0" document keeps its historical lenient behavior - `links.selection`
  // must be a bare string and every other link key is silently ignored - so
  // existing v2 files load unchanged, including files with a typo'd key.
  void PrewalkValidate(const nlohmann::json& node, bool isV3, PrewalkResult& result)
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
        PrewalkValidate(child, isV3, result);
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
      if (!WINDOW_ID_PATTERN.match(QString::fromStdString(id)).hasMatch())
      {
        mitkThrow() << "Layout window id '" << id
                    << "' does not match the required pattern '"
                    << WINDOW_ID_PATTERN.pattern().toStdString()
                    << "' (URL-segment-safe, qualified `<editor_name>__<bare_id>`).";
      }
      if (!result.windowIds.insert(id).second)
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
      if (!links.contains("selection"))
      {
        mitkThrow() << "Layout window '" << id
                    << "' is missing the required 'links.selection' string.";
      }

      std::string selectionGroup;
      if (isV3)
      {
        selectionGroup = ParseV3LinkValue(id, "selection", std::nullopt, links["selection"]).first;
      }
      else
      {
        if (!links["selection"].is_string())
        {
          mitkThrow() << "Layout window '" << id
                      << "' is missing the required 'links.selection' string.";
        }
        selectionGroup = links["selection"].get<std::string>();
        ValidateGroupNamePattern(id, selectionGroup);
      }
      result.selectionGroups.insert(selectionGroup);
      result.seedingOrder.emplace_back(id, selectionGroup);

      std::vector<NavLinkSpec> navSpecs;
      for (auto it = links.begin(); it != links.end(); ++it)
      {
        if (it.key() == "selection")
        {
          continue;
        }
        if (!isV3)
        {
          continue;
        }
        const auto dimension = QmitkMxNSyncDimensionFromLinkKey(it.key());
        if (!dimension.has_value())
        {
          mitkThrow() << "Layout window '" << id << "' has unknown link key 'links."
                      << it.key() << "'.";
        }
        auto [group, offset] = ParseV3LinkValue(id, it.key(), dimension, it.value());
        result.navGroups.insert(group);
        navSpecs.push_back({ *dimension, std::move(group), offset });
      }
      if (!navSpecs.empty())
      {
        result.navLinks.emplace_back(id, std::move(navSpecs));
      }
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
  // Reject malformed names at construction; once stored, an `_` in the
  // editor name would later produce schema-invalid ids or break the
  // first-`__` split rule that separates editor-name from bare-id segments.
  if (!EDITOR_NAME_PATTERN.match(multiWidgetName).hasMatch())
  {
    mitkThrow() << "QmitkMxNMultiWidget: multiWidgetName '"
                << multiWidgetName.toStdString()
                << "' does not match the required pattern '"
                << EDITOR_NAME_PATTERN.pattern().toStdString()
                << "'. Editor names must start with a letter, contain no '_', "
                << "and use only the alphabet [A-Za-z0-9.-].";
  }

  // A cell's frame carries its group identity (mono group hue, else neutral),
  // and the per-cell utility-strip sync barcodes track the same per-dimension
  // membership the layout editor shows; refresh both whenever the links change
  // or the layout (and thus the set of cells) does.
  connect(this, &QmitkMxNMultiWidget::LayoutChanged, this, &QmitkMxNMultiWidget::RefreshFrameColors);
  connect(this, &QmitkMxNMultiWidget::SyncLinksChanged, this, &QmitkMxNMultiWidget::RefreshFrameColors);
  connect(this, &QmitkMxNMultiWidget::LayoutChanged, this, &QmitkMxNMultiWidget::RefreshSyncBarcodes);
  connect(this, &QmitkMxNMultiWidget::SyncLinksChanged, this, &QmitkMxNMultiWidget::RefreshSyncBarcodes);
}

QmitkMxNMultiWidget::~QmitkMxNMultiWidget()
{
}

void QmitkMxNMultiWidget::InitializeMultiWidget()
{

  AddSynchronizationGroup(1);
  SetLayout(1, 1);
  this->InstallSynchronizedHandler();
}

void QmitkMxNMultiWidget::InstallSynchronizedHandler()
{
  auto handler = std::make_unique<mitk::DisplayActionEventHandlerSynchronized>();

  auto navPredicate = [this](QmitkMxNSyncDimension dimension)
  {
    return mitk::DisplayActionEventFunctions::TargetPredicate(
      [this, dimension](const mitk::BaseRenderer* sender, const mitk::BaseRenderer* target)
      {
        return this->IsNavTarget(dimension, sender, target);
      });
  };
  mitk::DisplayActionEventHandlerSynchronized::Predicates predicates;
  predicates.pan = navPredicate(QmitkMxNSyncDimension::Pan);
  predicates.zoom = navPredicate(QmitkMxNSyncDimension::Zoom);
  predicates.slice = navPredicate(QmitkMxNSyncDimension::Slice);
  predicates.crosshair = navPredicate(QmitkMxNSyncDimension::Crosshair);
  predicates.levelWindow = [this](const mitk::BaseRenderer* sender, const mitk::BaseRenderer* target)
  {
    return this->IsWindowingTarget(sender, target);
  };
  handler->SetPredicates(predicates);
  SetDisplayActionEventHandler(std::move(handler));

  auto displayActionEventHandler = GetDisplayActionEventHandler();
  if (nullptr != displayActionEventHandler)
  {
    displayActionEventHandler->InitActions(this->GetMultiWidgetName().toStdString());
  }
}

bool QmitkMxNMultiWidget::IsNavTarget(QmitkMxNSyncDimension dimension,
                                      const mitk::BaseRenderer* sender,
                                      const mitk::BaseRenderer* target) const
{
  // Only this editor's cells participate, in both roles: every editor's
  // broadcast observes every interaction event application-wide, so
  // admitting foreign renderers here would double-handle windows that
  // their own editor's handler already serves.
  const auto editorPrefix = this->GetMultiWidgetName() + NAMESPACE_DELIMITER;
  const auto senderId = QString::fromUtf8(sender->GetName());
  const auto targetId = QString::fromUtf8(target->GetName());
  if (!senderId.startsWith(editorPrefix) || !targetId.startsWith(editorPrefix))
  {
    return false;
  }

  // A cell always receives its own gesture; an unlinked cell is thereby a
  // singleton group and behaves desynchronized without extra bookkeeping.
  if (sender == target)
  {
    return true;
  }

  const auto senderLinks = m_CellSyncLinks.find(senderId);
  const auto targetLinks = m_CellSyncLinks.find(targetId);
  if (senderLinks == m_CellSyncLinks.end() || targetLinks == m_CellSyncLinks.end())
  {
    return false;
  }
  const auto& senderGroup = senderLinks->second.groups[DimensionIndex(dimension)];
  const auto& targetGroup = targetLinks->second.groups[DimensionIndex(dimension)];
  return senderGroup.has_value() && targetGroup.has_value() && *senderGroup == *targetGroup;
}

bool QmitkMxNMultiWidget::IsWindowingTarget(const mitk::BaseRenderer* sender,
                                            const mitk::BaseRenderer* target) const
{
  const auto editorPrefix = this->GetMultiWidgetName() + NAMESPACE_DELIMITER;
  const auto senderId = QString::fromUtf8(sender->GetName());
  const auto targetId = QString::fromUtf8(target->GetName());
  if (!senderId.startsWith(editorPrefix) || !targetId.startsWith(editorPrefix))
  {
    return false;
  }

  const auto senderLinks = m_CellSyncLinks.find(senderId);
  const auto targetLinks = m_CellSyncLinks.find(targetId);
  if (senderLinks == m_CellSyncLinks.end() || targetLinks == m_CellSyncLinks.end())
  {
    return false;
  }
  const auto& senderGroup = senderLinks->second.groups[DimensionIndex(QmitkMxNSyncDimension::Windowing)];
  const auto& targetGroup = targetLinks->second.groups[DimensionIndex(QmitkMxNSyncDimension::Windowing)];
  return senderGroup.has_value() && targetGroup.has_value() && *senderGroup == *targetGroup;
}

void QmitkMxNMultiWidget::Synchronize(bool synchronized)
{
  // Pure membership rewrite: the handler stays installed and its predicates
  // read the link map live, so no re-wiring is needed. Deliberately no
  // convergence (unlike SetSyncLink) - the toggle couples views in place.
  m_SynchronizeMacroActive = synchronized;
  if (synchronized)
  {
    this->RegisterGroupForHue(SYNCHRONIZE_MACRO_GROUP);
  }
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    auto& links = m_CellSyncLinks[windowId];
    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
    {
      links.groups[DimensionIndex(dimension)] =
        synchronized ? std::optional<std::string>(SYNCHRONIZE_MACRO_GROUP) : std::nullopt;
    }
  }
  this->RefreshSyncControls();
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

  QmitkAbstractMultiWidget::SetActiveRenderWindowWidget(activeRenderWindowWidget);

  // The frame color carries group identity, not active-ness; the active cell is
  // marked by a color-independent inner ring the overlay paints. Restyle every
  // cell and repaint the overlays so the ring follows the active-cell change.
  this->RefreshFrameColors();
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
      const auto id = this->GetMultiWidgetName() + NAMESPACE_DELIMITER + QStringLiteral("widget") + QString::number(i);
      if (nullptr != this->GetRenderWindowWidget(id))
      {
        this->RemoveRenderWindowWidget(id);
        m_CellSyncLinks.erase(id);
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

  // Layout-tracking furniture (layout editor, seams) follows this signal;
  // without it a shrink leaves them rendering removed cells.
  emit LayoutChanged();
}

QmitkAbstractMultiWidget::RenderWindowWidgetPointer QmitkMxNMultiWidget::CreateRenderWindowWidget()
{
  // Pick the smallest non-negative 'i' such that
  // '<multiWidgetName>__widget<i>' is not already registered in this editor.
  // Replaces the old 'widget<count>' form, which silently collided when
  // custom-id'd cells already used the same index (e.g. existing
  // {widget0, widget3} + adding a 4th cell would have produced 'widget3'
  // again, which std::map::insert silently rejects).
  const auto prefix = this->GetMultiWidgetName() + NAMESPACE_DELIMITER + QStringLiteral("widget");
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

  // Guards in-process API misuse (an unqualified bare name slipping through);
  // document-driven creation is additionally gated by ValidateIdsForThisEditor.
  const auto requiredPrefix = this->GetMultiWidgetName() + NAMESPACE_DELIMITER;
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
  // The cell's plane label is drawn by the Qt overlay in the same layer as the
  // slice readout (so the two align); the VTK corner annotation, which would
  // otherwise show the internal cell id, is blanked to avoid a second, stale
  // label in the OpenGL scene.
  renderWindowWidget->SetCornerAnnotationText(std::string());
  this->AddRenderWindowWidget(id, renderWindowWidget);

  auto renderWindow = renderWindowWidget->GetRenderWindow();

  QmitkRenderWindowUtilityWidget* utilityWidget = new QmitkRenderWindowUtilityWidget(this, renderWindow, this->GetDataStorage());
  renderWindowWidget->AddUtilityWidget(utilityWidget);

  // Viewport furniture: one proximity controller and one composite overlay
  // per cell, both children of the cell (they die with it; the editor
  // reaches them via findChild, so no pointer bookkeeping can go stale).
  auto* proximity = new QmitkRenderWindowProximity(renderWindowWidget.get(), renderWindowWidget.get());
  proximity->AddEventSource(renderWindow);
  proximity->SetSuppressed(m_CleanView);

  auto* cellOverlay = new QmitkMxNCellOverlay(renderWindowWidget.get(), this, proximity);
  cellOverlay->SetReadoutVisible(m_LevelWindowReadoutVisible);
  cellOverlay->SetCleanView(m_CleanView);
  cellOverlay->SetNavigatorExpanded(m_NavigatorExpanded);

  // The utility row auto-hides into a top-edge strip; approaching the top
  // reveals it as a floating panel, so the render window never resizes.
  renderWindowWidget->SetUtilityWidgetAutoHide(true);
  proximity->AddEventSource(utilityWidget);

  connect(utilityWidget, &QmitkRenderWindowUtilityWidget::CleanViewToggled,
          this, &QmitkMxNMultiWidget::SetCleanView);
  connect(this, &QmitkMxNMultiWidget::CleanViewChanged,
          utilityWidget, &QmitkRenderWindowUtilityWidget::SetCleanViewChecked);
  utilityWidget->SetCleanViewChecked(m_CleanView);

  connect(utilityWidget, &QmitkRenderWindowUtilityWidget::NavigatorToggled,
          this, &QmitkMxNMultiWidget::SetNavigatorExpanded);
  connect(this, &QmitkMxNMultiWidget::NavigatorExpandedChanged,
          utilityWidget, &QmitkRenderWindowUtilityWidget::SetNavigatorChecked);
  utilityWidget->SetNavigatorChecked(m_NavigatorExpanded);

  // The cell's data-selection group is now one axis among the others: it is
  // assigned from the layout editor and shown in the sync barcode, so the
  // utility widget no longer carries a group combobox to wire up. The
  // authoritative store is the node selection widget, set via
  // 'SetSynchronizationGroup' during layout construction / editor edits.

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

  // The cell's "Sync" button opens the editor-wide layout editor; the view
  // hosting it lives above this module, so the request is only relayed.
  connect(utilityWidget, &QmitkRenderWindowUtilityWidget::LayoutEditorRequested,
          this, &QmitkMxNMultiWidget::LayoutEditorRequested);

  // The Synchronize macro covers every cell of the editor, including cells
  // created while it is active - not only those present at toggle time.
  if (m_SynchronizeMacroActive)
  {
    auto& links = m_CellSyncLinks[id];
    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
    {
      links.groups[DimensionIndex(dimension)] = SYNCHRONIZE_MACRO_GROUP;
    }
  }
  this->RefreshSyncControls();

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
  // Strict mode requires every referenced group declared, across all
  // dimensions (shared namespace). Groups referenced only by navigation
  // links carry no persisted per-group state and are declared as empty
  // entries; a name that also serves as a selection group already has its
  // entry (with select_all) from the loop above.
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    for (const auto& navGroup : this->GetSyncGroupNames(dimension))
    {
      if (!groupsJson.contains(navGroup))
      {
        groupsJson[navGroup] = nlohmann::json::object();
      }
    }
  }
  // Cosmetic per-group fields, emitted only when set so a document that
  // never carried them stays shaped as before.
  for (auto& [name, entry] : groupsJson.items())
  {
    const auto color = m_GroupColors.find(name);
    if (color != m_GroupColors.end())
    {
      entry["color"] = color->second;
    }
    const auto displayName = m_GroupDisplayNames.find(name);
    if (displayName != m_GroupDisplayNames.end() && !displayName->second.empty())
    {
      entry["name"] = displayName->second;
    }
  }

  nlohmann::json doc;
  doc["version"] = "3.0";
  // Round-trip the optional `name` from the document that produced the
  // current state; emit the field only when set so empty strings never
  // land on disk.
  if (!m_LayoutName.empty())
  {
    doc["name"] = m_LayoutName;
  }
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

      nlohmann::json linksJson{ { "selection", descriptor.selectionGroup.toStdString() } };
      const auto linksIt = m_CellSyncLinks.find(descriptor.id);
      if (linksIt != m_CellSyncLinks.end())
      {
        const auto& links = linksIt->second;
        for (const auto dimension : QmitkMxNAllSyncDimensions)
        {
          const auto& group = links.groups[DimensionIndex(dimension)];
          if (!group.has_value())
          {
            continue;
          }
          // Identity offsets serialize as the string shorthand; only a real
          // offset needs the object form.
          nlohmann::json linkValue = *group;
          switch (dimension)
          {
            case QmitkMxNSyncDimension::Slice:
              if (0 != links.sliceOffset)
              {
                linkValue = nlohmann::json{ { "target", *group }, { "offset", links.sliceOffset } };
              }
              break;
            case QmitkMxNSyncDimension::Zoom:
              if (1.0 != links.zoomOffset)
              {
                linkValue = nlohmann::json{ { "target", *group }, { "offset", links.zoomOffset } };
              }
              break;
            case QmitkMxNSyncDimension::Pan:
              if (0.0 != links.panOffset[0] || 0.0 != links.panOffset[1])
              {
                linkValue = nlohmann::json{
                  { "target", *group },
                  { "offset", nlohmann::json::array({ links.panOffset[0], links.panOffset[1] }) } };
              }
              break;
            default:
              break;
          }
          linksJson[QmitkMxNSyncDimensionToLinkKey(dimension)] = linkValue;
        }
      }
      childJson["links"] = linksJson;
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

QSplitter* QmitkMxNMultiWidget::BuildSplitterFromJson(
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
        auto* sub = this->BuildSplitterFromJson(child, nameToInt, split.get());
        split->addWidget(sub);
      }
      else  // "window"
      {
        // Id passes through verbatim - validation already happened upstream
        // (PrewalkValidate + ValidateIdsForThisEditor).
        const auto id = QString::fromStdString(child["id"].get<std::string>());
        const auto viewDirection = ParseViewDirection(child["view_direction"].get<std::string>());
        const auto groupName = LinkTarget(child["links"]["selection"]);
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
  // The divergence path here is uncovered by CI today by design: fresh
  // layouts have no divergence to detect. Once scene-after-layout reseeding
  // is implemented, an integration test should exercise this lambda with
  // a scene that injects per-renderer divergence after the layout was applied.
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
  m_GroupHueOrder.clear();
  m_GroupDisplayNames.clear();
  m_GroupColors.clear();

  // Per-cell synchronization links die with their cells; the document (or
  // the user) defines the links of the next cell set. The macro state does
  // not survive either - a loaded document is authoritative for coupling.
  m_CellSyncLinks.clear();
  m_SynchronizeMacroActive = false;

  // The rolled-back single-default-cell state has no preset name to claim.
  m_LayoutName.clear();

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
  // The schema's id pattern cannot encode "matches THIS editor instance's
  // `multiWidgetName`" - that is a loader-instance constraint. Walk every
  // window node and reject ids that do not start with this editor's prefix.
  const auto requiredPrefix = (this->GetMultiWidgetName() + NAMESPACE_DELIMITER).toStdString();
  std::function<void(const nlohmann::json&)> walk = [&](const nlohmann::json& node)
  {
    if (!node.is_object() || !node.contains("type") || !node["type"].is_string())
    {
      // Shape errors are surfaced by PrewalkValidate; this pass focuses on
      // the prefix check and only inspects well-shaped window nodes.
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
      mitkThrow() << "Layout document is missing the 'version' string field; only '2.0' and '3.0' are supported.";
    }
    const auto version = doc["version"].get<std::string>();
    if (version != "2.0" && version != "3.0")
    {
      // Point v1.x documents at the migration documentation. This message
      // surfaces to end users via the QMessageBox load wrapper, so it stays
      // neutral about install location: the developer documentation is the
      // canonical reference for the migration tool. The phrase 'migration
      // tool' is pinned by the 'Version_RejectsAllNonV2' test for v1.x
      // versions.
      const bool looksV1 = version.size() >= 2 && version[0] == '1' && version[1] == '.';
      if (looksV1)
      {
        mitkThrow() << "Layout document version is '" << version
                    << "'; only '2.0' and '3.0' are supported. If this is a v1.x layout "
                    << "from before the format change, see the MxN layout "
                    << "developer documentation for the migration tool "
                    << "(Modules/QtWidgets/resource/migrate-mxn-layout-v1-to-v2.py).";
      }
      mitkThrow() << "Layout document version is '" << version
                  << "'; only '2.0' and '3.0' are supported.";
    }
    const bool isV3 = ("3.0" == version);
    if (!doc.contains("root"))
    {
      mitkThrow() << "Layout document is missing the 'root' field.";
    }

    // Run the prefix check before PrewalkValidate so a misrouted document
    // fails with a precise "wrong editor" message rather than a downstream
    // symptom. Both passes run before any engine state is mutated.
    this->ValidateIdsForThisEditor(doc);

    // Pre-walk: validate structural shape, collect window ids + referenced
    // group labels + navigation links, and capture document-order seeding
    // pairs for the post-build group-seeding pass.
    PrewalkResult prewalk;
    PrewalkValidate(doc.at("root"), isV3, prewalk);
    const auto& seedingOrder = prewalk.seedingOrder;

    // Group resolution. Strict mode = top-level 'groups' present; lazy mode
    // (no 'groups' block) defaults every referenced label to select_all=true.
    // The declaration requirement of strict mode covers every dimension
    // (shared group namespace); 'select_all' only matters for groups that a
    // cell references via links.selection - it is dormant otherwise.
    const bool strictMode = doc.contains("groups");
    std::map<std::string, bool> groupSelectAll;
    std::map<std::string, std::string> groupColors;
    std::map<std::string, std::string> groupDisplayNames;
    if (strictMode)
    {
      const auto& groupsDict = doc.at("groups");
      if (!groupsDict.is_object())
      {
        mitkThrow() << "Layout 'groups' field must be a JSON object.";
      }
      // Covers group names declared in 'groups' but never referenced by a
      // cell - those would not pass through PrewalkValidate's per-cell loop.
      for (auto it = groupsDict.begin(); it != groupsDict.end(); ++it)
      {
        if (!GROUP_NAME_PATTERN.match(QString::fromStdString(it.key())).hasMatch())
        {
          mitkThrow() << "Layout 'groups' contains key '" << it.key()
                      << "' which does not match the required pattern '"
                      << GROUP_NAME_PATTERN.pattern().toStdString()
                      << "' (URL-segment-safe).";
        }
        // Cosmetic per-group fields. Purely presentational, so malformed
        // values are ignored with a warning - a color must never make a
        // layout unloadable.
        if (it.value().is_object())
        {
          const auto& entry = it.value();
          if (entry.contains("color"))
          {
            const auto& color = entry.at("color");
            if (color.is_string()
                && GROUP_COLOR_PATTERN.match(QString::fromStdString(color.get<std::string>())).hasMatch())
            {
              groupColors[it.key()] = color.get<std::string>();
            }
            else
            {
              MITK_WARN << "Ignoring malformed 'groups." << it.key()
                        << ".color' (expected '#RRGGBB'); using the default hue.";
            }
          }
          if (entry.contains("name"))
          {
            const auto& displayName = entry.at("name");
            if (displayName.is_string() && !displayName.get<std::string>().empty())
            {
              groupDisplayNames[it.key()] = displayName.get<std::string>();
            }
            else
            {
              MITK_WARN << "Ignoring malformed 'groups." << it.key()
                        << ".name' (expected a non-empty string); using the group id.";
            }
          }
        }
      }
      for (const auto& g : prewalk.selectionGroups)
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
      for (const auto& g : prewalk.navGroups)
      {
        if (!groupsDict.contains(g))
        {
          mitkThrow() << "Layout references group '" << g
                      << "' which is not declared in the 'groups' dict.";
        }
        if (!groupsDict.at(g).is_object())
        {
          mitkThrow() << "Layout 'groups." << g << "' entry must be a JSON object.";
        }
      }
    }
    else
    {
      for (const auto& g : prewalk.selectionGroups)
      {
        groupSelectAll[g] = true;
      }
    }

    // Read the optional `name` here, after validation but before mutation,
    // so a malformed document does not pollute the existing stash.
    std::string stashedName;
    if (doc.contains("name") && doc.at("name").is_string())
    {
      stashedName = doc.at("name").get<std::string>();
    }

    // ----- Tear down existing state -----
    // Invalidate the grid-layout sentinel BEFORE tearing down: from this
    // point on the editor no longer holds a regular grid, so GetRowCount()
    // / GetColumnCount() return 0 to signal callers that the cell set must
    // be enumerated through the cell map. The rollback path also re-runs
    // SetLayout(1, 1), which restores both fields to (1, 1).
    this->ResetGridState();
    this->TearDownAllCells();
    m_LayoutName = stashedName;
    // Install the cosmetic group state before any group is allocated, so
    // 'SyncGroupAdded' already carries the document's display names.
    m_GroupColors = std::move(groupColors);
    m_GroupDisplayNames = std::move(groupDisplayNames);
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
    auto* rootSplitter = this->BuildSplitterFromJson(doc.at("root"), nameToInt, /*parent=*/nullptr);
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

    // ----- Navigation links (v3) -----
    // Applied in document order, so the first cell to link a group becomes
    // its seed and every later member converges to it on joining (windows
    // without world geometry yet defer to 'ReconvergeSyncGroup').
    for (const auto& [windowId, specs] : prewalk.navLinks)
    {
      for (const auto& spec : specs)
      {
        this->SetSyncLink(QString::fromStdString(windowId), spec.dimension, spec.group, spec.offset);
      }
    }
    this->RefreshSyncControls();

    // Point at the first cell so downstream code that dereferences
    // GetActive... has a sane target after a fresh load.
    auto firstCell = this->GetFirstRenderWindowWidget();
    if (nullptr != firstCell)
    {
      this->SetActiveRenderWindowWidget(firstCell);
    }

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
      const auto id = this->GetMultiWidgetName() + NAMESPACE_DELIMITER
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
  this->RegisterGroupForHue(m_GroupNameByIndex[index]);

  emit SyncGroupAdded(index, QString::fromStdString(this->GetSyncGroupDisplayName(m_GroupNameByIndex[index])));
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

QmitkMxNMultiWidget::GroupSyncIndexType
QmitkMxNMultiWidget::EnsureSelectionGroupIndex(const std::string& group)
{
  // Reuse the existing selection connector for this string id if there is one
  // (a "+"/document group, or one this method allocated earlier).
  for (const auto& [index, name] : m_GroupNameByIndex)
  {
    if (name == group)
    {
      return index;
    }
  }
  // Otherwise allocate one and remember it so it can be reclaimed when empty.
  const auto index = this->NextFreeSyncGroupIndex();
  this->AddSynchronizationGroup(index, group);
  m_SelectionGroupsAllocatedForLinks.insert(index);
  return index;
}

void QmitkMxNMultiWidget::ReclaimSelectionGroupIfEmpty(GroupSyncIndexType index)
{
  // Only connectors this method allocated are reclaimed; a document- or
  // default-seed group persists even when empty (it stays assignable).
  if (m_SelectionGroupsAllocatedForLinks.find(index) == m_SelectionGroupsAllocatedForLinks.end())
  {
    return;
  }
  for (const auto& [windowId, cell] : this->GetRenderWindowWidgets())
  {
    auto* utility = cell->GetUtilityWidget();
    if (nullptr != utility && utility->GetSyncGroup() == index)
    {
      return;  // still has a member
    }
  }
  m_SynchronizedWidgetConnectors.erase(index);
  m_GroupNameByIndex.erase(index);
  m_SelectionGroupsAllocatedForLinks.erase(index);
}

void QmitkMxNMultiWidget::SetCellSelectionGroup(const QString& windowId, const std::string& group)
{
  const auto cell = this->GetRenderWindowWidget(windowId);
  if (nullptr == cell || nullptr == cell->GetUtilityWidget())
  {
    return;
  }
  auto* widget = cell->GetUtilityWidget()->GetNodeSelectionWidget();
  if (nullptr == widget)
  {
    return;
  }

  const auto newIndex = this->EnsureSelectionGroupIndex(group);
  const auto oldIndex = widget->GetSyncGroup();
  if (oldIndex == newIndex)
  {
    return;
  }

  this->SetSynchronizationGroup(widget, newIndex);
  this->ReclaimSelectionGroupIfEmpty(oldIndex);
}

void QmitkMxNMultiWidget::ClearCellSelectionGroup(const QString& windowId)
{
  const auto cell = this->GetRenderWindowWidget(windowId);
  if (nullptr == cell || nullptr == cell->GetUtilityWidget())
  {
    return;
  }
  auto* widget = cell->GetUtilityWidget()->GetNodeSelectionWidget();
  if (nullptr == widget)
  {
    return;
  }

  // Index 1 is the default selection group every cell starts in; unlinking a
  // cell's selection means returning it to that default (there is no "no
  // selection group" state - every cell always belongs to one).
  constexpr GroupSyncIndexType defaultGroup = 1;
  const auto oldIndex = widget->GetSyncGroup();
  if (oldIndex == defaultGroup)
  {
    return;
  }
  this->SetSynchronizationGroup(widget, defaultGroup);
  this->ReclaimSelectionGroupIfEmpty(oldIndex);
}

std::string QmitkMxNMultiWidget::GetCellSelectionGroup(const QString& windowId) const
{
  const auto cell = this->GetRenderWindowWidget(windowId);
  if (nullptr == cell || nullptr == cell->GetUtilityWidget())
  {
    return {};
  }
  const auto index = cell->GetUtilityWidget()->GetSyncGroup();
  const auto it = m_GroupNameByIndex.find(index);
  return (it != m_GroupNameByIndex.end()) ? it->second : std::string();
}

void QmitkMxNMultiWidget::SetSyncLink(const QString& windowId,
                                      QmitkMxNSyncDimension dimension,
                                      const std::string& group,
                                      const SyncOffset& offset)
{
  if (nullptr == this->GetRenderWindowWidget(windowId))
  {
    mitkThrow() << "SetSyncLink: unknown render window '" << windowId.toStdString() << "'.";
  }
  if (!GROUP_NAME_PATTERN.match(QString::fromStdString(group)).hasMatch())
  {
    mitkThrow() << "SetSyncLink: group name '" << group
                << "' does not match the required pattern '"
                << GROUP_NAME_PATTERN.pattern().toStdString() << "' (URL-segment-safe).";
  }

  auto& links = m_CellSyncLinks[windowId];
  switch (dimension)
  {
    case QmitkMxNSyncDimension::Slice:
      if (std::holds_alternative<int>(offset))
      {
        links.sliceOffset = std::get<int>(offset);
      }
      else if (std::holds_alternative<std::monostate>(offset))
      {
        links.sliceOffset = 0;
      }
      else
      {
        mitkThrow() << "SetSyncLink: the 'Slice' offset must be an integer (slice steps).";
      }
      break;
    case QmitkMxNSyncDimension::Zoom:
      if (std::holds_alternative<double>(offset))
      {
        if (std::get<double>(offset) <= 0.0)
        {
          mitkThrow() << "SetSyncLink: the 'Zoom' offset must be > 0 (multiplicative factor).";
        }
        links.zoomOffset = std::get<double>(offset);
      }
      else if (std::holds_alternative<std::monostate>(offset))
      {
        links.zoomOffset = 1.0;
      }
      else
      {
        mitkThrow() << "SetSyncLink: the 'Zoom' offset must be a number > 0 (multiplicative factor).";
      }
      break;
    case QmitkMxNSyncDimension::Pan:
      if (std::holds_alternative<mitk::Vector2D>(offset))
      {
        links.panOffset = std::get<mitk::Vector2D>(offset);
      }
      else if (std::holds_alternative<std::monostate>(offset))
      {
        links.panOffset = mitk::Vector2D(0.0);
      }
      else
      {
        mitkThrow() << "SetSyncLink: the 'Pan' offset must be a 2D in-plane world-mm vector.";
      }
      break;
    default:
      if (!std::holds_alternative<std::monostate>(offset))
      {
        mitkThrow() << "SetSyncLink: dimension '" << QmitkMxNSyncDimensionToLinkKey(dimension)
                    << "' does not accept an offset.";
      }
      break;
  }
  links.groups[DimensionIndex(dimension)] = group;
  this->RegisterGroupForHue(group);

  // An orientation link aligns the joining cell's plane to the seed's
  // (absolute state; joining means adopting the group's plane).
  if (QmitkMxNSyncDimension::Orientation == dimension)
  {
    const auto seedId = this->FindSyncGroupSeed(dimension, group);
    if (!seedId.isEmpty() && seedId != windowId)
    {
      const auto seedWidget = this->GetRenderWindowWidget(seedId);
      const auto memberWidget = this->GetRenderWindowWidget(windowId);
      if (nullptr != seedWidget && nullptr != memberWidget && nullptr != memberWidget->GetUtilityWidget())
      {
        // Silent path: adopting the group plane is a relayed change, not a
        // new orientation gesture.
        memberWidget->GetUtilityWidget()->SetViewDirectionSelection(
          seedWidget->GetSliceNavigationController()->GetDefaultViewDirection());
      }
    }
  }

  // Slice and orientation need a shared reference geometry (a step index or
  // a plane name means different physical locations across divergent
  // geometries); align the cell's geometry-authority component to its seed
  // and restore offsets that the alignment reset.
  if (QmitkMxNSyncDimension::Slice == dimension || QmitkMxNSyncDimension::Orientation == dimension)
  {
    const auto reinitialized = this->EnforceComponentGeometry(windowId);
    this->ReconvergeGeometryRelativeGroups(reinitialized);
  }

  // Converge the joining cell to the group's reference. The pre-order first
  // member is the seed and defines the reference, so it is never converged
  // itself. Crosshair propagation is absolute (no state to converge);
  // windowing / lut have no engine yet.
  if (QmitkMxNSyncDimension::Slice == dimension || QmitkMxNSyncDimension::Zoom == dimension
      || QmitkMxNSyncDimension::Pan == dimension)
  {
    const auto seedId = this->FindSyncGroupSeed(dimension, group);
    if (!seedId.isEmpty() && seedId != windowId)
    {
      this->ConvergeMemberToSeed(dimension, seedId, windowId);
    }
  }
}

void QmitkMxNMultiWidget::ClearSyncLink(const QString& windowId, QmitkMxNSyncDimension dimension)
{
  if (nullptr == this->GetRenderWindowWidget(windowId))
  {
    mitkThrow() << "ClearSyncLink: unknown render window '" << windowId.toStdString() << "'.";
  }
  const auto it = m_CellSyncLinks.find(windowId);
  if (it == m_CellSyncLinks.end())
  {
    return;
  }
  auto& links = it->second;
  links.groups[DimensionIndex(dimension)].reset();
  switch (dimension)
  {
    case QmitkMxNSyncDimension::Slice: links.sliceOffset = 0; break;
    case QmitkMxNSyncDimension::Zoom:  links.zoomOffset = 1.0; break;
    case QmitkMxNSyncDimension::Pan:   links.panOffset = mitk::Vector2D(0.0); break;
    default: break;
  }
}

std::optional<QmitkMxNMultiWidget::SyncLinkState>
QmitkMxNMultiWidget::GetSyncLink(const QString& windowId, QmitkMxNSyncDimension dimension) const
{
  const auto it = m_CellSyncLinks.find(windowId);
  if (it == m_CellSyncLinks.end())
  {
    return std::nullopt;
  }
  const auto& links = it->second;
  const auto& group = links.groups[DimensionIndex(dimension)];
  if (!group.has_value())
  {
    return std::nullopt;
  }

  SyncLinkState state;
  state.group = *group;
  switch (dimension)
  {
    case QmitkMxNSyncDimension::Slice: state.offset = links.sliceOffset; break;
    case QmitkMxNSyncDimension::Zoom:  state.offset = links.zoomOffset; break;
    case QmitkMxNSyncDimension::Pan:   state.offset = links.panOffset; break;
    default: break;
  }
  return state;
}

std::vector<std::string> QmitkMxNMultiWidget::GetSyncGroupNames(QmitkMxNSyncDimension dimension) const
{
  // Only live cells count; stale map entries of removed cells must not
  // resurface as offered group names.
  std::set<std::string> names;
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    const auto it = m_CellSyncLinks.find(windowId);
    if (it != m_CellSyncLinks.end())
    {
      const auto& group = it->second.groups[DimensionIndex(dimension)];
      if (group.has_value())
      {
        names.insert(*group);
      }
    }
  }
  return { names.begin(), names.end() };
}

void QmitkMxNMultiWidget::ReconvergeSyncGroup(QmitkMxNSyncDimension dimension, const std::string& group)
{
  if (QmitkMxNSyncDimension::Slice != dimension && QmitkMxNSyncDimension::Zoom != dimension
      && QmitkMxNSyncDimension::Pan != dimension)
  {
    mitkThrow() << "ReconvergeSyncGroup: dimension '" << QmitkMxNSyncDimensionToLinkKey(dimension)
                << "' carries no convergence bookkeeping (only slice, zoom, and pan do).";
  }

  const auto seedId = this->FindSyncGroupSeed(dimension, group);
  if (seedId.isEmpty())
  {
    mitkThrow() << "ReconvergeSyncGroup: no cell links group '" << group
                << "' for dimension '" << QmitkMxNSyncDimensionToLinkKey(dimension) << "'.";
  }

  for (const auto& descriptor : this->ListWindowDescriptors())
  {
    if (descriptor.id == seedId)
    {
      continue;
    }
    const auto it = m_CellSyncLinks.find(descriptor.id);
    if (it != m_CellSyncLinks.end() && it->second.groups[DimensionIndex(dimension)] == group)
    {
      this->ConvergeMemberToSeed(dimension, seedId, descriptor.id);
    }
  }
}

void QmitkMxNMultiWidget::RefreshSyncControls()
{
  emit SyncLinksChanged();
}

QString QmitkMxNMultiWidget::FindSyncGroupSeed(QmitkMxNSyncDimension dimension, const std::string& group) const
{
  for (const auto& descriptor : this->ListWindowDescriptors())
  {
    const auto it = m_CellSyncLinks.find(descriptor.id);
    if (it != m_CellSyncLinks.end() && it->second.groups[DimensionIndex(dimension)] == group)
    {
      return descriptor.id;
    }
  }
  return {};
}

void QmitkMxNMultiWidget::ConvergeMemberToSeed(QmitkMxNSyncDimension dimension,
                                               const QString& seedId,
                                               const QString& memberId)
{
  const auto seedWidget = this->GetRenderWindowWidget(seedId);
  const auto memberWidget = this->GetRenderWindowWidget(memberId);
  if (nullptr == seedWidget || nullptr == memberWidget || seedId == memberId)
  {
    return;
  }
  auto* seedRenderer = mitk::BaseRenderer::GetInstance(seedWidget->GetRenderWindow()->GetVtkRenderWindow());
  auto* memberRenderer = mitk::BaseRenderer::GetInstance(memberWidget->GetRenderWindow()->GetVtkRenderWindow());
  if (nullptr == seedRenderer || nullptr == memberRenderer)
  {
    return;
  }
  // Unrealized windows (no world geometry yet) carry default camera/stepper
  // state that must not be propagated as a reference; the deferred converge
  // is covered by 'ReconvergeSyncGroup'.
  if (nullptr == seedRenderer->GetCurrentWorldGeometry()
      || nullptr == memberRenderer->GetCurrentWorldGeometry())
  {
    return;
  }

  const auto memberLinksIt = m_CellSyncLinks.find(memberId);
  if (memberLinksIt == m_CellSyncLinks.end())
  {
    return;
  }
  const auto& memberLinks = memberLinksIt->second;

  switch (dimension)
  {
    case QmitkMxNSyncDimension::Slice:
    {
      auto* seedStepper = seedRenderer->GetSliceNavigationController()->GetStepper();
      auto* memberStepper = memberRenderer->GetSliceNavigationController()->GetStepper();
      if (nullptr == seedStepper || nullptr == memberStepper
          || 0 == seedStepper->GetSteps() || 0 == memberStepper->GetSteps())
      {
        return;
      }
      // Signed arithmetic: the steppers count unsigned, so a negative offset
      // on a seed at slice 0 would wrap and clamp to the last slice instead
      // of the first.
      const long lastStep = static_cast<long>(memberStepper->GetSteps()) - 1;
      const long target = std::clamp(
        static_cast<long>(seedStepper->GetPos()) + memberLinks.sliceOffset, 0L, lastStep);
      memberStepper->SetPos(static_cast<unsigned int>(target));
      break;
    }
    case QmitkMxNSyncDimension::Zoom:
    {
      auto* seedCamera = seedRenderer->GetVtkRenderer()->GetActiveCamera();
      auto* memberCamera = memberRenderer->GetVtkRenderer()->GetActiveCamera();
      if (nullptr == seedCamera || nullptr == memberCamera)
      {
        return;
      }
      // Offset factor > 1 keeps the member zoomed in relative to the seed;
      // parallel scale is the inverse zoom measure, hence the division.
      memberCamera->SetParallelScale(seedCamera->GetParallelScale() / memberLinks.zoomOffset);
      mitk::RenderingManager::GetInstance()->RequestUpdate(
        memberWidget->GetRenderWindow()->GetVtkRenderWindow());
      break;
    }
    case QmitkMxNSyncDimension::Pan:
    {
      auto* seedController = seedRenderer->GetCameraController();
      auto* memberController = memberRenderer->GetCameraController();
      if (nullptr == seedController || nullptr == memberController)
      {
        return;
      }
      memberController->MoveCameraToPoint(
        seedController->GetCameraPositionOnPlane() + memberLinks.panOffset);
      mitk::RenderingManager::GetInstance()->RequestUpdate(
        memberWidget->GetRenderWindow()->GetVtkRenderWindow());
      break;
    }
    default:
      break;
  }
}

void QmitkMxNMultiWidget::SetViewDirection(const QString& windowId, mitk::AnatomicalPlane viewDirection)
{
  const auto widget = this->GetRenderWindowWidget(windowId);
  if (nullptr == widget)
  {
    mitkThrow() << "SetViewDirection: unknown render window '" << windowId.toStdString() << "'.";
  }
  if (mitk::AnatomicalPlane::Axial != viewDirection && mitk::AnatomicalPlane::Coronal != viewDirection
      && mitk::AnatomicalPlane::Sagittal != viewDirection)
  {
    mitkThrow() << "SetViewDirection: only the standard anatomical planes "
                << "(axial, coronal, sagittal) are supported.";
  }
  auto* utilityWidget = widget->GetUtilityWidget();
  if (nullptr == utilityWidget)
  {
    mitkThrow() << "SetViewDirection: cell '" << windowId.toStdString()
                << "' has no utility widget.";
  }

  utilityWidget->SetViewDirectionSelection(viewDirection);
  this->PropagateOrientation(windowId, viewDirection);
}

void QmitkMxNMultiWidget::PropagateOrientation(const QString& sourceId, mitk::AnatomicalPlane viewDirection)
{
  // A change that arrives while a relay is in flight is itself a relayed
  // change; dropping it here is what makes orientation sync cycle-free.
  if (m_OrientationPropagationDepth > 0)
  {
    return;
  }
  if (mitk::AnatomicalPlane::Axial != viewDirection && mitk::AnatomicalPlane::Coronal != viewDirection
      && mitk::AnatomicalPlane::Sagittal != viewDirection)
  {
    return;
  }
  const auto sourceLink = this->GetSyncLink(sourceId, QmitkMxNSyncDimension::Orientation);
  if (!sourceLink.has_value())
  {
    return;
  }

  struct DepthGuard
  {
    unsigned int& depth;
    explicit DepthGuard(unsigned int& d) : depth(d) { ++depth; }
    ~DepthGuard() { --depth; }
  } guard(m_OrientationPropagationDepth);

  std::vector<QString> affected{ sourceId };
  for (const auto& descriptor : this->ListWindowDescriptors())
  {
    if (descriptor.id == sourceId)
    {
      continue;
    }
    const auto memberLink = this->GetSyncLink(descriptor.id, QmitkMxNSyncDimension::Orientation);
    if (!memberLink.has_value() || memberLink->group != sourceLink->group)
    {
      continue;
    }
    const auto memberWidget = this->GetRenderWindowWidget(descriptor.id);
    auto* memberUtility = (nullptr != memberWidget) ? memberWidget->GetUtilityWidget() : nullptr;
    if (nullptr == memberUtility)
    {
      continue;
    }
    memberUtility->SetViewDirectionSelection(viewDirection);
    ++m_OrientationApplyCount;
    affected.push_back(descriptor.id);
  }

  // A plane flip re-initializes each member's stepper and camera; restore
  // the component's shared geometry and the declared offsets.
  auto reinitialized = this->EnforceComponentGeometry(sourceId);
  affected.insert(affected.end(), reinitialized.begin(), reinitialized.end());
  this->ReconvergeGeometryRelativeGroups(affected);
}

std::vector<QString> QmitkMxNMultiWidget::ComputeGeometryComponent(const QString& windowId) const
{
  const auto descriptors = this->ListWindowDescriptors();

  auto sharesGeometryGroup = [this](const QString& a, const QString& b)
  {
    for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Orientation })
    {
      const auto linkA = this->GetSyncLink(a, dimension);
      const auto linkB = this->GetSyncLink(b, dimension);
      if (linkA.has_value() && linkB.has_value() && linkA->group == linkB->group)
      {
        return true;
      }
    }
    return false;
  };

  std::set<QString> component{ windowId };
  bool grew = true;
  while (grew)
  {
    grew = false;
    for (const auto& descriptor : descriptors)
    {
      if (component.count(descriptor.id) > 0)
      {
        continue;
      }
      for (const auto& member : component)
      {
        if (sharesGeometryGroup(descriptor.id, member))
        {
          component.insert(descriptor.id);
          grew = true;
          break;
        }
      }
    }
  }

  std::vector<QString> preOrder;
  for (const auto& descriptor : descriptors)
  {
    if (component.count(descriptor.id) > 0)
    {
      preOrder.push_back(descriptor.id);
    }
  }
  return preOrder;
}

std::vector<QString> QmitkMxNMultiWidget::EnforceComponentGeometry(const QString& windowId)
{
  std::vector<QString> reinitialized;
  const auto component = this->ComputeGeometryComponent(windowId);
  if (component.size() < 2)
  {
    return reinitialized;
  }

  const auto seedWidget = this->GetRenderWindowWidget(component.front());
  if (nullptr == seedWidget)
  {
    return reinitialized;
  }
  const auto* referenceGeometry = seedWidget->GetSliceNavigationController()->GetInputWorldTimeGeometry();
  if (nullptr == referenceGeometry)
  {
    return reinitialized;
  }

  for (std::size_t i = 1; i < component.size(); ++i)
  {
    const auto memberWidget = this->GetRenderWindowWidget(component[i]);
    if (nullptr == memberWidget)
    {
      continue;
    }
    const auto* memberGeometry = memberWidget->GetSliceNavigationController()->GetInputWorldTimeGeometry();
    if (nullptr != memberGeometry && mitk::Equal(*memberGeometry, *referenceGeometry, mitk::eps, false))
    {
      continue;
    }
    mitk::RenderingManager::GetInstance()->InitializeView(
      memberWidget->GetRenderWindow()->GetVtkRenderWindow(), referenceGeometry);
    reinitialized.push_back(component[i]);
  }
  return reinitialized;
}

void QmitkMxNMultiWidget::ReconvergeGeometryRelativeGroups(const std::vector<QString>& windowIds)
{
  std::set<std::pair<QmitkMxNSyncDimension, std::string>> groups;
  for (const auto& windowId : windowIds)
  {
    for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Pan })
    {
      const auto link = this->GetSyncLink(windowId, dimension);
      if (link.has_value())
      {
        groups.emplace(dimension, link->group);
      }
    }
  }
  for (const auto& [dimension, group] : groups)
  {
    this->ReconvergeSyncGroup(dimension, group);
  }
}

void QmitkMxNMultiWidget::ReinitSyncGroupGeometry(const QString& windowId)
{
  const auto widget = this->GetRenderWindowWidget(windowId);
  if (nullptr == widget)
  {
    mitkThrow() << "ReinitSyncGroupGeometry: unknown render window '" << windowId.toStdString() << "'.";
  }
  const auto dataStorage = this->GetDataStorage();
  if (nullptr == dataStorage)
  {
    mitkThrow() << "ReinitSyncGroupGeometry: no data storage set on the multi widget.";
  }
  auto* triggerRenderer = mitk::BaseRenderer::GetInstance(widget->GetRenderWindow()->GetVtkRenderWindow());

  // The bounding geometry a single-cell reinit would compute (nodes not
  // excluded from the bounding box, non-helper), evaluated with the
  // triggering cell's visibility - "last reinit wins" for the component.
  const auto includeInBoundingBox =
    mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false));
  const auto helperObject =
    mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
  const auto predicate = mitk::NodePredicateAnd::New(
    mitk::NodePredicateNot::New(includeInBoundingBox), mitk::NodePredicateNot::New(helperObject));
  const auto filteredNodes = dataStorage->GetSubset(predicate);
  const auto bounds = dataStorage->ComputeBoundingGeometry3D(filteredNodes, "visible", triggerRenderer);

  const auto component = this->ComputeGeometryComponent(windowId);
  for (const auto& memberId : component)
  {
    const auto memberWidget = this->GetRenderWindowWidget(memberId);
    if (nullptr != memberWidget)
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        memberWidget->GetRenderWindow()->GetVtkRenderWindow(), bounds);
    }
  }
  this->ReconvergeGeometryRelativeGroups(component);
}

unsigned int QmitkMxNMultiWidget::GetOrientationApplyCount() const
{
  return m_OrientationApplyCount;
}

void QmitkMxNMultiWidget::SetLookupTable(const QString& windowId, mitk::DataNode* node, mitk::LookupTable* lookupTable)
{
  const auto widget = this->GetRenderWindowWidget(windowId);
  if (nullptr == widget)
  {
    mitkThrow() << "SetLookupTable: unknown render window '" << windowId.toStdString() << "'.";
  }
  if (nullptr == node)
  {
    mitkThrow() << "SetLookupTable: node must not be null.";
  }
  if (nullptr == lookupTable)
  {
    mitkThrow() << "SetLookupTable: lookupTable must not be null.";
  }

  std::vector<QString> targets{ windowId };
  const auto link = this->GetSyncLink(windowId, QmitkMxNSyncDimension::Lut);
  if (link.has_value())
  {
    for (const auto& descriptor : this->ListWindowDescriptors())
    {
      if (descriptor.id == windowId)
      {
        continue;
      }
      const auto memberLink = this->GetSyncLink(descriptor.id, QmitkMxNSyncDimension::Lut);
      if (memberLink.has_value() && memberLink->group == link->group)
      {
        targets.push_back(descriptor.id);
      }
    }
  }

  for (const auto& targetId : targets)
  {
    const auto targetWidget = this->GetRenderWindowWidget(targetId);
    if (nullptr == targetWidget)
    {
      continue;
    }
    auto* targetRenderer = mitk::BaseRenderer::GetInstance(
      targetWidget->GetRenderWindow()->GetVtkRenderWindow());
    if (nullptr == targetRenderer)
    {
      continue;
    }
    node->SetProperty("LookupTable", mitk::LookupTableProperty::New(lookupTable), targetRenderer);
    mitk::RenderingManager::GetInstance()->RequestUpdate(
      targetWidget->GetRenderWindow()->GetVtkRenderWindow());
  }
}

void QmitkMxNMultiWidget::SetLevelWindow(const QString& windowId, mitk::DataNode* node,
                                         const mitk::LevelWindow& levelWindow)
{
  this->ApplyLevelWindow(windowId, node,
    [&levelWindow](mitk::LevelWindow& value) { value = levelWindow; });
}

void QmitkMxNMultiWidget::AdjustLevelWindow(const QString& windowId, mitk::DataNode* node,
                                            mitk::ScalarType levelDelta, mitk::ScalarType windowDelta)
{
  this->ApplyLevelWindow(windowId, node,
    [levelDelta, windowDelta](mitk::LevelWindow& value)
    {
      value.SetLevelWindow(value.GetLevel() + levelDelta, value.GetWindow() + windowDelta);
    });
}

void QmitkMxNMultiWidget::ApplyLevelWindow(const QString& windowId, mitk::DataNode* node,
                                           const std::function<void(mitk::LevelWindow&)>& modify)
{
  const auto widget = this->GetRenderWindowWidget(windowId);
  if (nullptr == widget)
  {
    mitkThrow() << "ApplyLevelWindow: unknown render window '" << windowId.toStdString() << "'.";
  }
  if (nullptr == node)
  {
    mitkThrow() << "ApplyLevelWindow: node must not be null.";
  }

  const auto link = this->GetSyncLink(windowId, QmitkMxNSyncDimension::Windowing);
  if (!link.has_value())
  {
    // Unlinked cells keep the classic node-global write, staying coupled to
    // the global level/window controls like the gesture path's fallback.
    mitk::LevelWindow levelWindow;
    node->GetLevelWindow(levelWindow);
    modify(levelWindow);
    node->SetProperty("levelwindow", mitk::LevelWindowProperty::New(levelWindow));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return;
  }

  std::vector<QString> targets{ windowId };
  for (const auto& descriptor : this->ListWindowDescriptors())
  {
    if (descriptor.id == windowId)
    {
      continue;
    }
    const auto memberLink = this->GetSyncLink(descriptor.id, QmitkMxNSyncDimension::Windowing);
    if (memberLink.has_value() && memberLink->group == link->group)
    {
      targets.push_back(descriptor.id);
    }
  }

  for (const auto& targetId : targets)
  {
    const auto targetWidget = this->GetRenderWindowWidget(targetId);
    if (nullptr == targetWidget)
    {
      continue;
    }
    auto* targetRenderer = mitk::BaseRenderer::GetInstance(
      targetWidget->GetRenderWindow()->GetVtkRenderWindow());
    if (nullptr == targetRenderer)
    {
      continue;
    }
    // Read renderer-specific with node-global fallback, write renderer-
    // specific: same per-member semantics as the synchronized gesture, so the
    // mapper prefers the member's own value from now on.
    mitk::LevelWindow levelWindow;
    node->GetLevelWindow(levelWindow, targetRenderer);
    modify(levelWindow);
    node->SetProperty("levelwindow", mitk::LevelWindowProperty::New(levelWindow), targetRenderer);
    mitk::RenderingManager::GetInstance()->RequestUpdate(
      targetWidget->GetRenderWindow()->GetVtkRenderWindow());
  }
}

void QmitkMxNMultiWidget::SetCleanView(bool cleanView)
{
  if (cleanView == m_CleanView)
  {
    return;
  }

  m_CleanView = cleanView;
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    if (auto* proximity = renderWindowWidget->findChild<QmitkRenderWindowProximity*>(
          QString(), Qt::FindDirectChildrenOnly))
    {
      proximity->SetSuppressed(cleanView);
    }
    if (auto* cellOverlay = renderWindowWidget->findChild<QmitkMxNCellOverlay*>(
          QString(), Qt::FindDirectChildrenOnly))
    {
      cellOverlay->SetCleanView(cleanView);
    }
  }

  emit CleanViewChanged(cleanView);
}

bool QmitkMxNMultiWidget::IsCleanView() const
{
  return m_CleanView;
}

void QmitkMxNMultiWidget::SetLevelWindowReadoutVisible(bool visible)
{
  m_LevelWindowReadoutVisible = visible;
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    if (auto* cellOverlay = renderWindowWidget->findChild<QmitkMxNCellOverlay*>(
          QString(), Qt::FindDirectChildrenOnly))
    {
      cellOverlay->SetReadoutVisible(visible);
    }
  }
}

void QmitkMxNMultiWidget::SetNavigatorExpanded(bool expanded)
{
  if (expanded == m_NavigatorExpanded)
  {
    return;
  }

  m_NavigatorExpanded = expanded;
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    if (auto* cellOverlay = renderWindowWidget->findChild<QmitkMxNCellOverlay*>(
          QString(), Qt::FindDirectChildrenOnly))
    {
      cellOverlay->SetNavigatorExpanded(expanded);
    }
  }

  emit NavigatorExpandedChanged(expanded);
}

bool QmitkMxNMultiWidget::IsNavigatorExpanded() const
{
  return m_NavigatorExpanded;
}

void QmitkMxNMultiWidget::RequestLayoutEditor()
{
  emit LayoutEditorRequested();
}


void QmitkMxNMultiWidget::RefreshSyncBarcodes()
{
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    auto* utilityWidget = renderWindowWidget->GetUtilityWidget();
    if (nullptr == utilityWidget)
    {
      continue;
    }

    QList<QmitkMxNSyncBarcodeWidget::AxisSlot> axisSlots;
    axisSlots.reserve(static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1);

    // The seven per-dimension axes: a slot carries the group hue when linked and
    // is an unsynced gap otherwise, plus the axis glyph the barcode draws when
    // wide enough.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      QString label;
      QmitkMxNAxisGlyph glyph = QmitkMxNAxisGlyph::Pan;
      switch (dimension)
      {
        case QmitkMxNSyncDimension::Pan:         label = tr("Pan"); glyph = QmitkMxNAxisGlyph::Pan; break;
        case QmitkMxNSyncDimension::Zoom:        label = tr("Zoom"); glyph = QmitkMxNAxisGlyph::Zoom; break;
        case QmitkMxNSyncDimension::Slice:       label = tr("Slice"); glyph = QmitkMxNAxisGlyph::Slice; break;
        case QmitkMxNSyncDimension::Crosshair:   label = tr("Crosshair"); glyph = QmitkMxNAxisGlyph::Crosshair; break;
        case QmitkMxNSyncDimension::Orientation: label = tr("Orientation"); glyph = QmitkMxNAxisGlyph::Orientation; break;
        case QmitkMxNSyncDimension::Windowing:   label = tr("Windowing"); glyph = QmitkMxNAxisGlyph::Windowing; break;
        case QmitkMxNSyncDimension::Lut:         label = tr("LUT"); glyph = QmitkMxNAxisGlyph::Lut; break;
      }

      QmitkMxNSyncBarcodeWidget::AxisSlot slot;
      slot.glyph = glyph;
      if (const auto link = this->GetSyncLink(windowId, dimension); link.has_value())
      {
        try
        {
          slot.color = this->GetSyncGroupColor(link->group);
          slot.tooltip = tr("%1 - group %2").arg(label,
            QString::fromStdString(this->GetSyncGroupDisplayName(link->group)));
        }
        catch (const mitk::Exception&)
        {
          // Group not registered mid-change; leave the slot a gap this round.
        }
      }
      if (!slot.color.isValid())
      {
        slot.tooltip = tr("%1 - not linked").arg(label);
      }
      axisSlots.append(slot);
    }

    // The selection axis is single-valued per cell. Every cell always carries a
    // selection group, but the default group 1 is the one every cell starts in;
    // painting it as synced would light up every cell at rest, so the slot reads
    // as a gap for the default group and shows a hue only once a cell is
    // deliberately assigned to another selection group.
    QmitkMxNSyncBarcodeWidget::AxisSlot selectionSlot;
    selectionSlot.glyph = QmitkMxNAxisGlyph::Selection;
    if (const auto index = utilityWidget->GetSyncGroup(); index > 1)
    {
      if (const auto recorded = m_GroupNameByIndex.find(index); recorded != m_GroupNameByIndex.end())
      {
        try
        {
          selectionSlot.color = this->GetSyncGroupColor(recorded->second);
          selectionSlot.tooltip = tr("Data selection - group %1").arg(
            QString::fromStdString(this->GetSyncGroupDisplayName(recorded->second)));
        }
        catch (const mitk::Exception&)
        {
        }
      }
    }
    if (!selectionSlot.color.isValid())
    {
      selectionSlot.tooltip = tr("Data selection - not linked");
    }
    axisSlots.append(selectionSlot);

    utilityWidget->SetSyncBarcodeSlots(axisSlots);
  }
}

QColor QmitkMxNMultiWidget::GetSyncGroupColor(const std::string& group) const
{
  // A layout-designer-set color wins over the default hue and is honored
  // verbatim (not themed), so a designed layout looks the same in dark and
  // light styles.
  const auto explicitColor = m_GroupColors.find(group);
  if (explicitColor != m_GroupColors.end())
  {
    return QColor(QString::fromStdString(explicitColor->second));
  }

  const auto it = std::find(m_GroupHueOrder.begin(), m_GroupHueOrder.end(), group);
  if (it == m_GroupHueOrder.end())
  {
    mitkThrow() << "GetSyncGroupColor: unknown group '" << group << "'.";
  }

  const auto position = static_cast<std::size_t>(std::distance(m_GroupHueOrder.begin(), it));
  return QColor(GROUP_HUE_PALETTE[position % GROUP_HUE_PALETTE.size()]);
}

QmitkMxNMultiWidget::CellGroupIdentity
QmitkMxNMultiWidget::ResolveCellGroupIdentity(const QString& windowId) const
{
  CellGroupIdentity identity;

  std::vector<std::string> distinctGroups;  // in dimension order, de-duplicated
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    const auto link = this->GetSyncLink(windowId, dimension);
    if (!link.has_value())
    {
      continue;
    }
    if (std::find(distinctGroups.begin(), distinctGroups.end(), link->group) == distinctGroups.end())
    {
      distinctGroups.push_back(link->group);
    }
  }

  if (distinctGroups.empty())
  {
    return identity;
  }

  // A single hue only when every linked dimension names the same group; a
  // heterogeneous cell stays neutral rather than picking one hue that would
  // misrepresent it (the barcode and editor carry the per-dimension detail).
  try
  {
    if (distinctGroups.size() == 1)
    {
      identity.hue = this->GetSyncGroupColor(distinctGroups.front());
      identity.kind = CellGroupIdentityKind::Mono;
    }
    else
    {
      identity.kind = CellGroupIdentityKind::Complex;
    }
  }
  catch (const mitk::Exception&)
  {
    // Group not registered mid-layout-change; treat as no identity this pass.
    return CellGroupIdentity();
  }

  return identity;
}

void QmitkMxNMultiWidget::RefreshFrameColors()
{
  for (const auto& [windowId, renderWindowWidget] : this->GetRenderWindowWidgets())
  {
    // The border always carries group identity - the mono group hue, else a
    // neutral gray (ungrouped or heterogeneous) - so an active grouped cell
    // keeps its hue rather than losing it to a highlight color. Active-ness is
    // marked by white corner brackets the cell overlay paints on top. This is
    // the single MxN writer of the border stylesheet; it never touches the
    // shared 'SetDecorationColor' (which would leak into StdMultiWidget).
    // Dark-theme colors; a light theme would derive its own (deferred).
    const auto identity = this->ResolveCellGroupIdentity(windowId);
    const QColor border = (CellGroupIdentityKind::Mono == identity.kind) ? identity.hue : QColor(0x60, 0x60, 0x60);
    renderWindowWidget->setStyleSheet("QmitkRenderWindowWidget { border: 2px solid " +
                                      border.name(QColor::HexRgb) + "; }");

    // Repaint the overlay so its active-corner brackets track the active change.
    if (auto* overlay = renderWindowWidget->findChild<QmitkMxNCellOverlay*>(QString(), Qt::FindDirectChildrenOnly))
    {
      overlay->update();
    }
  }
}

std::vector<QmitkMxNMultiWidget::SyncGroupInfo> QmitkMxNMultiWidget::GetSyncGroupInfos() const
{
  // Registered selection groups appear even while empty (a freshly created
  // "+" group must be assignable from the editor); link groups exist exactly
  // as long as a live cell links them.
  std::set<std::string> selectionGroupIds;
  for (const auto& [index, name] : m_GroupNameByIndex)
  {
    selectionGroupIds.insert(name);
  }

  const auto descriptors = this->ListWindowDescriptors();

  std::vector<SyncGroupInfo> infos;
  for (const auto& id : m_GroupHueOrder)
  {
    SyncGroupInfo info;
    info.id = id;
    info.displayName = this->GetSyncGroupDisplayName(id);
    info.color = this->GetSyncGroupColor(id);
    info.hasExplicitColor = m_GroupColors.find(id) != m_GroupColors.end();

    for (const auto& descriptor : descriptors)
    {
      if (descriptor.selectionGroup.toStdString() == id)
      {
        info.selectionMembers.push_back(descriptor.id);
      }
      for (const auto dimension : QmitkMxNAllSyncDimensions)
      {
        const auto link = this->GetSyncLink(descriptor.id, dimension);
        if (link.has_value() && link->group == id)
        {
          info.members[dimension].push_back(descriptor.id);
        }
      }
    }

    if (info.selectionMembers.empty() && info.members.empty()
        && selectionGroupIds.find(id) == selectionGroupIds.end())
    {
      continue;
    }
    infos.push_back(std::move(info));
  }

  return infos;
}

std::string QmitkMxNMultiWidget::GetSyncGroupDisplayName(const std::string& id) const
{
  const auto it = m_GroupDisplayNames.find(id);
  return (it != m_GroupDisplayNames.end() && !it->second.empty()) ? it->second : id;
}

QString QmitkMxNMultiWidget::GetSyncGroupDisplayName(GroupSyncIndexType index) const
{
  const auto it = m_GroupNameByIndex.find(index);
  if (it == m_GroupNameByIndex.end())
  {
    mitkThrow() << "GetSyncGroupDisplayName: no group with engine index " << index << ".";
  }
  return QString::fromStdString(this->GetSyncGroupDisplayName(it->second));
}

void QmitkMxNMultiWidget::SetSyncGroupDisplayName(const std::string& id, const std::string& displayName)
{
  if (std::find(m_GroupHueOrder.begin(), m_GroupHueOrder.end(), id) == m_GroupHueOrder.end())
  {
    mitkThrow() << "SetSyncGroupDisplayName: unknown group '" << id << "'.";
  }

  if (displayName.empty())
  {
    m_GroupDisplayNames.erase(id);
  }
  else
  {
    m_GroupDisplayNames[id] = displayName;
  }

  // Selection groups surface the label in every cell's group selector.
  for (const auto& [index, name] : m_GroupNameByIndex)
  {
    if (name == id)
    {
      emit SyncGroupLabelChanged(index, QString::fromStdString(this->GetSyncGroupDisplayName(id)));
    }
  }
  emit SyncLinksChanged();
}

void QmitkMxNMultiWidget::SetSyncGroupColor(const std::string& id, const QColor& color)
{
  if (std::find(m_GroupHueOrder.begin(), m_GroupHueOrder.end(), id) == m_GroupHueOrder.end())
  {
    mitkThrow() << "SetSyncGroupColor: unknown group '" << id << "'.";
  }
  if (!color.isValid())
  {
    mitkThrow() << "SetSyncGroupColor: invalid color for group '" << id << "'.";
  }

  m_GroupColors[id] = color.name(QColor::HexRgb).toStdString();
  emit SyncLinksChanged();
}

void QmitkMxNMultiWidget::RegisterGroupForHue(const std::string& group)
{
  if (std::find(m_GroupHueOrder.begin(), m_GroupHueOrder.end(), group) == m_GroupHueOrder.end())
  {
    m_GroupHueOrder.push_back(group);
  }
}
