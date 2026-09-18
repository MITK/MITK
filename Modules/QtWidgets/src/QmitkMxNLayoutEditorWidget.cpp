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
#include <QDialog>
#include <QDoubleSpinBox>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
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
   * Tint an advanced-matrix link combo's text with the linked group's hue, so the
   * matrix speaks the same color language as the cards and tiles. A cleared or
   * unlinked combo drops back to the default ink.
   */
  void ColorizeLinkCombo(QComboBox* combo, QmitkMxNMultiWidget* multiWidget, const QString& group)
  {
    QString sheet;
    if (nullptr != multiWidget && !group.isEmpty() && group != NotLinkedEntry)
    {
      try
      {
        const QColor hue = multiWidget->GetSyncGroupColor(group.toStdString());
        if (hue.isValid())
        {
          sheet = QStringLiteral("QComboBox { color: %1; }").arg(hue.name());
        }
      }
      catch (const mitk::Exception&)
      {
      }
    }
    combo->setStyleSheet(sheet);
  }

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
      if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
      {
        m_PressPosition = event->pos();
      }
      QFrame::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
      const bool dragging = event->buttons().testFlag(Qt::LeftButton)
                            || event->buttons().testFlag(Qt::RightButton);
      if (dragging
          && (event->pos() - m_PressPosition).manhattanLength() >= QApplication::startDragDistance())
      {
        auto* mimeData = new QMimeData();
        mimeData->setData(QmitkMxNCellMapWidget::GroupMimeType, m_GroupId.toUtf8());
        if (event->buttons().testFlag(Qt::RightButton))
        {
          // Same gesture as on the cell map: the right button defers the join mode
          // to a menu on drop, so the modifiers stay optional.
          mimeData->setData(QmitkMxNCellMapWidget::AskModeMimeType, QByteArray());
        }
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

      const auto mode = QmitkMxNCellMapWidget::ResolveJoinMode(
        event->mimeData(), event->modifiers(), this,
        this->mapToGlobal(event->position().toPoint()));
      if (!mode.has_value())
      {
        return;
      }

      const auto ids = QString::fromUtf8(
        event->mimeData()->data(QmitkMxNCellMapWidget::CellsMimeType));
      m_OnCellsDropped(ids.split(QStringLiteral("\n"), Qt::SkipEmptyParts), *mode);
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

  // The grid-shape picker (grid size, presets, save/load) is no longer an
  // always-on box: it opens on demand in a modal "Edit grid..." dialog
  // (ShowGridDialog). It is created here, hidden, so the hosting view can bind its
  // data storage and apply signals before the dialog exists, and the same
  // instance is reparented into the dialog and reused on each open.
  m_LayoutSelection = new QmitkMultiWidgetLayoutSelectionWidget(this);
  m_LayoutSelection->hide();

  // The cell map is the primary canvas, front and center; the "Sync groups" box
  // reads as a legend below it. A vertical splitter lets the user trade space
  // between them; its dotted handle mirrors the segmentation view's splitter
  // (darkstyle.qss) so the drag affordance is visible.
  auto* splitter = new QSplitter(Qt::Vertical, this);
  splitter->setObjectName(QStringLiteral("QmitkMxNLayoutEditorSplitter"));
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(2);
  splitter->setStyleSheet(QStringLiteral(
    "QSplitter::handle { margin-top: 4px; margin-bottom: 4px; "
    "border-top: 1px dotted #9e9e9e; border-bottom: 1px dotted #9e9e9e; "
    "background-color: transparent; }"));

  auto* mapPane = new QWidget(splitter);
  auto* mapPaneLayout = new QVBoxLayout(mapPane);
  mapPaneLayout->setContentsMargins(0, 0, 0, 0);

  m_CellMap = new QmitkMxNCellMapWidget(mapPane);
  m_CellMap->setMinimumHeight(200);
  m_CellMap->setToolTip(tr("Select render windows by click, Ctrl-click to toggle one, or "
                           "Shift-click to select the range from the last one clicked; "
                           "assign them by dropping them onto a group (or a group's color "
                           "onto a window). Drag with the right button to pick the join mode "
                           "from a menu on drop; a left drop replaces the window's groups, "
                           "with Alt to merge and Shift to fill only its unsynced axes. Each "
                           "window shows its sync axes as glyphs: the seven dimensions plus "
                           "data selection, tinted by group, a gap where the window is not "
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
  // Hovering a tile's axis glyph lights up every cell that shares that
  // synchronization, so the sync topology is legible at a glance.
  connect(m_CellMap, &QmitkMxNCellMapWidget::GlyphHovered, this,
          &QmitkMxNLayoutEditorWidget::HighlightCellAxis);
  connect(m_CellMap, &QmitkMxNCellMapWidget::GlyphHoverCleared, this,
          &QmitkMxNLayoutEditorWidget::ClearSyncHighlight);
  mapPaneLayout->addWidget(m_CellMap, 1);

  // Grid controls: quick trailing add/remove of a row or column, plus the full
  // "Edit grid..." picker. The +/- operations edit the splitter tree in place, so
  // existing windows keep their ids, sync links, renderer-specific node
  // properties, and positions - only the trailing edge is added (empty) or
  // removed. They require a rectangular grid, which the tree-derived
  // ResolveGridShape decides, so the buttons disable (with an explaining tooltip)
  // for irregular or non-grid layouts.
  auto* gridRow = new QHBoxLayout();
  gridRow->setContentsMargins(0, 0, 0, 0);
  m_RemoveRowButton = new QToolButton(mapPane);
  m_RemoveRowButton->setText(QStringLiteral("-"));
  m_AddRowButton = new QToolButton(mapPane);
  m_AddRowButton->setText(QStringLiteral("+"));
  m_RemoveColumnButton = new QToolButton(mapPane);
  m_RemoveColumnButton->setText(QStringLiteral("-"));
  m_AddColumnButton = new QToolButton(mapPane);
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
  gridRow->addWidget(new QLabel(tr("Rows:"), mapPane));
  gridRow->addWidget(m_RemoveRowButton);
  gridRow->addWidget(m_AddRowButton);
  gridRow->addSpacing(12);
  gridRow->addWidget(new QLabel(tr("Columns:"), mapPane));
  gridRow->addWidget(m_RemoveColumnButton);
  gridRow->addWidget(m_AddColumnButton);
  gridRow->addSpacing(12);
  m_EditGridButton = new QToolButton(mapPane);
  m_EditGridButton->setText(tr("Edit grid..."));
  m_EditGridButton->setToolTip(tr("Choose a grid size or preset, or load or save a layout. "
                                  "Applying a new layout replaces the current window "
                                  "arrangement and its synchronization groups."));
  connect(m_EditGridButton, &QToolButton::clicked, this, [this]() { this->ShowGridDialog(); });
  gridRow->addWidget(m_EditGridButton);
  gridRow->addStretch();
  mapPaneLayout->addLayout(gridRow);

  splitter->addWidget(mapPane);

  // "Sync groups" gathers the group actions and the group-card list into one
  // bounded object below the map. Its header carries the group actions: create a
  // group, and open the advanced per-window matrix.
  auto* groupsBox = new QGroupBox(tr("Sync groups"), splitter);
  auto* groupsBoxLayout = new QVBoxLayout(groupsBox);
  groupsBoxLayout->setContentsMargins(4, 4, 4, 4);

  auto* groupsActionRow = new QHBoxLayout();
  m_AddGroupButton = new QToolButton(groupsBox);
  m_AddGroupButton->setText(tr("+ Group"));
  m_AddGroupButton->setToolTip(tr("Create a new synchronization group"));
  connect(m_AddGroupButton, &QToolButton::clicked, this, [this]() { this->CreateGroup(); });
  groupsActionRow->addWidget(m_AddGroupButton);
  groupsActionRow->addStretch();
  m_AdvancedButton = new QToolButton(groupsBox);
  m_AdvancedButton->setText(tr("Advanced"));
  m_AdvancedButton->setCheckable(true);
  m_AdvancedButton->setToolTip(tr("Show the full per-window, per-dimension link matrix with "
                                  "offset editors below, for precise live editing of the "
                                  "associations"));
  connect(m_AdvancedButton, &QToolButton::toggled, this, [this](bool on)
  {
    m_MatrixPane->setVisible(on);
    if (on)
    {
      this->RebuildMatrixNow();
    }
  });
  groupsActionRow->addWidget(m_AdvancedButton);
  groupsBoxLayout->addLayout(groupsActionRow);

  // Cards scroll when they outgrow the box; no inner frame, so the box border is
  // the only one.
  auto* cardsScroll = new QScrollArea(groupsBox);
  cardsScroll->setWidgetResizable(true);
  cardsScroll->setFrameShape(QFrame::NoFrame);
  auto* groupsContainer = new QWidget(cardsScroll);
  m_GroupsLayout = new QVBoxLayout(groupsContainer);
  m_GroupsLayout->setContentsMargins(0, 0, 0, 0);
  m_GroupsLayout->setSpacing(6);
  m_GroupsLayout->addStretch();
  cardsScroll->setWidget(groupsContainer);
  groupsBoxLayout->addWidget(cardsScroll, 1);

  splitter->addWidget(groupsBox);

  // The advanced matrix stays inside the editor - a third splitter pane, hidden
  // until the "Advanced" toggle reveals it - so associations can be edited live
  // beside the map and cards. It rebuilds only on structural changes (a cell or
  // group added/removed), never on a routine link refresh, so an open combo is
  // not torn down mid-edit.
  m_MatrixPane = new QWidget(splitter);
  auto* matrixPaneLayout = new QVBoxLayout(m_MatrixPane);
  matrixPaneLayout->setContentsMargins(0, 0, 0, 0);
  auto* matrixHint = new QLabel(
    tr("Per-window, per-dimension links. Type a group name to create it; use the offset "
       "editors for the exact per-window relationship. This is the deliberate way to author "
       "a partial ('some windows') group state."),
    m_MatrixPane);
  matrixHint->setWordWrap(true);
  matrixPaneLayout->addWidget(matrixHint);
  m_Matrix = new QTableWidget(m_MatrixPane);
  m_Matrix->setSelectionMode(QAbstractItemView::NoSelection);
  m_Matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_Matrix->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  matrixPaneLayout->addWidget(m_Matrix, 1);
  m_MatrixPane->setVisible(false);
  splitter->addWidget(m_MatrixPane);

  // Cell grid roughly a third, group list two thirds; the cell pane cannot shrink
  // below its content (the map's minimum height). setSizes seeds the initial
  // split; the stretch factors keep the ratio on resize. The hidden matrix pane
  // takes no space until revealed.
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 2);
  splitter->setSizes(QList<int>{ 200, 400 });
  mainLayout->addWidget(splitter, 1);

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

void QmitkMxNLayoutEditorWidget::DeleteGroup(const std::string& group)
{
  // The default group is the appearance/selection home every cell falls back to;
  // it is never removable.
  if (m_MultiWidget.isNull() || group == DefaultSelectionGroup)
  {
    return;
  }
  try
  {
    m_MultiWidget->RemoveSynchronizationGroup(group);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: group removal ignored: " << e.GetDescription();
  }
  // Drop any pending empty-group intent so a later same-id group starts clean.
  m_EmptyGroupAxisCache.erase(group);
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
  // Rebuilds the advanced matrix only when the cell or group set changed (a grid
  // resize, a group added/removed) - a pure link/selection edit leaves both sets
  // untouched, so an open combo is not torn down mid-edit.
  this->RefreshAdvancedMatrixIfVisible();
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
  if (nullptr != m_Matrix)
  {
    m_Matrix->clear();
    m_Matrix->setRowCount(0);
    m_Matrix->setColumnCount(0);
    // Invalidate the reflected-structure signature so the trailing refresh forces
    // a fresh matrix build when the advanced pane is open.
    m_MatrixCellIds.clear();
    m_MatrixGroupIds.clear();
  }

  if (m_MultiWidget.isNull())
  {
    m_CellMap->Rebuild();
    this->UpdateGridButtons();
    return;
  }

  std::vector<QmitkMxNMultiWidget::SyncGroupInfo> infos;
  try
  {
    infos = m_MultiWidget->GetSyncGroupInfos();
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

  this->RefreshAdvancedMatrixIfVisible();
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

  // Refresh the kept cards' contents and the secondary surfaces (cell map, grid
  // buttons, and the advanced matrix if its structure changed); RefreshCards
  // covers them.
  this->RefreshCards();
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

QStringList QmitkMxNLayoutEditorWidget::CellsSharingAxis(const QString& group, int axisIndex) const
{
  QStringList result;
  const int selectionAxis = static_cast<int>(QmitkMxNAllSyncDimensions.size());
  if (m_MultiWidget.isNull() || axisIndex < 0 || axisIndex > selectionAxis)
  {
    return result;
  }

  const auto groupId = group.toStdString();
  try
  {
    for (const auto& info : m_MultiWidget->GetSyncGroupInfos())
    {
      if (info.id != groupId)
      {
        continue;
      }
      if (axisIndex == selectionAxis)
      {
        for (const auto& windowId : info.selectionMembers)
        {
          result.append(windowId);
        }
      }
      else
      {
        // 'members' is keyed only for dimensions some cell links, so a missing
        // key is the common "nobody links this axis" case, not an error.
        const auto dimension = QmitkMxNAllSyncDimensions[static_cast<std::size_t>(axisIndex)];
        const auto it = info.members.find(dimension);
        if (it != info.members.end())
        {
          for (const auto& windowId : it->second)
          {
            result.append(windowId);
          }
        }
      }
      break;
    }
  }
  catch (const mitk::Exception&)
  {
    result.clear();  // transient mid-layout-change state
  }
  return result;
}

void QmitkMxNLayoutEditorWidget::HighlightGroupAxis(const QString& group, int axisIndex)
{
  if (nullptr == m_CellMap)
  {
    return;
  }
  if (axisIndex < 0)
  {
    this->ClearSyncHighlight();
    return;
  }

  const QStringList members = this->CellsSharingAxis(group, axisIndex);
  QColor hue;
  if (!m_MultiWidget.isNull())
  {
    try
    {
      hue = m_MultiWidget->GetSyncGroupColor(group.toStdString());
    }
    catch (const mitk::Exception&)
    {
    }
  }
  m_CellMap->SetHighlightedCells(members, axisIndex, hue);
}

void QmitkMxNLayoutEditorWidget::HighlightCellAxis(const QString& windowId, int axisIndex)
{
  if (m_MultiWidget.isNull() || axisIndex < 0)
  {
    this->ClearSyncHighlight();
    return;
  }

  // Resolve which group the hovered cell is on for this axis, then highlight
  // that group's members. Selection (the last axis) is single-valued per cell
  // and lives on the connector, not in the per-dimension links.
  std::string group;
  if (axisIndex == static_cast<int>(QmitkMxNAllSyncDimensions.size()))
  {
    group = m_MultiWidget->GetCellSelectionGroup(windowId);
  }
  else if (const auto link = m_MultiWidget->GetSyncLink(
             windowId, QmitkMxNAllSyncDimensions[static_cast<std::size_t>(axisIndex)]))
  {
    group = link->group;
  }

  if (group.empty())
  {
    this->ClearSyncHighlight();  // the cell syncs nothing on this axis
    return;
  }
  this->HighlightGroupAxis(QString::fromStdString(group), axisIndex);
}

void QmitkMxNLayoutEditorWidget::ClearSyncHighlight()
{
  if (nullptr != m_CellMap)
  {
    m_CellMap->SetHighlightedCells(QStringList(), -1, QColor());
  }
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
                      "drop cells from the map here to add them. Drag with the right button "
                      "to pick the join mode from a menu on drop; a left drop replaces, with "
                      "Alt to merge and Shift to fill only unsynced axes."));
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

    // The default group is every cell's fallback and cannot be removed; all other
    // groups can. Removing a populated group unsynchronizes its windows, so
    // confirm that case (an empty group carries no state, so it goes quietly).
    if (groupId != DefaultSelectionGroup)
    {
      menu->addSeparator();
      auto* deleteGroup = menu->addAction(tr("Delete group"));
      connect(deleteGroup, &QAction::triggered, this, [this, groupId]()
      {
        if (!this->GroupMembers(groupId).empty())
        {
          const auto answer = QMessageBox::warning(
            this, tr("Delete synchronization group?"),
            tr("Deleting this group unsynchronizes its windows and removes it. "
               "This cannot be undone.\n\nContinue?"),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
          if (QMessageBox::Yes != answer)
          {
            return;
          }
        }
        this->DeleteGroup(groupId);
      });
    }
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
  connect(strip, &QmitkMxNSyncBarcodeWidget::AxisHovered, this, [this, groupId](int index)
  {
    this->HighlightGroupAxis(QString::fromStdString(groupId), index);
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

void QmitkMxNLayoutEditorWidget::ShowGridDialog()
{
  if (nullptr == m_GridDialog)
  {
    m_GridDialog = new QDialog(this);
    m_GridDialog->setWindowTitle(tr("Edit grid layout"));
    m_GridDialog->setModal(true);
    auto* dialogLayout = new QVBoxLayout(m_GridDialog);
    dialogLayout->setContentsMargins(6, 6, 6, 6);
    dialogLayout->addWidget(m_LayoutSelection);  // reparents the picker into the dialog

    // The picker applies through the hosting view, which guards the destructive
    // paths (LayoutSet / preset load / data-based) against silently discarding a
    // non-trivial configuration. Whatever the guard decides, the picker's own
    // controls have finished their gesture, so close the modal when one fires.
    connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LayoutSet,
            m_GridDialog, &QDialog::accept);
    connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LoadLayout,
            m_GridDialog, &QDialog::accept);
    connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SetDataBasedLayout,
            m_GridDialog, &QDialog::accept);
  }

  // Open fresh each time: the picker is a forward chooser, so a stale prior pick
  // would misread as the current grid. It may also have hidden itself after a
  // previous apply, so re-show it.
  m_LayoutSelection->ResetSelection();
  m_LayoutSelection->show();
  m_GridDialog->exec();
}

void QmitkMxNLayoutEditorWidget::RebuildMatrixNow()
{
  if (m_MultiWidget.isNull() || nullptr == m_Matrix)
  {
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
    // Transient mid-layout-change state; the next engine signal refreshes.
    MITK_DEBUG << "Layout editor: skipped matrix rebuild: " << e.GetDescription();
    return;
  }
  this->RebuildMatrix(infos, descriptors);

  // Record the structure the matrix now reflects, so RefreshAdvancedMatrixIfVisible
  // can rebuild on a cell- or group-set change but skip a pure link edit.
  m_MatrixCellIds.clear();
  for (const auto& descriptor : descriptors)
  {
    m_MatrixCellIds.push_back(descriptor.id);
  }
  m_MatrixGroupIds.clear();
  for (const auto& info : infos)
  {
    m_MatrixGroupIds.push_back(info.id);
  }
}

void QmitkMxNLayoutEditorWidget::RefreshAdvancedMatrixIfVisible()
{
  // Only while the matrix is revealed. Rebuild it when the cell set (a grid
  // resize / layout load changes the rows) or the group set (a group added or
  // removed changes the column dropdowns) differs from what it reflects - but
  // never on a pure link/selection edit, so an editable combo the user is
  // interacting with is not torn down mid-edit.
  if (nullptr == m_AdvancedButton || !m_AdvancedButton->isChecked() || m_MultiWidget.isNull())
  {
    return;
  }

  std::vector<QmitkMxNMultiWidget::SyncGroupInfo> infos;
  std::vector<QmitkMxNMultiWidget::WindowDescriptor> descriptors;
  try
  {
    infos = m_MultiWidget->GetSyncGroupInfos();
    descriptors = m_MultiWidget->ListWindowDescriptors();
  }
  catch (const mitk::Exception&)
  {
    return;
  }

  std::vector<QString> cellIds;
  for (const auto& descriptor : descriptors)
  {
    cellIds.push_back(descriptor.id);
  }
  std::vector<std::string> groupIds;
  for (const auto& info : infos)
  {
    groupIds.push_back(info.id);
  }

  if (cellIds != m_MatrixCellIds || groupIds != m_MatrixGroupIds)
  {
    this->RebuildMatrixNow();
  }
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

  // Eight axes: the seven QmitkMxNSyncDimension links plus the data-selection axis
  // in the last column, matching the tiles and group headers.
  const auto dimensionCount = static_cast<int>(QmitkMxNAllSyncDimensions.size());
  const int selectionColumn = dimensionCount;
  m_Matrix->setColumnCount(dimensionCount + 1);
  m_Matrix->setRowCount(static_cast<int>(descriptors.size()));

  // Column headers carry the same axis glyph the tiles and barcodes use, recolored
  // to the header ink, next to the dimension name.
  const QColor headerInk = this->palette().color(QPalette::Text);
  int headerColumn = 0;
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    auto* headerItem = new QTableWidgetItem(QString::fromUtf8(DimensionLabel(dimension)));
    const QPixmap glyph = QmitkMxNRenderAxisGlyph(GlyphFor(dimension), headerInk, 16);
    if (!glyph.isNull())
    {
      headerItem->setIcon(QIcon(glyph));
    }
    m_Matrix->setHorizontalHeaderItem(headerColumn++, headerItem);
  }
  auto* selectionHeader = new QTableWidgetItem(tr("Data selection"));
  const QPixmap selectionGlyph = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Selection, headerInk, 16);
  if (!selectionGlyph.isNull())
  {
    selectionHeader->setIcon(QIcon(selectionGlyph));
  }
  m_Matrix->setHorizontalHeaderItem(selectionColumn, selectionHeader);

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
      ColorizeLinkCombo(combo, m_MultiWidget, combo->currentText());
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
        // Recolor immediately so the hue tracks the selection even when the edit
        // is not structural enough to rebuild the matrix (e.g. switching between
        // two existing groups).
        ColorizeLinkCombo(combo, m_MultiWidget, text);
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

    // The data-selection axis (the 8th): single-valued per cell and offset-free,
    // so a plain combo. Empty reverts the cell to the default group; any other
    // name moves its selection there (creating the group on commit).
    {
      auto* selectionCell = new QWidget(m_Matrix);
      auto* selectionLayout = new QHBoxLayout(selectionCell);
      selectionLayout->setContentsMargins(2, 2, 2, 2);

      auto* selectionCombo = new QComboBox(selectionCell);
      selectionCombo->setEditable(true);
      selectionCombo->addItems(groupNames);
      selectionCombo->setCurrentText(
        QString::fromStdString(m_MultiWidget->GetCellSelectionGroup(windowId)));
      ColorizeLinkCombo(selectionCombo, m_MultiWidget, selectionCombo->currentText());
      selectionLayout->addWidget(selectionCombo);

      const auto commitSelection = [this, windowId, selectionCombo]()
      {
        if (m_MultiWidget.isNull())
        {
          return;
        }
        const auto text = selectionCombo->currentText().trimmed();
        try
        {
          if (text.isEmpty())
          {
            m_MultiWidget->ClearCellSelectionGroup(windowId);
          }
          else
          {
            m_MultiWidget->SetCellSelectionGroup(windowId, text.toStdString());
          }
        }
        catch (const mitk::Exception& e)
        {
          MITK_WARN << "Layout editor: selection change for '" << windowId.toStdString()
                    << "' ignored: " << e.GetDescription();
        }
        ColorizeLinkCombo(selectionCombo, m_MultiWidget, text);
        this->ScheduleRebuild();
      };
      connect(selectionCombo, QOverload<int>::of(&QComboBox::activated), this,
              [commitSelection](int) { commitSelection(); });
      connect(selectionCombo->lineEdit(), &QLineEdit::editingFinished, this, commitSelection);

      m_Matrix->setCellWidget(static_cast<int>(row), selectionColumn, selectionCell);
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

bool QmitkMxNLayoutEditorWidget::HasNonTrivialSyncConfig() const
{
  if (m_MultiWidget.isNull())
  {
    return false;
  }

  std::vector<QmitkMxNMultiWidget::SyncGroupInfo> infos;
  std::vector<QmitkMxNMultiWidget::WindowDescriptor> descriptors;
  try
  {
    infos = m_MultiWidget->GetSyncGroupInfos();
    descriptors = m_MultiWidget->ListWindowDescriptors();
  }
  catch (const mitk::Exception&)
  {
    // Mid-layout-change state: treat as trivial rather than warn spuriously.
    return false;
  }

  // Any group beyond the default "main" is a user-built structure a layout
  // replace would discard - including an empty group the user created but has not
  // populated yet.
  for (const auto& info : infos)
  {
    if (info.id != DefaultSelectionGroup)
    {
      return true;
    }
  }

  // Otherwise the only groups are "main". The configuration is trivial only when
  // every cell rests as a clean Mono("main") - the fresh-cell default, where
  // windowing/LUT/selection are on "main" and nothing else is linked. A cell that
  // spans groups (Complex) or sits wholly on some non-"main" group means the user
  // linked synchronization the warning must cover.
  QColor mainHue;
  try
  {
    mainHue = m_MultiWidget->GetSyncGroupColor(DefaultSelectionGroup);
  }
  catch (const mitk::Exception&)
  {
  }
  for (const auto& descriptor : descriptors)
  {
    const auto identity = m_MultiWidget->ResolveCellGroupIdentity(descriptor.id);
    if (identity.kind != QmitkMxNMultiWidget::CellGroupIdentityKind::Mono)
    {
      return true;
    }
    if (mainHue.isValid() && identity.hue != mainHue)
    {
      return true;
    }
  }
  return false;
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
