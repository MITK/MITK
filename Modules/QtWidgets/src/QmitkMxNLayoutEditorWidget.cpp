/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNLayoutEditorWidget.h"

#include <QmitkMultiWidgetLayoutSelectionWidget.h>
#include <QmitkMxNCellMapWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>

#include <mitkExceptionMacro.h>
#include <mitkLog.h>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <functional>
#include <initializer_list>

namespace
{
  const char* DimensionLabel(QmitkMxNSyncDimension dimension)
  {
    switch (dimension)
    {
      case QmitkMxNSyncDimension::Pan:         return "Pan";
      case QmitkMxNSyncDimension::Zoom:        return "Zoom";
      case QmitkMxNSyncDimension::Slice:       return "Slice";
      case QmitkMxNSyncDimension::Crosshair:   return "Crosshair";
      case QmitkMxNSyncDimension::Orientation: return "Orientation";
      case QmitkMxNSyncDimension::Windowing:   return "Windowing";
      case QmitkMxNSyncDimension::Lut:         return "LUT";
    }
    return "";
  }

  QmitkMxNAxisGlyph GlyphFor(QmitkMxNSyncDimension dimension)
  {
    switch (dimension)
    {
      case QmitkMxNSyncDimension::Pan:         return QmitkMxNAxisGlyph::Pan;
      case QmitkMxNSyncDimension::Zoom:        return QmitkMxNAxisGlyph::Zoom;
      case QmitkMxNSyncDimension::Slice:       return QmitkMxNAxisGlyph::Slice;
      case QmitkMxNSyncDimension::Crosshair:   return QmitkMxNAxisGlyph::Crosshair;
      case QmitkMxNSyncDimension::Orientation: return QmitkMxNAxisGlyph::Orientation;
      case QmitkMxNSyncDimension::Windowing:   return QmitkMxNAxisGlyph::Windowing;
      case QmitkMxNSyncDimension::Lut:         return QmitkMxNAxisGlyph::Lut;
    }
    return QmitkMxNAxisGlyph::Pan;
  }

  constexpr std::array<QmitkMxNSyncDimension, 4> NavigationBundle{
    QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair
  };

  // Name of the default selection group (engine index 1; see
  // QmitkMxNMultiWidget::AddSynchronizationGroup). A cell "resting on the default"
  // is one whose selection has not been placed into a specific group.
  const std::string DefaultSelectionGroup = "main";

  // Mode (a) "replace" primitive: strip a cell's ties to every group other than
  // 'keepGroup'. The seven dimension axes are unlinked; the selection reverts to
  // the default "main" (there is no unlinked state for selection). Shared by the
  // SetCellMembership join and the empty-group cache flush so both fully replace.
  void ClearOtherGroupTies(QmitkMxNMultiWidget* multiWidget, const QString& windowId,
                           const std::string& keepGroup)
  {
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      const auto link = multiWidget->GetSyncLink(windowId, dimension);
      if (link.has_value() && link->group != keepGroup)
      {
        multiWidget->ClearSyncLink(windowId, dimension);
      }
    }
    if (multiWidget->GetCellSelectionGroup(windowId) != keepGroup)
    {
      multiWidget->ClearCellSelectionGroup(windowId);
    }
  }

  const QString NotLinkedEntry = QStringLiteral("(not linked)");

  /**
   * Style a group card's header as a solid bar in the group hue, with the name,
   * count, and menu button in a contrasting ink. Shared by card creation and the
   * in-place refresh so a recolor updates both the bar and its text.
   */
  void StyleGroupHeader(QFrame* header, QLabel* name, QLabel* count, QToolButton* menuButton,
                        const QColor& hue)
  {
    const double luminance = 0.299 * hue.red() + 0.587 * hue.green() + 0.114 * hue.blue();
    const QString ink = luminance > 140.0 ? QStringLiteral("#1a1a1a") : QStringLiteral("#ffffff");
    header->setStyleSheet(QStringLiteral("background-color: %1; border-top-left-radius: 3px; "
                                         "border-top-right-radius: 3px;").arg(hue.name()));
    name->setStyleSheet(QStringLiteral("color: %1; font-weight: bold; background: transparent;").arg(ink));
    count->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(ink));
    menuButton->setStyleSheet(
      QStringLiteral("QToolButton { color: %1; background: transparent; border: none; }").arg(ink));
  }

  void ClearLayout(QLayout* layout)
  {
    while (auto* item = layout->takeAt(0))
    {
      delete item->widget();
      delete item;
    }
  }

  /**
   * The group card as one coherent object: dragging its background assigns the
   * group to the map's selected cells (the whole card is the drag source, not a
   * tiny swatch), and it accepts cell drops from the map, highlighting in the
   * group hue while a drag hovers. Interactive children (name field, glyph
   * strip, buttons) receive their own events first, so the drag starts only from
   * the card's own surface.
   */
  class GroupCardFrame : public QFrame
  {
  public:
    GroupCardFrame(QString groupId, QColor hue,
                   std::function<void(const QStringList&, QmitkMxNGroupJoinMode)> onCellsDropped,
                   QWidget* parent = nullptr)
      : QFrame(parent)
      , m_GroupId(std::move(groupId))
      , m_Hue(std::move(hue))
      , m_OnCellsDropped(std::move(onCellsDropped))
    {
      this->setAcceptDrops(true);
    }

  protected:
    void mousePressEvent(QMouseEvent* event) override
    {
      if (event->button() == Qt::LeftButton)
      {
        m_PressPosition = event->pos();
      }
      QFrame::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
      if (event->buttons().testFlag(Qt::LeftButton)
          && (event->pos() - m_PressPosition).manhattanLength() >= QApplication::startDragDistance())
      {
        auto* mimeData = new QMimeData();
        mimeData->setData(QmitkMxNCellMapWidget::GroupMimeType, m_GroupId.toUtf8());
        auto* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
        return;
      }
      QFrame::mouseMoveEvent(event);
    }

    void dragEnterEvent(QDragEnterEvent* event) override
    {
      if (event->mimeData()->hasFormat(QmitkMxNCellMapWidget::CellsMimeType))
      {
        m_DropHighlight = true;
        this->update();
        event->acceptProposedAction();
      }
    }

    void dragLeaveEvent(QDragLeaveEvent*) override
    {
      m_DropHighlight = false;
      this->update();
    }

    void dropEvent(QDropEvent* event) override
    {
      m_DropHighlight = false;
      this->update();
      const auto ids = QString::fromUtf8(
        event->mimeData()->data(QmitkMxNCellMapWidget::CellsMimeType));
      m_OnCellsDropped(ids.split(QStringLiteral("\n"), Qt::SkipEmptyParts),
                       QmitkMxNCellMapWidget::JoinModeFromModifiers(event->modifiers()));
      event->acceptProposedAction();
    }

    void paintEvent(QPaintEvent* event) override
    {
      QFrame::paintEvent(event);
      if (m_DropHighlight)
      {
        QPainter painter(this);
        QColor tint = m_Hue.isValid() ? m_Hue : this->palette().color(QPalette::Highlight);
        tint.setAlpha(70);
        painter.fillRect(this->rect(), tint);
      }
    }

  private:
    QString m_GroupId;
    QColor m_Hue;
    std::function<void(const QStringList&, QmitkMxNGroupJoinMode)> m_OnCellsDropped;
    QPoint m_PressPosition;
    bool m_DropHighlight = false;
  };
}

QmitkMxNLayoutEditorWidget::QmitkMxNLayoutEditorWidget(QWidget* parent)
  : QWidget(parent)
{
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(4, 4, 4, 4);

  auto* topRow = new QHBoxLayout();
  m_AddGroupButton = new QToolButton(this);
  m_AddGroupButton->setText(tr("+ Group"));
  m_AddGroupButton->setToolTip(tr("Create a new synchronization group"));
  connect(m_AddGroupButton, &QToolButton::clicked, this, [this]() { this->CreateGroup(); });
  topRow->addWidget(m_AddGroupButton);
  topRow->addStretch();

  m_AdvancedButton = new QToolButton(this);
  m_AdvancedButton->setText(tr("Advanced"));
  m_AdvancedButton->setCheckable(true);
  m_AdvancedButton->setToolTip(tr("Power-user view: the full per-dimension link matrix "
                                  "with offset editors"));
  topRow->addWidget(m_AdvancedButton);
  mainLayout->addLayout(topRow);

  m_Faces = new QStackedWidget(this);

  // Main face: layout shape controls, the interactive cell map, group cards.
  auto* mainFace = new QScrollArea(m_Faces);
  mainFace->setWidgetResizable(true);
  auto* mainContainer = new QWidget(mainFace);
  auto* mainFaceLayout = new QVBoxLayout(mainContainer);

  auto* layoutBox = new QGroupBox(tr("Layout"), mainContainer);
  auto* layoutBoxLayout = new QVBoxLayout(layoutBox);
  m_LayoutSelection = new QmitkMultiWidgetLayoutSelectionWidget(layoutBox);
  layoutBoxLayout->addWidget(m_LayoutSelection);
  mainFaceLayout->addWidget(layoutBox);

  m_CellMap = new QmitkMxNCellMapWidget(mainContainer);
  m_CellMap->setMinimumHeight(140);
  m_CellMap->setToolTip(tr("Select render windows by click or Ctrl-click; "
                           "assign them by dropping them onto a group (or a group's color "
                           "onto a window). A plain drop replaces the window's groups; hold "
                           "Alt to merge, Shift to fill only its unsynced axes. Each window "
                           "shows its sync axes as glyphs: the seven dimensions plus data "
                           "selection, tinted by group, a gap where the window is not "
                           "synchronized on that axis."));
  connect(m_CellMap, &QmitkMxNCellMapWidget::AssignRequested, this,
          [this](const QString& group, const QStringList& windowIds, QmitkMxNGroupJoinMode mode)
          {
            this->AssignCellsToGroup(windowIds, group.toStdString(), mode);
          });
  connect(m_CellMap, &QmitkMxNCellMapWidget::SelectionChanged, this,
          [this](const QStringList& windowIds)
          {
            // Selecting a tile makes its render window the active one (the first
            // selected window for a multi-selection), so the map and the editor's
            // focus stay in step.
            if (!m_MultiWidget.isNull() && !windowIds.isEmpty())
            {
              if (const auto cell = m_MultiWidget->GetRenderWindowWidget(windowIds.first()))
              {
                m_MultiWidget->SetActiveRenderWindowWidget(cell);
              }
            }
            this->ScheduleRebuild();
          });
  mainFaceLayout->addWidget(m_CellMap);

  // Grow or shrink the grid by a trailing row or column. The operations edit the
  // splitter tree in place, so existing windows keep their ids, sync links,
  // renderer-specific node properties, and positions - only the trailing edge is
  // added (empty) or removed. They require a rectangular grid, which the
  // tree-derived ResolveGridShape decides, so the buttons disable (with an
  // explaining tooltip) for irregular or non-grid layouts.
  auto* gridRow = new QHBoxLayout();
  gridRow->setContentsMargins(0, 0, 0, 0);
  m_RemoveRowButton = new QToolButton(mainContainer);
  m_RemoveRowButton->setText(QStringLiteral("-"));
  m_AddRowButton = new QToolButton(mainContainer);
  m_AddRowButton->setText(QStringLiteral("+"));
  m_RemoveColumnButton = new QToolButton(mainContainer);
  m_RemoveColumnButton->setText(QStringLiteral("-"));
  m_AddColumnButton = new QToolButton(mainContainer);
  m_AddColumnButton->setText(QStringLiteral("+"));
  connect(m_RemoveRowButton, &QToolButton::clicked, this, [this]()
  {
    if (!m_MultiWidget.isNull())
    {
      m_MultiWidget->RemoveGridRow();
    }
  });
  connect(m_AddRowButton, &QToolButton::clicked, this, [this]()
  {
    if (!m_MultiWidget.isNull())
    {
      m_MultiWidget->AddGridRow();
    }
  });
  connect(m_RemoveColumnButton, &QToolButton::clicked, this, [this]()
  {
    if (!m_MultiWidget.isNull())
    {
      m_MultiWidget->RemoveGridColumn();
    }
  });
  connect(m_AddColumnButton, &QToolButton::clicked, this, [this]()
  {
    if (!m_MultiWidget.isNull())
    {
      m_MultiWidget->AddGridColumn();
    }
  });
  gridRow->addStretch();
  gridRow->addWidget(new QLabel(tr("Rows:"), mainContainer));
  gridRow->addWidget(m_RemoveRowButton);
  gridRow->addWidget(m_AddRowButton);
  gridRow->addSpacing(12);
  gridRow->addWidget(new QLabel(tr("Columns:"), mainContainer));
  gridRow->addWidget(m_RemoveColumnButton);
  gridRow->addWidget(m_AddColumnButton);
  gridRow->addStretch();
  mainFaceLayout->addLayout(gridRow);

  auto* groupsContainer = new QWidget(mainContainer);
  m_GroupsLayout = new QVBoxLayout(groupsContainer);
  m_GroupsLayout->setContentsMargins(0, 0, 0, 0);
  m_GroupsLayout->setSpacing(6);
  m_GroupsLayout->addStretch();
  mainFaceLayout->addWidget(groupsContainer);
  mainFaceLayout->addStretch();

  mainFace->setWidget(mainContainer);
  m_Faces->addWidget(mainFace);

  // Advanced face: the full per-dimension matrix.
  m_Matrix = new QTableWidget(m_Faces);
  m_Matrix->setSelectionMode(QAbstractItemView::NoSelection);
  m_Matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_Matrix->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_Faces->addWidget(m_Matrix);

  connect(m_AdvancedButton, &QToolButton::toggled, this,
          [this](bool advanced) { m_Faces->setCurrentIndex(advanced ? 1 : 0); });

  mainLayout->addWidget(m_Faces);

  this->setEnabled(false);
}

QmitkMxNLayoutEditorWidget::~QmitkMxNLayoutEditorWidget()
{
}

void QmitkMxNLayoutEditorWidget::SetMultiWidget(QmitkMxNMultiWidget* multiWidget)
{
  if (multiWidget == m_MultiWidget)
  {
    return;
  }

  if (!m_MultiWidget.isNull())
  {
    disconnect(m_MultiWidget, nullptr, this, nullptr);
  }

  m_MultiWidget = multiWidget;
  m_CellMap->SetMultiWidget(multiWidget);
  // The empty-group intent cache belongs to the attached editor's groups; a
  // swap (or detach) invalidates it.
  m_EmptyGroupAxisCache.clear();

  if (!m_MultiWidget.isNull())
  {
    connect(m_MultiWidget, &QmitkMxNMultiWidget::SyncLinksChanged,
            this, &QmitkMxNLayoutEditorWidget::ScheduleRebuild);
    connect(m_MultiWidget, &QmitkMxNMultiWidget::LayoutChanged,
            this, &QmitkMxNLayoutEditorWidget::ScheduleRebuild);
    connect(m_MultiWidget, &QmitkMxNMultiWidget::SyncGroupAdded,
            this, [this]() { this->ScheduleRebuild(); });
    // Reverse of the tile-selects-active link: when the editor's active render
    // window changes (e.g. the user clicks a window), select its tile in the
    // map. The engine setters no-op when unchanged, so this does not loop with
    // the forward direction.
    connect(m_MultiWidget, &QmitkMxNMultiWidget::ActiveRenderWindowChanged,
            this, &QmitkMxNLayoutEditorWidget::SelectActiveWindowTile);
  }

  this->setEnabled(!m_MultiWidget.isNull());
  this->ScheduleRebuild();
}

QmitkMxNMultiWidget* QmitkMxNLayoutEditorWidget::GetMultiWidget() const
{
  return m_MultiWidget;
}

QmitkMultiWidgetLayoutSelectionWidget* QmitkMxNLayoutEditorWidget::GetLayoutSelectionWidget() const
{
  return m_LayoutSelection;
}

void QmitkMxNLayoutEditorWidget::AssignCellsToGroup(const QStringList& windowIds,
                                                    const std::string& group,
                                                    QmitkMxNGroupJoinMode mode)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  // An empty group carrying a configured intent cache defines its axes exactly:
  // apply the cached axes (per the join mode) to the joining windows and clear
  // the cache, instead of the derived-dimensions default. The cache's selection
  // bit is honored here, which the derived path cannot do for a group's first
  // member (there is no prior member to follow).
  if (this->HasCachedGroupIntent(group) && this->GroupMembers(group).empty())
  {
    const auto it = m_EmptyGroupAxisCache.find(group);
    const auto intent = it->second;
    m_EmptyGroupAxisCache.erase(it);  // erase before applying so a re-entrant signal cannot re-flush

    const std::size_t dimCount = QmitkMxNAllSyncDimensions.size();
    std::vector<QmitkMxNSyncDimension> dimensions;
    for (std::size_t axis = 0; axis < dimCount; ++axis)
    {
      if (intent[axis])
      {
        dimensions.push_back(QmitkMxNAllSyncDimensions[axis]);
      }
    }
    const bool includeSelection = intent[dimCount];
    for (const auto& windowId : windowIds)
    {
      this->ApplyGroupAxesToCell(windowId, group, dimensions, includeSelection, mode);
    }
    m_MultiWidget->RefreshSyncControls();
    return;
  }

  for (const auto& windowId : windowIds)
  {
    auto dimensions = this->GroupDimensions(group);
    if (dimensions.empty())
    {
      dimensions.assign(NavigationBundle.begin(), NavigationBundle.end());
    }
    this->ApplyGroupAxesToCell(windowId, group, dimensions, this->GroupSelectionEnabled(group), mode);
  }
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::ApplyGroupAxesToCell(
    const QString& windowId, const std::string& group,
    const std::vector<QmitkMxNSyncDimension>& dimensions, bool includeSelection,
    QmitkMxNGroupJoinMode mode)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  try
  {
    if (mode == QmitkMxNGroupJoinMode::Replace)
    {
      // Wholly replace: drop every other tie first (selection reverts to "main").
      ClearOtherGroupTies(m_MultiWidget, windowId, group);
    }
    for (const auto dimension : dimensions)
    {
      // FillEmpty leaves an already-linked axis alone; Replace and Merge set it.
      if (mode == QmitkMxNGroupJoinMode::FillEmpty
          && m_MultiWidget->GetSyncLink(windowId, dimension).has_value())
      {
        continue;
      }
      m_MultiWidget->SetSyncLink(windowId, dimension, group);
    }
    if (includeSelection)
    {
      // FillEmpty adopts the group's selection only for a cell resting on the
      // default group; Replace and Merge move it.
      const bool restingOnDefault =
        (m_MultiWidget->GetCellSelectionGroup(windowId) == DefaultSelectionGroup);
      if (mode != QmitkMxNGroupJoinMode::FillEmpty || restingOnDefault)
      {
        m_MultiWidget->SetCellSelectionGroup(windowId, group);
      }
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: group axis apply for '" << windowId.toStdString()
              << "' ignored: " << e.GetDescription();
  }
}

void QmitkMxNLayoutEditorWidget::ApplyDimensionToGroup(const std::string& group,
                                                       QmitkMxNSyncDimension dimension, bool enabled)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  for (const auto& windowId : this->GroupMembers(group))
  {
    try
    {
      if (enabled)
      {
        m_MultiWidget->SetSyncLink(windowId, dimension, group);
      }
      else
      {
        const auto link = m_MultiWidget->GetSyncLink(windowId, dimension);
        if (link.has_value() && link->group == group)
        {
          m_MultiWidget->ClearSyncLink(windowId, dimension);
        }
      }
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Layout editor: dimension change for '" << windowId.toStdString()
                << "' ignored: " << e.GetDescription();
    }
  }
  // Notify the per-cell furniture (barcodes, frame colors) of the link change;
  // this also drives the editor's own coalesced rebuild via SyncLinksChanged.
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::ApplySelectionToGroup(const std::string& group, bool enabled)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  for (const auto& windowId : this->GroupMembers(group))
  {
    try
    {
      if (enabled)
      {
        m_MultiWidget->SetCellSelectionGroup(windowId, group);
      }
      else
      {
        m_MultiWidget->ClearCellSelectionGroup(windowId);
      }
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Layout editor: selection change for '" << windowId.toStdString()
                << "' ignored: " << e.GetDescription();
    }
  }
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::ToggleGroupAxis(const std::string& groupId, int axisIndex)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  const int dimCount = static_cast<int>(QmitkMxNAllSyncDimensions.size());
  if (axisIndex < 0 || axisIndex > dimCount)  // dimCount + 1 axes: [0, dimCount]
  {
    return;
  }

  const bool empty = this->GroupMembers(groupId).empty();

  // Empty group: toggle the per-group intent cache and repaint the card's barcode
  // from it; the engine is not touched. Configuring an empty group must not assign
  // any cell just because it is the active/selected one - the cache is applied only
  // when windows are explicitly assigned to the group (a drop, or the card menu).
  if (empty)
  {
    auto& intent = m_EmptyGroupAxisCache[groupId];
    const auto axis = static_cast<std::size_t>(axisIndex);
    intent[axis] = !intent[axis];
    // Keep "entry present" == "intent configured": drop an entry that toggling
    // left with no axis on, so BuildGroupBarcodeSlots and AssignCellsToGroup
    // never treat an empty intent as a cache.
    if (!this->HasCachedGroupIntent(groupId))
    {
      m_EmptyGroupAxisCache.erase(groupId);
    }
    // Repaint through the card's own refresher, whose first action re-pushes
    // BuildGroupBarcodeSlots(groupId) into the barcode - which now reads the
    // cache. Reusing the existing seam avoids reaching for the strip by hand.
    if (auto it = m_CardRefreshers.find(groupId); it != m_CardRefreshers.end() && it->second)
    {
      it->second();
    }
    return;
  }

  // Non-empty group: homogenize the axis over the members (link all / unlink all).
  const auto axisSlots = this->BuildGroupBarcodeSlots(groupId);
  if (axisIndex >= axisSlots.size())
  {
    return;
  }
  const bool enable = !(axisSlots[axisIndex].color.isValid() && !axisSlots[axisIndex].partial);
  if (axisIndex < dimCount)
  {
    this->ApplyDimensionToGroup(groupId,
      QmitkMxNAllSyncDimensions[static_cast<std::size_t>(axisIndex)], enable);
  }
  else
  {
    this->ApplySelectionToGroup(groupId, enable);
  }
}

bool QmitkMxNLayoutEditorWidget::GroupSelectionEnabled(const std::string& group) const
{
  if (m_MultiWidget.isNull())
  {
    return false;
  }
  // Computed directly over all cells, not via GroupMembers: since GroupMembers
  // now counts the selection tie, routing through it would make this tautological
  // (every selection member trivially matches) and let selection follow joins
  // more eagerly than intended. "The group synchronizes selection" means at least
  // one cell's selection names it, independent of membership.
  for (const auto& descriptor : m_MultiWidget->ListWindowDescriptors())
  {
    if (m_MultiWidget->GetCellSelectionGroup(descriptor.id) == group)
    {
      return true;
    }
  }
  return false;
}

void QmitkMxNLayoutEditorWidget::SetCellMembership(const QString& windowId,
                                                   const std::string& group, bool member)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  if (member)
  {
    // Adding a cell wholly replaces its membership (mode Replace): it joins the
    // group's currently synchronized dimensions, or the navigation bundle when
    // the group synchronizes nothing yet. The (b)/(c) merge variants are reachable
    // only through the drop selector (AssignCellsToGroup).
    auto dimensions = this->GroupDimensions(group);
    if (dimensions.empty())
    {
      dimensions.assign(NavigationBundle.begin(), NavigationBundle.end());
    }
    this->ApplyGroupAxesToCell(windowId, group, dimensions, this->GroupSelectionEnabled(group),
                               QmitkMxNGroupJoinMode::Replace);
  }
  else
  {
    try
    {
      for (const auto dimension : QmitkMxNAllSyncDimensions)
      {
        const auto link = m_MultiWidget->GetSyncLink(windowId, dimension);
        if (link.has_value() && link->group == group)
        {
          m_MultiWidget->ClearSyncLink(windowId, dimension);
        }
      }
      if (m_MultiWidget->GetCellSelectionGroup(windowId) == group)
      {
        m_MultiWidget->ClearCellSelectionGroup(windowId);
      }
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Layout editor: membership change for '" << windowId.toStdString()
                << "' ignored: " << e.GetDescription();
    }
  }
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::LinkNavigationBundle(const std::string& group)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  for (const auto& windowId : this->GroupMembers(group))
  {
    for (const auto dimension : NavigationBundle)
    {
      try
      {
        m_MultiWidget->SetSyncLink(windowId, dimension, group);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "Layout editor: navigation bundle for '" << windowId.toStdString()
                  << "' ignored: " << e.GetDescription();
      }
    }
  }
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::ReconvergeGroup(const std::string& group)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Zoom,
                                QmitkMxNSyncDimension::Pan })
  {
    const auto groups = m_MultiWidget->GetSyncGroupNames(dimension);
    if (std::find(groups.begin(), groups.end(), group) == groups.end())
    {
      continue;
    }
    try
    {
      m_MultiWidget->ReconvergeSyncGroup(dimension, group);
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Layout editor: re-converge ignored: " << e.GetDescription();
    }
  }
}

void QmitkMxNLayoutEditorWidget::ReinitGroupGeometry(const std::string& group)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  const auto members = this->GroupMembers(group);
  if (members.empty())
  {
    return;
  }
  try
  {
    m_MultiWidget->ReinitSyncGroupGeometry(members.front());
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: geometry reinit ignored: " << e.GetDescription();
  }
}

std::string QmitkMxNLayoutEditorWidget::CreateGroup()
{
  if (m_MultiWidget.isNull())
  {
    return {};
  }

  try
  {
    const auto index = m_MultiWidget->NextFreeSyncGroupIndex();
    m_MultiWidget->AddSynchronizationGroup(index);
    return m_MultiWidget->GetSyncGroupDisplayName(index).toStdString();
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: group creation failed: " << e.GetDescription();
    return {};
  }
}

void QmitkMxNLayoutEditorWidget::ScheduleRebuild()
{
  if (m_RebuildPending)
  {
    return;
  }

  // Deferred: engine signals arrive synchronously from mutations triggered by
  // this widget's own controls; acting immediately could delete a control out
  // from under its own slot. The deferral also coalesces a burst of signals.
  m_RebuildPending = true;
  QTimer::singleShot(0, this, [this]()
  {
    m_RebuildPending = false;
    this->RefreshOrRebuild();
  });
}

void QmitkMxNLayoutEditorWidget::RefreshOrRebuild()
{
  if (m_MultiWidget.isNull())
  {
    this->Rebuild();
    return;
  }

  std::vector<QmitkMxNMultiWidget::SyncGroupInfo> infos;
  try
  {
    infos = m_MultiWidget->GetSyncGroupInfos();
  }
  catch (const mitk::Exception& e)
  {
    // Mid-layout-change states are transient; the next engine signal retries.
    MITK_DEBUG << "Layout editor: skipped refresh: " << e.GetDescription();
    return;
  }

  std::vector<std::string> currentIds;
  currentIds.reserve(infos.size());
  for (const auto& info : infos)
  {
    currentIds.push_back(info.id);
  }

  // The group set is unchanged (the usual case - a link, membership, name, or
  // color edit), so update the existing cards in place. A changed set (a group
  // added or removed, or a whole layout loaded / pushed) is reconciled: only the
  // affected cards change, the others keep their widgets.
  if (currentIds == m_DisplayedGroupIds)
  {
    this->RefreshCards();
  }
  else
  {
    this->ReconcileGroupCards(currentIds, infos);
  }
}

void QmitkMxNLayoutEditorWidget::RefreshCards()
{
  for (const auto& [id, refresher] : m_CardRefreshers)
  {
    refresher();
  }
  // The cell map is one custom-painted widget; recomputing its tiles just
  // repaints (no child widgets, so no flicker).
  m_CellMap->Rebuild();
  this->UpdateGridButtons();
}

void QmitkMxNLayoutEditorWidget::SelectActiveWindowTile()
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  const auto active = m_MultiWidget->GetActiveRenderWindowWidget();
  if (nullptr == active)
  {
    return;
  }
  for (const auto& [windowId, widget] : m_MultiWidget->GetRenderWindowWidgets())
  {
    if (widget == active)
    {
      // Do not collapse an existing multi-selection to the active cell. Selecting
      // tiles makes the first one active, which fires ActiveRenderWindowChanged
      // back into here; without this guard a Ctrl-click multi-selection would
      // immediately shrink to that one cell. Only mirror an external focus change
      // (the active cell is not already part of the map selection).
      if (!m_CellMap->GetSelectedWindowIds().contains(windowId))
      {
        m_CellMap->SetSelectedWindowIds(QStringList{ windowId });
      }
      return;
    }
  }
}

void QmitkMxNLayoutEditorWidget::UpdateGridButtons()
{
  // Gate on the tree-derived grid shape, not the stored row/column counts: a
  // loaded layout reports 0/0, and a render-window layout-design-menu change
  // leaves the stored counts stale-but-nonzero, so a count-based gate would keep
  // the buttons wrongly enabled on a non-grid tree.
  int rows = 0;
  int columns = 0;
  const bool grid = !m_MultiWidget.isNull() && m_MultiWidget->ResolveGridShape(rows, columns);

  m_AddRowButton->setEnabled(grid);
  m_AddColumnButton->setEnabled(grid);
  m_RemoveRowButton->setEnabled(grid && rows > 1);
  m_RemoveColumnButton->setEnabled(grid && columns > 1);

  if (grid)
  {
    m_AddRowButton->setToolTip(tr("Add a row at the bottom"));
    m_AddColumnButton->setToolTip(tr("Add a column at the right"));
    m_RemoveRowButton->setToolTip(rows > 1
      ? tr("Remove the bottom row")
      : tr("A grid must keep at least one row"));
    m_RemoveColumnButton->setToolTip(columns > 1
      ? tr("Remove the rightmost column")
      : tr("A grid must keep at least one column"));
  }
  else
  {
    const QString why = tr("Adding or removing a row or column works only on a regular grid "
                           "layout. Use the Layout controls above to set a grid first.");
    m_AddRowButton->setToolTip(why);
    m_AddColumnButton->setToolTip(why);
    m_RemoveRowButton->setToolTip(why);
    m_RemoveColumnButton->setToolTip(why);
  }
}

void QmitkMxNLayoutEditorWidget::Rebuild()
{
  ClearLayout(m_GroupsLayout);
  m_CardRefreshers.clear();
  m_CardsById.clear();
  m_DisplayedGroupIds.clear();
  m_Matrix->clear();
  m_Matrix->setRowCount(0);
  m_Matrix->setColumnCount(0);

  if (m_MultiWidget.isNull())
  {
    m_CellMap->Rebuild();
    this->UpdateGridButtons();
    return;
  }

  std::vector<QmitkMxNMultiWidget::SyncGroupInfo> infos;
  std::vector<QmitkMxNMultiWidget::WindowDescriptor> descriptors;
  try
  {
    infos = m_MultiWidget->GetSyncGroupInfos();
    descriptors = m_MultiWidget->ListWindowDescriptors();
  }
  catch (const mitk::Exception& e)
  {
    // Mid-layout-change states (no top-level splitter yet) are transient;
    // the next engine signal rebuilds again.
    MITK_DEBUG << "Layout editor: skipped rebuild: " << e.GetDescription();
    return;
  }

  m_CellMap->Rebuild();

  for (const auto& info : infos)
  {
    m_GroupsLayout->addWidget(this->BuildGroupCard(info));
    m_DisplayedGroupIds.push_back(info.id);
  }
  m_GroupsLayout->addStretch();

  this->RebuildMatrix(infos, descriptors);
  this->UpdateGridButtons();
}

void QmitkMxNLayoutEditorWidget::ReconcileGroupCards(
  const std::vector<std::string>& currentIds,
  const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>& infos)
{
  std::map<std::string, const QmitkMxNMultiWidget::SyncGroupInfo*> infoById;
  for (const auto& info : infos)
  {
    infoById[info.id] = &info;
  }
  const auto inCurrent = [&currentIds](const std::string& id)
  {
    return std::find(currentIds.begin(), currentIds.end(), id) != currentIds.end();
  };

  // Delete cards for groups that are gone; their bindings go with them.
  for (const auto& id : m_DisplayedGroupIds)
  {
    if (!inCurrent(id))
    {
      if (auto it = m_CardsById.find(id); it != m_CardsById.end())
      {
        delete it->second.data();  // also removes it from the layout; QPointer nulls
        m_CardsById.erase(it);
      }
      m_CardRefreshers.erase(id);
      m_EmptyGroupAxisCache.erase(id);  // a gone group's pending intent is moot
    }
  }

  // Detach the surviving cards and the trailing stretch from the layout without
  // destroying the cards (deleting a QLayoutItem does not delete its widget), so
  // they can be re-added in the new order - a move, not a teardown, which is what
  // preserves each card's widget identity and live state.
  while (auto* item = m_GroupsLayout->takeAt(0))
  {
    delete item;
  }

  // Re-add in the engine's group order, building cards for ids not yet shown.
  for (const auto& id : currentIds)
  {
    QWidget* card = nullptr;
    if (auto it = m_CardsById.find(id); it != m_CardsById.end() && !it->second.isNull())
    {
      card = it->second;
    }
    else if (auto infoIt = infoById.find(id); infoIt != infoById.end())
    {
      card = this->BuildGroupCard(*infoIt->second);
    }
    if (nullptr != card)
    {
      m_GroupsLayout->addWidget(card);
    }
  }
  m_GroupsLayout->addStretch();

  m_DisplayedGroupIds = currentIds;

  // Refresh the kept cards' contents and the secondary surfaces. The advanced
  // matrix stays a full rebuild (secondary, usually hidden); the cell map is one
  // custom-painted widget that just repaints.
  this->RefreshCards();
  std::vector<QmitkMxNMultiWidget::WindowDescriptor> descriptors;
  try
  {
    descriptors = m_MultiWidget->ListWindowDescriptors();
  }
  catch (const mitk::Exception&)
  {
  }
  this->RebuildMatrix(infos, descriptors);
}

QList<QmitkMxNSyncBarcodeWidget::AxisSlot>
QmitkMxNLayoutEditorWidget::BuildGroupBarcodeSlots(const std::string& group) const
{
  QList<QmitkMxNSyncBarcodeWidget::AxisSlot> result;
  result.reserve(static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1);
  if (m_MultiWidget.isNull())
  {
    return result;
  }

  const auto members = this->GroupMembers(group);
  // An empty group with a configured intent cache renders from the cache (each
  // cached-on axis solid in the group hue), so the user sees the pending
  // configuration before any window is assigned. total = 1 there makes a
  // cached-on axis read as "all", not the partial "some".
  const auto cacheIt = m_EmptyGroupAxisCache.find(group);
  const bool useCache = members.empty() && cacheIt != m_EmptyGroupAxisCache.end();
  const int total = useCache ? 1 : static_cast<int>(members.size());

  QColor hue;
  try
  {
    hue = m_MultiWidget->GetSyncGroupColor(group);
  }
  catch (const mitk::Exception&)
  {
  }

  const auto stateSlot = [total](QmitkMxNAxisGlyph glyph, const QColor& groupHue, int linked,
                                 const QString& label) -> QmitkMxNSyncBarcodeWidget::AxisSlot
  {
    QmitkMxNSyncBarcodeWidget::AxisSlot slot;
    slot.glyph = glyph;
    if (linked > 0 && groupHue.isValid())
    {
      slot.color = groupHue;
      slot.partial = linked < total;
      slot.tooltip = slot.partial ? QObject::tr("%1 - %2 of %3 windows").arg(label).arg(linked).arg(total)
                                   : QObject::tr("%1 - all %2 windows").arg(label).arg(total);
    }
    else
    {
      slot.tooltip = QObject::tr("%1 - no windows").arg(label);
    }
    return slot;
  };

  std::size_t axisIndex = 0;
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    int linked = 0;
    if (useCache)
    {
      linked = cacheIt->second[axisIndex] ? 1 : 0;
    }
    else
    {
      for (const auto& windowId : members)
      {
        const auto link = m_MultiWidget->GetSyncLink(windowId, dimension);
        if (link.has_value() && link->group == group)
        {
          ++linked;
        }
      }
    }
    result.append(stateSlot(GlyphFor(dimension), hue, linked,
                            QString::fromUtf8(DimensionLabel(dimension))));
    ++axisIndex;
  }

  int selectionLinked = 0;
  if (useCache)
  {
    selectionLinked = cacheIt->second[axisIndex] ? 1 : 0;
  }
  else
  {
    for (const auto& windowId : members)
    {
      if (m_MultiWidget->GetCellSelectionGroup(windowId) == group)
      {
        ++selectionLinked;
      }
    }
  }
  result.append(stateSlot(QmitkMxNAxisGlyph::Selection, hue, selectionLinked, tr("Data selection")));

  return result;
}

QWidget* QmitkMxNLayoutEditorWidget::BuildGroupCard(const QmitkMxNMultiWidget::SyncGroupInfo& info)
{
  const auto groupId = info.id;

  auto* card = new GroupCardFrame(
    QString::fromStdString(groupId), info.color,
    [this, groupId](const QStringList& windowIds, QmitkMxNGroupJoinMode mode)
    {
      this->AssignCellsToGroup(windowIds, groupId, mode);
    },
    this);
  // Stable, group-derived object name so the incremental reconcile and tests can
  // find a specific card.
  card->setObjectName(QStringLiteral("mxnGroupCard__") + QString::fromStdString(groupId));
  // A plain (non-hue) box frame delimits each card as its own object.
  card->setFrameShape(QFrame::Box);
  card->setLineWidth(1);
  card->setToolTip(tr("Drag this card onto a render window in the map to assign the group; "
                      "drop cells from the map here to add them. A plain drop replaces; hold "
                      "Alt to merge, Shift to fill only unsynced axes."));
  auto* cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(0, 0, 6, 6);
  cardLayout->setSpacing(4);

  // Header: a solid bar in the group hue - the card's strongest identity cue -
  // carrying the display name, the member count, and a "..." menu for the
  // infrequent and advanced actions.
  auto* header = new QFrame(card);
  header->setAttribute(Qt::WA_StyledBackground, true);
  auto* headerRow = new QHBoxLayout(header);
  headerRow->setContentsMargins(6, 3, 3, 3);

  auto* nameLabel = new QLabel(QString::fromStdString(info.displayName), header);
  headerRow->addWidget(nameLabel, 1);

  auto* countLabel = new QLabel(tr("%n window(s)", nullptr,
                                   static_cast<int>(this->GroupMembers(groupId).size())), header);
  headerRow->addWidget(countLabel);

  auto* menuButton = new QToolButton(header);
  menuButton->setText(QStringLiteral("..."));
  menuButton->setPopupMode(QToolButton::InstantPopup);
  menuButton->setToolTip(tr("Rename, recolor, and group actions"));
  auto* menu = new QMenu(menuButton);
  // Rebuilt each time it opens so the selection-dependent actions reflect the
  // map's current selection.
  connect(menu, &QMenu::aboutToShow, this, [this, groupId, menu]()
  {
    menu->clear();
    const bool hasSelection = !m_CellMap->GetSelectedWindowIds().isEmpty();
    const bool hasMembers = !this->GroupMembers(groupId).empty();

    connect(menu->addAction(tr("Rename...")), &QAction::triggered, this, [this, groupId]()
    {
      if (m_MultiWidget.isNull())
      {
        return;
      }
      bool ok = false;
      const auto text = QInputDialog::getText(
        this, tr("Rename group"), tr("Display name:"), QLineEdit::Normal,
        QString::fromStdString(m_MultiWidget->GetSyncGroupDisplayName(groupId)), &ok);
      if (ok)
      {
        try
        {
          m_MultiWidget->SetSyncGroupDisplayName(groupId, text.trimmed().toStdString());
        }
        catch (const mitk::Exception& e)
        {
          MITK_WARN << "Layout editor: display-name change ignored: " << e.GetDescription();
        }
      }
    });
    connect(menu->addAction(tr("Change color...")), &QAction::triggered, this, [this, groupId]()
    {
      if (m_MultiWidget.isNull())
      {
        return;
      }
      const auto color = QColorDialog::getColor(m_MultiWidget->GetSyncGroupColor(groupId), this);
      if (color.isValid())
      {
        m_MultiWidget->SetSyncGroupColor(groupId, color);
      }
    });

    menu->addSeparator();
    auto* addSelected = menu->addAction(tr("Add selected windows"));
    addSelected->setEnabled(hasSelection);
    connect(addSelected, &QAction::triggered, this, [this, groupId]()
    {
      this->AssignCellsToGroup(m_CellMap->GetSelectedWindowIds(), groupId);
    });
    auto* removeSelected = menu->addAction(tr("Remove selected windows"));
    removeSelected->setEnabled(hasSelection);
    connect(removeSelected, &QAction::triggered, this, [this, groupId]()
    {
      for (const auto& windowId : m_CellMap->GetSelectedWindowIds())
      {
        this->SetCellMembership(windowId, groupId, false);
      }
    });

    menu->addSeparator();
    auto* linkNav = menu->addAction(tr("Link navigation"));
    linkNav->setEnabled(hasMembers);
    connect(linkNav, &QAction::triggered, this, [this, groupId]() { this->LinkNavigationBundle(groupId); });
    auto* reconverge = menu->addAction(tr("Re-converge"));
    reconverge->setEnabled(hasMembers);
    connect(reconverge, &QAction::triggered, this, [this, groupId]() { this->ReconvergeGroup(groupId); });
    auto* reinit = menu->addAction(tr("Reinit geometry"));
    reinit->setEnabled(hasMembers);
    connect(reinit, &QAction::triggered, this, [this, groupId]() { this->ReinitGroupGeometry(groupId); });
  });
  menuButton->setMenu(menu);
  headerRow->addWidget(menuButton);

  const QColor hue = info.color.isValid() ? info.color : card->palette().color(QPalette::Mid);
  StyleGroupHeader(header, nameLabel, countLabel, menuButton, hue);
  cardLayout->addWidget(header);

  // Axis strip: the eight axes as glyphs in the group perspective (all / none /
  // some). Clicking an axis homogenizes the group - "some" or "none" links every
  // member, "all" unlinks them; the granular "some" state is reached from the
  // cell map or the advanced matrix. A group's members are the windows it links
  // on any axis, so an empty group has nothing to homogenize: there, linking an
  // axis instead adds the windows currently selected in the map (select them,
  // then click), which is how a group is built up from scratch.
  auto* strip = new QmitkMxNSyncBarcodeWidget(card);
  strip->SetAxisClickable(true);
  strip->setFixedHeight(24);
  strip->setToolTip(tr("Synchronization axes for this group. Click an axis to link or unlink it "
                       "for every window in the group; a dashed axis is linked for only some. "
                       "For an empty group, select windows in the map first, then click an axis "
                       "to add them."));
  strip->SetSlots(this->BuildGroupBarcodeSlots(groupId));
  connect(strip, &QmitkMxNSyncBarcodeWidget::AxisClicked, this, [this, groupId](int index)
  {
    this->ToggleGroupAxis(groupId, index);
  });
  auto* stripRow = new QHBoxLayout();
  stripRow->setContentsMargins(6, 0, 0, 0);
  stripRow->addWidget(strip);
  cardLayout->addLayout(stripRow);

  // Update the card in place on an engine change without recreating it (avoids
  // the flicker of a full teardown; see RefreshOrRebuild).
  QPointer<QmitkMxNSyncBarcodeWidget> stripPtr = strip;
  QPointer<QLabel> namePtr = nameLabel;
  QPointer<QLabel> countPtr = countLabel;
  QPointer<QFrame> headerPtr = header;
  QPointer<QToolButton> menuPtr = menuButton;
  m_CardsById[groupId] = card;
  m_CardRefreshers[groupId] = [this, groupId, stripPtr, namePtr, countPtr, headerPtr, menuPtr]()
  {
    if (m_MultiWidget.isNull())
    {
      return;
    }
    if (stripPtr)
    {
      stripPtr->SetSlots(this->BuildGroupBarcodeSlots(groupId));
    }
    if (countPtr)
    {
      countPtr->setText(tr("%n window(s)", nullptr, static_cast<int>(this->GroupMembers(groupId).size())));
    }
    try
    {
      if (namePtr)
      {
        namePtr->setText(QString::fromStdString(m_MultiWidget->GetSyncGroupDisplayName(groupId)));
      }
      const QColor refreshedHue = m_MultiWidget->GetSyncGroupColor(groupId);
      if (headerPtr && namePtr && countPtr && menuPtr && refreshedHue.isValid())
      {
        StyleGroupHeader(headerPtr, namePtr, countPtr, menuPtr, refreshedHue);
      }
    }
    catch (const mitk::Exception&)
    {
    }
  };

  return card;
}

void QmitkMxNLayoutEditorWidget::RebuildMatrix(
  const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>& infos,
  const std::vector<QmitkMxNMultiWidget::WindowDescriptor>& descriptors)
{
  QStringList groupNames;
  for (const auto& info : infos)
  {
    groupNames.append(QString::fromStdString(info.id));
  }

  const auto dimensionCount = static_cast<int>(QmitkMxNAllSyncDimensions.size());
  m_Matrix->setColumnCount(dimensionCount);
  m_Matrix->setRowCount(static_cast<int>(descriptors.size()));

  QStringList headers;
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    headers.append(DimensionLabel(dimension));
  }
  m_Matrix->setHorizontalHeaderLabels(headers);

  QStringList rowLabels;
  for (const auto& descriptor : descriptors)
  {
    rowLabels.append(CellLabel(descriptor));
  }
  m_Matrix->setVerticalHeaderLabels(rowLabels);

  for (std::size_t row = 0; row < descriptors.size(); ++row)
  {
    const auto windowId = descriptors[row].id;
    for (std::size_t column = 0; column < QmitkMxNAllSyncDimensions.size(); ++column)
    {
      const auto dimension = QmitkMxNAllSyncDimensions[column];
      const auto link = m_MultiWidget->GetSyncLink(windowId, dimension);

      auto* cellWidget = new QWidget(m_Matrix);
      auto* cellLayout = new QHBoxLayout(cellWidget);
      cellLayout->setContentsMargins(2, 2, 2, 2);

      auto* combo = new QComboBox(cellWidget);
      combo->setEditable(true);  // typing a new name creates the group on commit
      combo->addItem(NotLinkedEntry);
      combo->addItems(groupNames);
      combo->setCurrentText(link.has_value() ? QString::fromStdString(link->group)
                                             : NotLinkedEntry);
      cellLayout->addWidget(combo);

      QSpinBox* sliceOffset = nullptr;
      QDoubleSpinBox* zoomOffset = nullptr;
      QDoubleSpinBox* panOffsetX = nullptr;
      QDoubleSpinBox* panOffsetY = nullptr;
      switch (dimension)
      {
        case QmitkMxNSyncDimension::Slice:
          sliceOffset = new QSpinBox(cellWidget);
          sliceOffset->setRange(-9999, 9999);
          sliceOffset->setToolTip(tr("Slice offset (steps relative to the group seed)"));
          if (link.has_value() && std::holds_alternative<int>(link->offset))
          {
            sliceOffset->setValue(std::get<int>(link->offset));
          }
          cellLayout->addWidget(sliceOffset);
          break;
        case QmitkMxNSyncDimension::Zoom:
          zoomOffset = new QDoubleSpinBox(cellWidget);
          zoomOffset->setRange(0.01, 100.0);
          zoomOffset->setSingleStep(0.1);
          zoomOffset->setValue(1.0);
          zoomOffset->setToolTip(tr("Zoom factor relative to the group seed"));
          if (link.has_value() && std::holds_alternative<double>(link->offset))
          {
            zoomOffset->setValue(std::get<double>(link->offset));
          }
          cellLayout->addWidget(zoomOffset);
          break;
        case QmitkMxNSyncDimension::Pan:
        {
          panOffsetX = new QDoubleSpinBox(cellWidget);
          panOffsetY = new QDoubleSpinBox(cellWidget);
          for (auto* box : { panOffsetX, panOffsetY })
          {
            box->setRange(-1.0e5, 1.0e5);
            box->setToolTip(tr("Pan offset in world mm relative to the group seed"));
            cellLayout->addWidget(box);
          }
          if (link.has_value() && std::holds_alternative<mitk::Vector2D>(link->offset))
          {
            const auto offset = std::get<mitk::Vector2D>(link->offset);
            panOffsetX->setValue(offset[0]);
            panOffsetY->setValue(offset[1]);
          }
          break;
        }
        default:
          break;
      }

      const auto commit = [this, windowId, dimension, combo,
                           sliceOffset, zoomOffset, panOffsetX, panOffsetY]()
      {
        if (m_MultiWidget.isNull())
        {
          return;
        }
        const auto text = combo->currentText().trimmed();
        try
        {
          if (text.isEmpty() || text == NotLinkedEntry)
          {
            m_MultiWidget->ClearSyncLink(windowId, dimension);
          }
          else
          {
            QmitkMxNMultiWidget::SyncOffset offset;
            if (nullptr != sliceOffset)
            {
              offset = sliceOffset->value();
            }
            else if (nullptr != zoomOffset)
            {
              offset = zoomOffset->value();
            }
            else if (nullptr != panOffsetX)
            {
              mitk::Vector2D pan;
              pan[0] = panOffsetX->value();
              pan[1] = panOffsetY->value();
              offset = pan;
            }
            m_MultiWidget->SetSyncLink(windowId, dimension, text.toStdString(), offset);
          }
        }
        catch (const mitk::Exception& e)
        {
          // The engine-signal-driven rebuild snaps the matrix back to the
          // authoritative state.
          MITK_WARN << "Layout editor: link change for '" << windowId.toStdString()
                    << "' ignored: " << e.GetDescription();
        }
        this->ScheduleRebuild();
      };

      connect(combo, QOverload<int>::of(&QComboBox::activated), this, [commit](int) { commit(); });
      connect(combo->lineEdit(), &QLineEdit::editingFinished, this, commit);
      for (auto* box : std::initializer_list<QAbstractSpinBox*>{ sliceOffset, zoomOffset,
                                                                 panOffsetX, panOffsetY })
      {
        if (nullptr != box)
        {
          connect(box, &QAbstractSpinBox::editingFinished, this, commit);
        }
      }

      m_Matrix->setCellWidget(static_cast<int>(row), static_cast<int>(column), cellWidget);
    }
  }
}

bool QmitkMxNLayoutEditorWidget::HasCachedGroupIntent(const std::string& group) const
{
  const auto it = m_EmptyGroupAxisCache.find(group);
  if (it == m_EmptyGroupAxisCache.end())
  {
    return false;
  }
  for (const bool on : it->second)
  {
    if (on)
    {
      return true;
    }
  }
  return false;
}

std::vector<QString> QmitkMxNLayoutEditorWidget::GroupMembers(const std::string& group) const
{
  std::vector<QString> members;
  if (m_MultiWidget.isNull())
  {
    return members;
  }

  for (const auto& descriptor : m_MultiWidget->ListWindowDescriptors())
  {
    // A cell belongs to a group when it is tied on ANY of the eight axes: the
    // seven QmitkMxNSyncDimension links OR its data-selection group. Selection is
    // a different stack part (the node-selection widget's group index) but the
    // same axis to the user - the 8th barcode slot - so membership counts it
    // uniformly, and the group card, count, and axis actions cover it.
    bool member = (m_MultiWidget->GetCellSelectionGroup(descriptor.id) == group);
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      const auto link = m_MultiWidget->GetSyncLink(descriptor.id, dimension);
      if (link.has_value() && link->group == group)
      {
        member = true;
        break;
      }
    }
    if (member)
    {
      members.push_back(descriptor.id);
    }
  }
  return members;
}

std::vector<QmitkMxNSyncDimension> QmitkMxNLayoutEditorWidget::GroupDimensions(const std::string& group) const
{
  std::vector<QmitkMxNSyncDimension> dimensions;
  if (m_MultiWidget.isNull())
  {
    return dimensions;
  }

  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    const auto groups = m_MultiWidget->GetSyncGroupNames(dimension);
    if (std::find(groups.begin(), groups.end(), group) != groups.end())
    {
      dimensions.push_back(dimension);
    }
  }
  return dimensions;
}

QString QmitkMxNLayoutEditorWidget::CellLabel(const QmitkMxNMultiWidget::WindowDescriptor& descriptor)
{
  if (!descriptor.displayName.isEmpty())
  {
    return descriptor.displayName;
  }
  // The bare segment after the editor prefix is friendlier than the fully
  // qualified id and unambiguous within one editor.
  const auto separator = descriptor.id.indexOf(QStringLiteral("__"));
  return separator >= 0 ? descriptor.id.mid(separator + 2) : descriptor.id;
}
