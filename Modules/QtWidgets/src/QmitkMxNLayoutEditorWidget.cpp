/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNLayoutEditorWidget.h"

#include "QmitkMultiWidgetLayoutSelectionWidget.h"

#include <QmitkIconTheme.h>
#include <QmitkMxNArrangeMode.h>
#include <QmitkMxNSyncBarcodeWidget.h>

#include <mitkExceptionMacro.h>
#include <mitkLog.h>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCursor>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QItemSelectionModel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <optional>
#include <utility>

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

  // Replace-mode primitive: strip a cell's ties to every group other than
  // 'keepGroup'. The seven dimension axes are unlinked; the selection reverts to
  // the default group (there is no unlinked state for selection). Shared by the
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

  // A group-level link action must leave a cell already linked to the group on
  // that dimension alone: re-linking it would reset its authored offset.
  bool IsLinkedTo(const QmitkMxNMultiWidget* multiWidget, const QString& windowId,
                  QmitkMxNSyncDimension dimension, const std::string& group)
  {
    const auto link = multiWidget->GetSyncLink(windowId, dimension);
    return link.has_value() && link->group == group;
  }

  const QString NotLinkedEntry = QStringLiteral("(not linked)");

  /** Readable ink on a filled group hue, shared by every surface that fills with one. */
  QColor InkFor(const QColor& hue)
  {
    const double luminance = 0.299 * hue.red() + 0.587 * hue.green() + 0.114 * hue.blue();
    return luminance > 140.0 ? QColor(0x1a, 0x1a, 0x1a) : QColor(Qt::white);
  }

  /** A group's hue as a menu / combo swatch. */
  QPixmap SwatchFor(const QColor& hue)
  {
    QPixmap pixmap(12, 12);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(hue.isValid() ? hue : QColor(Qt::gray));
    painter.drawRoundedRect(QRect(0, 0, 12, 12), 2, 2);
    return pixmap;
  }

  // Matrix chip item data. The display role carries the same text unelided, so
  // the view can measure a column and a screen reader still reads something.
  constexpr int ChipGroupRole = Qt::UserRole;
  constexpr int ChipNameRole = Qt::UserRole + 1;
  constexpr int ChipHueRole = Qt::UserRole + 2;
  constexpr int ChipOffsetRole = Qt::UserRole + 3;
  constexpr int ChipHighlightRole = Qt::UserRole + 4;

  // Out-of-domain values the offset editors rest on when the selected cells
  // disagree, surfaced through specialValueText. An offset the user cannot mean,
  // so committing one is refused rather than writing a value nobody chose - the
  // same contract as a text editor showing a blank font size for a mixed
  // selection.
  constexpr int MixedSliceOffset = -10000;
  constexpr double MixedZoomOffset = 0.0;
  constexpr double MixedPanOffset = -100001.0;

  /** A slice offset with its sign, which is how it reads everywhere it is shown. */
  QString SliceOffsetLabel(int steps)
  {
    return QStringLiteral("%1%2").arg(steps > 0 ? "+" : "").arg(steps);
  }


  /**
   * Paints a matrix cell as a chip filled in its group's hue: the group name,
   * elided from the middle so auto-named groups stay apart by their trailing
   * digit, and the offset pinned right where it is never the part that elides.
   * Selection is drawn as a frame rather than a fill, so it never hides the hue
   * it sits on. Paint only - the matrix is a read surface, edited from its
   * action bar.
   */
  class MatrixChipDelegate : public QStyledItemDelegate
  {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing, true);

      auto hue = index.data(ChipHueRole).value<QColor>();
      if (hue.isValid())
      {
        // A chip sharing the hovered synchronization brightens. A frame would be
        // invisible here - the chip is already filled in the group hue - and the
        // selection frame already owns the highlight color.
        if (index.data(ChipHighlightRole).toBool())
        {
          hue = hue.lighter(145);
        }
        const QRect chip = option.rect.adjusted(2, 2, -2, -2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(hue);
        painter->drawRoundedRect(chip, 3, 3);

        painter->setPen(InkFor(hue));
        QRect text = chip.adjusted(4, 0, -4, 0);
        const auto offset = index.data(ChipOffsetRole).toString();
        if (!offset.isEmpty())
        {
          painter->drawText(text, Qt::AlignRight | Qt::AlignVCenter, offset);
          text.setRight(text.right() - option.fontMetrics.horizontalAdvance(offset) - 4);
        }
        if (text.width() > 0)
        {
          painter->drawText(text, Qt::AlignLeft | Qt::AlignVCenter,
                            option.fontMetrics.elidedText(index.data(ChipNameRole).toString(),
                                                          Qt::ElideMiddle, text.width()));
        }
      }

      if (option.state & QStyle::State_Selected)
      {
        QPen pen(option.palette.color(QPalette::Highlight));
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 3, 3);
      }
      painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      auto size = QStyledItemDelegate::sizeHint(option, index);
      size.setWidth(size.width() + 10);  // the chip inset and its inner padding
      size.setHeight(size.height() + 6);
      return size;
    }
  };

  /**
   * Style a group card's header as a solid bar in the group hue, with the name,
   * count, and menu button in a contrasting ink. Shared by card creation and the
   * in-place refresh so a recolor updates both the bar and its text.
   */
  void StyleGroupHeader(QFrame* header, QLabel* name, QLabel* count, QToolButton* menuButton,
                        const QColor& hue)
  {
    const QString ink = InkFor(hue).name();
    header->setStyleSheet(QStringLiteral("background-color: %1; border-top-left-radius: 3px; "
                                         "border-top-right-radius: 3px;").arg(hue.name()));
    name->setStyleSheet(QStringLiteral("color: %1; font-weight: bold; background: transparent;").arg(ink));
    count->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(ink));
    // The "..." already says "menu"; the style's popup arrow would only be
    // drawn over it.
    menuButton->setStyleSheet(
      QStringLiteral("QToolButton { color: %1; background: transparent; border: none; }"
                     "QToolButton::menu-indicator { image: none; }").arg(ink));
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
   * The group card as one coherent object: dragging its background onto a
   * window assigns the group (the whole card is the drag source, not a tiny
   * swatch), and it accepts window drops from the plates, highlighting in the
   * group hue while a drag hovers. Interactive children (glyph strip, menu
   * button) receive their own events first, so the drag starts only from the
   * card's own surface.
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
        mimeData->setData(QmitkMxNGroupMimeType, m_GroupId.toUtf8());
        if (event->buttons().testFlag(Qt::RightButton))
        {
          // Same gesture as on the plates: the right button defers the join mode
          // to a menu on drop, so the modifiers stay optional.
          mimeData->setData(QmitkMxNAskModeMimeType, QByteArray());
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
      if (event->mimeData()->hasFormat(QmitkMxNCellsMimeType))
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

      const auto mode = QmitkMxNResolveJoinMode(
        event->mimeData(), event->modifiers(), this,
        this->mapToGlobal(event->position().toPoint()));
      if (!mode.has_value())
      {
        return;
      }

      const auto ids = QString::fromUtf8(
        event->mimeData()->data(QmitkMxNCellsMimeType));
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
  // The hosting view owns the outer margin; the sections keep the style's own
  // margins and spacing, so the editor reads like the other MITK views.
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // The grid-shape picker is not an always-on box: it opens on demand in a modal
  // "Edit grid..." dialog (ShowGridDialog). It is created here, hidden, so it can
  // take the data storage and serve the presets before the dialog exists, and the
  // same instance is reparented into the dialog and reused on each open.
  m_LayoutSelection = new QmitkMultiWidgetLayoutSelectionWidget(this);
  m_LayoutSelection->hide();

  // Applying a layout is the hosting view's call - it confirms the destructive
  // paths against the live editor - so the picker's actions are forwarded
  // untouched rather than acted on here.
  connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LayoutSet,
          this, &QmitkMxNLayoutEditorWidget::LayoutSet);
  connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SetDataBasedLayout,
          this, &QmitkMxNLayoutEditorWidget::SetDataBasedLayout);
  connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LoadLayout,
          this, &QmitkMxNLayoutEditorWidget::LoadLayout);
  connect(m_LayoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SaveLayout,
          this, &QmitkMxNLayoutEditorWidget::SaveLayout);

  // The layout document's own actions, at the top of the view: a preset, a file
  // load and a save act on the whole document - the arrangement together with
  // its synchronization groups - so none of them is a grid operation, and saving
  // is what a user reaches for after changing only the synchronization.
  // Document and grid actions share one titled section, the counterpart of the
  // segmentation view's data-selection box; the synchronization tabs follow it
  // as the second section.
  auto* layoutBox = new QGroupBox(tr("Layout"), this);
  auto* layoutBoxLayout = new QVBoxLayout(layoutBox);
  auto* documentRow = new QHBoxLayout();

  // The text stays beside the icons: the row is where the layout document is
  // handled, and naming the actions keeps them findable without a tooltip.
  // The size the segmentation view's preset buttons use, so the two views'
  // document actions look alike.
  const QSize documentIconSize(20, 24);
  auto* presetButton = new QToolButton(this);
  presetButton->setText(tr("Presets"));
  presetButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwLayout.svg")));
  presetButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  presetButton->setIconSize(documentIconSize);
  presetButton->setToolTip(tr("Replace the current layout with one of the arrangements that ship "
                              "with MITK"));
  presetButton->setPopupMode(QToolButton::InstantPopup);
  // An instant-popup button draws its arrow inside the content box, over the
  // text; room on the right keeps the arrow as a separate mark.
  presetButton->setStyleSheet(QStringLiteral(
    "QToolButton { padding-right: 14px; }"
    "QToolButton::menu-indicator { subcontrol-origin: padding; subcontrol-position: center right; "
    "right: 2px; }"));
  auto* presetMenu = new QMenu(presetButton);
  const QStringList presetNames = m_LayoutSelection->PresetNames();
  for (int preset = 0; preset < presetNames.size(); ++preset)
  {
    connect(presetMenu->addAction(presetNames[preset]), &QAction::triggered, this,
            [this, preset]() { m_LayoutSelection->ApplyPreset(preset); });
  }
  presetButton->setMenu(presetMenu);
  presetButton->setEnabled(!presetNames.isEmpty());
  documentRow->addWidget(presetButton);

  auto* loadButton = new QToolButton(this);
  loadButton->setText(tr("Load..."));
  loadButton->setIcon(QmitkIconTheme::GetIcon(
    QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-open.svg")));
  loadButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  loadButton->setIconSize(documentIconSize);
  loadButton->setToolTip(tr("Replace the current layout with one read from a layout file"));
  connect(loadButton, &QToolButton::clicked, this, [this]() { m_LayoutSelection->RequestLoad(); });
  documentRow->addWidget(loadButton);

  auto* saveButton = new QToolButton(this);
  saveButton->setText(tr("Save..."));
  saveButton->setIcon(QmitkIconTheme::GetIcon(
    QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg")));
  saveButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  saveButton->setIconSize(documentIconSize);
  saveButton->setToolTip(tr("Write the current layout - the window arrangement and its "
                            "synchronization groups - to a layout file"));
  connect(saveButton, &QToolButton::clicked, this, [this]() { m_LayoutSelection->RequestSave(); });
  documentRow->addWidget(saveButton);
  documentRow->addStretch();
  layoutBoxLayout->addLayout(documentRow);

  // Grid controls: quick trailing add/remove of a row or column, plus the full
  // "Edit grid..." picker. The +/- operations edit the splitter tree in place, so
  // existing windows keep their ids, sync links, renderer-specific node
  // properties, and positions - only the trailing edge is added (empty) or
  // removed. They require a rectangular grid, which the tree-derived
  // ResolveGridShape decides, so the buttons disable (with an explaining tooltip)
  // for irregular or non-grid layouts.
  // Icon-only, like a toolbar: the row label names the group, the icons say
  // which edge changes, and the tooltips (kept current by UpdateGridButtons)
  // carry the words. The text stays set as the buttons' accessible names.
  const QSize gridIconSize(20, 20);
  const auto makeGridButton = [this, gridIconSize](const QString& name, const QString& icon)
  {
    auto* button = new QToolButton(this);
    button->setText(name);
    button->setIcon(QmitkIconTheme::GetIcon(icon));
    button->setIconSize(gridIconSize);
    button->setAutoRaise(true);
    return button;
  };
  auto* gridRow = new QHBoxLayout();
  m_RemoveRowButton = makeGridButton(tr("Remove row"), QStringLiteral(":/Qmitk/mxn-grid-row-remove.svg"));
  m_AddRowButton = makeGridButton(tr("Add row"), QStringLiteral(":/Qmitk/mxn-grid-row-add.svg"));
  m_RemoveColumnButton = makeGridButton(tr("Remove column"), QStringLiteral(":/Qmitk/mxn-grid-column-remove.svg"));
  m_AddColumnButton = makeGridButton(tr("Add column"), QStringLiteral(":/Qmitk/mxn-grid-column-add.svg"));
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
  gridRow->addWidget(new QLabel(tr("Grid:"), this));
  gridRow->addWidget(m_RemoveRowButton);
  gridRow->addWidget(m_AddRowButton);
  gridRow->addSpacing(12);
  gridRow->addWidget(m_RemoveColumnButton);
  gridRow->addWidget(m_AddColumnButton);
  gridRow->addSpacing(12);
  m_EditGridButton = makeGridButton(tr("Edit grid..."), QStringLiteral(":/Qmitk/mxn-grid-edit.svg"));
  m_EditGridButton->setToolTip(tr("Edit grid: choose a grid size, or derive an arrangement from the "
                                  "loaded data. A new grid size re-flows the windows into a plain "
                                  "grid and removes those beyond it; the others keep their "
                                  "synchronization. A data-based arrangement replaces all windows "
                                  "and their synchronization groups."));
  connect(m_EditGridButton, &QToolButton::clicked, this, [this]() { this->ShowGridDialog(); });
  gridRow->addWidget(m_EditGridButton);
  gridRow->addStretch();
  layoutBoxLayout->addLayout(gridRow);
  mainLayout->addWidget(layoutBox);

  // The windows themselves are arranged on their peek plates in the display,
  // which stay up while this view is visible; this line says so, and says why
  // nothing can be arranged while a window is maximized.
  m_ArrangeHint = new QLabel(this);
  m_ArrangeHint->setWordWrap(true);
  m_ArrangeHint->setObjectName(QStringLiteral("QmitkMxNLayoutEditorArrangeHint"));
  mainLayout->addWidget(m_ArrangeHint);
  this->UpdateArrangeHint();

  // Two faces of the same configuration, as tabs: "Sync groups" for the
  // everyday card work, "Advanced" for the per-window link matrix. They are
  // mutually exclusive - one configuration surface at a time.
  m_FacesTab = new QTabWidget(this);
  m_FacesTab->setObjectName(QStringLiteral("QmitkMxNLayoutEditorFaces"));

  auto* groupsPage = new QWidget(m_FacesTab);
  auto* groupsPageLayout = new QVBoxLayout(groupsPage);

  auto* groupsActionRow = new QHBoxLayout();
  m_AddGroupButton = new QToolButton(groupsPage);
  m_AddGroupButton->setText(tr("+ Group"));
  m_AddGroupButton->setToolTip(tr("Create a new synchronization group"));
  connect(m_AddGroupButton, &QToolButton::clicked, this, [this]() { this->CreateGroup(); });
  groupsActionRow->addWidget(m_AddGroupButton);
  groupsActionRow->addStretch();
  groupsPageLayout->addLayout(groupsActionRow);

  // Cards scroll when they outgrow the page; no inner frame, so the tab frame is
  // the only border.
  auto* cardsScroll = new QScrollArea(groupsPage);
  cardsScroll->setWidgetResizable(true);
  cardsScroll->setFrameShape(QFrame::NoFrame);
  auto* groupsContainer = new QWidget(cardsScroll);
  m_GroupsLayout = new QVBoxLayout(groupsContainer);
  m_GroupsLayout->setContentsMargins(0, 0, 0, 0);
  m_GroupsLayout->setSpacing(6);
  m_GroupsLayout->addStretch();
  cardsScroll->setWidget(groupsContainer);
  groupsPageLayout->addWidget(cardsScroll, 1);

  m_FacesTab->addTab(groupsPage, tr("Sync groups"));

  // The advanced matrix is the editor's second face: every window a row, every
  // axis a column. The table itself is a read surface - chips, no editors - and
  // everything is edited from the action bar underneath it, so an edit never
  // covers the grid it acts on. Group creation stays with the cards, which keeps
  // this face to what it is for: association and offset.
  m_MatrixPane = new QWidget(m_FacesTab);
  auto* matrixPaneLayout = new QVBoxLayout(m_MatrixPane);

  // The same action in the same corner as on the cards page, so the gesture
  // carries over between the two faces.
  auto* matrixActionRow = new QHBoxLayout();
  m_MatrixAddGroupButton = new QToolButton(m_MatrixPane);
  m_MatrixAddGroupButton->setText(tr("+ Group"));
  m_MatrixAddGroupButton->setToolTip(tr("Create a new synchronization group"));
  connect(m_MatrixAddGroupButton, &QToolButton::clicked, this, [this]() { this->CreateGroup(); });
  matrixActionRow->addWidget(m_MatrixAddGroupButton);
  matrixActionRow->addStretch();
  matrixPaneLayout->addLayout(matrixActionRow);

  m_Matrix = new QTableWidget(m_MatrixPane);
  m_Matrix->setObjectName(QStringLiteral("QmitkMxNLayoutEditorMatrix"));
  m_Matrix->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Matrix->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Matrix->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Matrix->setItemDelegate(new MatrixChipDelegate(m_Matrix));
  m_Matrix->setContextMenuPolicy(Qt::CustomContextMenu);
  // Clickable headers take the whole line, which is what turns "link every
  // window on one dimension to this group" and "unlink this window entirely"
  // into two gestures. Qt's own header press handles the plain case; eventFilter
  // takes the modified ones over (see there).
  m_Matrix->horizontalHeader()->setSectionsClickable(true);
  m_Matrix->verticalHeader()->setSectionsClickable(true);
  m_Matrix->horizontalHeader()->viewport()->installEventFilter(this);
  m_Matrix->verticalHeader()->viewport()->installEventFilter(this);
  m_Matrix->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  // Only as tall as its rows, so the action bar sits right under the cells it
  // edits; with more windows than fit, the matrix gives way and scrolls.
  m_Matrix->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  m_Matrix->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  matrixPaneLayout->addWidget(m_Matrix);

  // The group picker also as a popup at the pointer, for the single quick edit
  // that does not warrant a trip to the action bar.
  connect(m_Matrix, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos)
  {
    const auto index = m_Matrix->indexAt(pos);
    if (index.isValid() && !m_Matrix->selectionModel()->isSelected(index))
    {
      m_Matrix->setCurrentIndex(index);
    }
    this->ShowMatrixGroupMenu(m_Matrix->viewport()->mapToGlobal(pos));
  });
  connect(m_Matrix, &QTableWidget::doubleClicked, this,
          [this](const QModelIndex&) { this->ShowMatrixGroupMenu(QCursor::pos()); });

  // Hovering a chip lights up every window sharing that synchronization, in the
  // map as well as here - the same resolution a tile's glyph hover runs, so the
  // two surfaces read as one.
  m_Matrix->viewport()->setMouseTracking(true);
  m_Matrix->viewport()->installEventFilter(this);
  connect(m_Matrix, &QAbstractItemView::entered, this, [this](const QModelIndex& index)
  {
    const auto* item = m_Matrix->item(index.row(), index.column());
    const auto group = nullptr != item ? item->data(ChipGroupRole).toString() : QString();
    const auto axis = QmitkMxNSyncAxisFromSlot(index.column());
    if (group.isEmpty() || !axis.has_value())
    {
      this->ClearSyncHighlight();
      return;
    }
    this->HighlightGroupAxis(group, *axis);
  });

  auto* clearShortcut = new QShortcut(QKeySequence::Delete, m_Matrix);
  clearShortcut->setContext(Qt::WidgetShortcut);
  connect(clearShortcut, &QShortcut::activated, this,
          [this]() { this->ApplyGroupToMatrixSelection({}); });

  connect(m_Matrix->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          [this](const QItemSelection&, const QItemSelection&)
  {
    this->UpdateMatrixActionBar();
    if (m_MirroringSelection)
    {
      return;
    }
    QStringList windowIds;
    for (const auto& [windowId, axis] : this->MatrixSelection())
    {
      if (!windowIds.contains(windowId))
      {
        windowIds.append(windowId);
      }
    }
    if (!m_MultiWidget.isNull())
    {
      m_MirroringSelection = true;
      m_MultiWidget->GetArrangeMode()->SetSelectedWindowIds(windowIds);
      m_MirroringSelection = false;
    }
  });

  matrixPaneLayout->addWidget(this->BuildMatrixActionBar());
  matrixPaneLayout->addStretch(1);

  m_FacesTab->addTab(m_MatrixPane, tr("Advanced"));
  m_FacesTab->setTabToolTip(
    m_FacesTab->indexOf(m_MatrixPane),
    tr("The per-window, per-dimension link matrix with its offset editors"));

  // The matrix is only kept current while its face is up (see
  // RefreshAdvancedMatrixIfVisible), so raising the tab has to catch it up on
  // everything that happened while the cards were showing.
  connect(m_FacesTab, &QTabWidget::currentChanged, this, [this](int)
  {
    if (this->AdvancedFaceIsCurrent())
    {
      this->RebuildMatrixNow();
    }
  });

  mainLayout->addWidget(m_FacesTab, 1);

  this->setEnabled(false);
}

QmitkMxNLayoutEditorWidget::~QmitkMxNLayoutEditorWidget()
{
}

bool QmitkMxNLayoutEditorWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (nullptr != m_Matrix && QEvent::Leave == event->type()
      && watched == m_Matrix->viewport())
  {
    this->ClearSyncHighlight();
    return false;  // observed, not consumed: the view still wants its own leave
  }

  if (nullptr != m_Matrix && QEvent::MouseButtonPress == event->type())
  {
    const bool vertical = watched == m_Matrix->verticalHeader()->viewport();
    if (vertical || watched == m_Matrix->horizontalHeader()->viewport())
    {
      auto* mouse = static_cast<QMouseEvent*>(event);
      const auto modifiers = mouse->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
      if (Qt::LeftButton == mouse->button() && modifiers != Qt::NoModifier)
      {
        auto* header = vertical ? m_Matrix->verticalHeader() : m_Matrix->horizontalHeader();
        const auto position = mouse->position().toPoint();
        const int section = header->logicalIndexAt(vertical ? position.y() : position.x());
        if (section >= 0)
        {
          this->SelectMatrixLine(section, vertical, modifiers);
          return true;
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void QmitkMxNLayoutEditorWidget::SelectMatrixLine(int section, bool wholeRow,
                                                  Qt::KeyboardModifiers modifiers)
{
  auto* model = m_Matrix->model();
  auto* selectionModel = m_Matrix->selectionModel();
  const int lastRow = m_Matrix->rowCount() - 1;
  const int lastColumn = m_Matrix->columnCount() - 1;
  if (nullptr == selectionModel || lastRow < 0 || lastColumn < 0)
  {
    return;
  }

  const auto block = [&](int from, int to)
  {
    return wholeRow ? QItemSelection(model->index(std::min(from, to), 0),
                                     model->index(std::max(from, to), lastColumn))
                    : QItemSelection(model->index(0, std::min(from, to)),
                                     model->index(lastRow, std::max(from, to)));
  };

  if (modifiers.testFlag(Qt::ShiftModifier))
  {
    // Extend from the line the last plain press left current, as a file list does.
    const auto current = selectionModel->currentIndex();
    const int anchor = !current.isValid() ? section
                                          : (wholeRow ? current.row() : current.column());
    selectionModel->select(block(anchor, section), QItemSelectionModel::ClearAndSelect);
  }
  else
  {
    const bool selected = wholeRow ? selectionModel->isRowSelected(section)
                                   : selectionModel->isColumnSelected(section);
    selectionModel->select(block(section, section), selected ? QItemSelectionModel::Deselect
                                                             : QItemSelectionModel::Select);
    selectionModel->setCurrentIndex(
      wholeRow ? model->index(section, 0) : model->index(0, section),
      QItemSelectionModel::NoUpdate);
  }
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
    disconnect(m_MultiWidget->GetArrangeMode(), nullptr, this, nullptr);
  }

  m_MultiWidget = multiWidget;
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
    // Reverse of the selection-makes-active link: when the editor's active
    // render window changes (e.g. the user clicks a window), select it. The
    // setters no-op when unchanged, so this does not loop with the forward
    // direction.
    connect(m_MultiWidget, &QmitkMxNMultiWidget::ActiveRenderWindowChanged,
            this, &QmitkMxNLayoutEditorWidget::SelectActiveWindow);
    connect(m_MultiWidget, &QmitkMxNMultiWidget::MaximizedCellChanged,
            this, &QmitkMxNLayoutEditorWidget::UpdateArrangeHint);

    auto* arrangeMode = m_MultiWidget->GetArrangeMode();
    connect(arrangeMode, &QmitkMxNArrangeMode::SelectionChanged, this,
            [this](const QStringList& windowIds)
            {
              // The first selected window becomes the active one, so the plates
              // and the editor's focus stay in step.
              if (!windowIds.isEmpty())
              {
                if (const auto cell = m_MultiWidget->GetRenderWindowWidget(windowIds.first()))
                {
                  m_MultiWidget->SetActiveRenderWindowWidget(cell);
                }
              }
              this->MirrorSelectionToMatrix(windowIds);
              this->ScheduleRebuild();
            });
    connect(arrangeMode, &QmitkMxNArrangeMode::AssignRequested, this,
            [this](const QString& group, const QStringList& windowIds, QmitkMxNGroupJoinMode mode)
            {
              this->AssignCellsToGroup(windowIds, group.toStdString(), mode);
            });
    connect(arrangeMode, &QmitkMxNArrangeMode::RemoveRequested, this,
            [this](const QString& group, const QStringList& windowIds)
            {
              for (const auto& windowId : windowIds)
              {
                this->SetCellMembership(windowId, group.toStdString(), false);
              }
            });
  }

  this->setEnabled(!m_MultiWidget.isNull());
  this->UpdateArrangeHint();
  this->ScheduleRebuild();
}

QmitkMxNMultiWidget* QmitkMxNLayoutEditorWidget::GetMultiWidget() const
{
  return m_MultiWidget;
}

void QmitkMxNLayoutEditorWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_LayoutSelection->SetDataStorage(dataStorage);
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
      // Wholly replace: drop every other tie first (selection reverts to the
      // default group).
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
      if (IsLinkedTo(m_MultiWidget, windowId, dimension, group))
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
        (m_MultiWidget->GetCellSelectionGroup(windowId) == m_MultiWidget->GetDefaultSyncGroupName());
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
      const bool linked = IsLinkedTo(m_MultiWidget, windowId, dimension, group);
      if (enabled && !linked)
      {
        m_MultiWidget->SetSyncLink(windowId, dimension, group);
      }
      else if (!enabled && linked)
      {
        m_MultiWidget->ClearSyncLink(windowId, dimension);
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

void QmitkMxNLayoutEditorWidget::ToggleGroupAxis(const std::string& groupId, QmitkMxNSyncAxis axis)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  const int slot = QmitkMxNSyncAxisToSlot(axis);

  const bool empty = this->GroupMembers(groupId).empty();

  // Empty group: toggle the per-group intent cache and repaint the card's barcode
  // from it; the engine is not touched. Configuring an empty group must not assign
  // any cell just because it is the active/selected one - the cache is applied only
  // when windows are explicitly assigned to the group (a drop, or the card menu).
  if (empty)
  {
    auto& intent = m_EmptyGroupAxisCache[groupId];
    intent[slot] = !intent[slot];
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
  if (slot >= axisSlots.size())
  {
    return;
  }
  const bool enable = !(axisSlots[slot].color.isValid() && !axisSlots[slot].partial);
  if (const auto dimension = QmitkMxNSyncAxisDimension(axis))
  {
    this->ApplyDimensionToGroup(groupId, *dimension, enable);
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
  // counts the selection tie, routing through it would make this tautological
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
    // the group synchronizes nothing yet. The FillEmpty and
    // MergeOverwriteCollisions variants are reachable only through the drop
    // selector (AssignCellsToGroup).
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
        if (!IsLinkedTo(m_MultiWidget, windowId, dimension, group))
        {
          m_MultiWidget->SetSyncLink(windowId, dimension, group);
        }
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
    return m_MultiWidget->GetSyncGroupName(index);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: group creation failed: " << e.GetDescription();
    return {};
  }
}

void QmitkMxNLayoutEditorWidget::DeleteGroup(const std::string& group)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  try
  {
    // The default group is the appearance/selection home every cell falls back to;
    // it is never removable.
    if (group == m_MultiWidget->GetDefaultSyncGroupName())
    {
      return;
    }
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
  this->UpdateGridButtons();
  // Rebuilds the advanced matrix only when the cell or group set changed (a grid
  // resize, a group added/removed) - a pure link/selection edit leaves both sets
  // untouched, so an open combo is not torn down mid-edit.
  this->RefreshAdvancedMatrixIfVisible();
}

void QmitkMxNLayoutEditorWidget::SelectActiveWindow()
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
  auto* arrangeMode = m_MultiWidget->GetArrangeMode();
  for (const auto& [windowId, widget] : m_MultiWidget->GetRenderWindowWidgets())
  {
    if (widget == active)
    {
      // Do not collapse an existing multi-selection to the active cell.
      // Selecting cells makes the first one active, which fires
      // ActiveRenderWindowChanged back into here; without this guard a
      // Ctrl-click multi-selection would immediately shrink to that one cell.
      // Only mirror an external focus change (the active cell is not already
      // part of the selection).
      if (!arrangeMode->GetSelectedWindowIds().contains(windowId))
      {
        arrangeMode->SetSelectedWindowIds(QStringList{ windowId });
      }
      return;
    }
  }
}

void QmitkMxNLayoutEditorWidget::UpdateArrangeHint()
{
  if (nullptr == m_ArrangeHint)
  {
    return;
  }
  const bool maximized = !m_MultiWidget.isNull() && !m_MultiWidget->GetMaximizedCell().isEmpty();
  // While maximized the hint is a warning - windows the user may mean to
  // arrange are out of reach - so it takes the theme's warning styling.
  const QString warningColor = QmitkIconTheme::GetWarningColor();
  m_ArrangeHint->setStyleSheet(!maximized ? QString()
                               : warningColor.isEmpty() ? QStringLiteral("font-weight: bold;")
                               : QStringLiteral("color: %1; font-weight: bold;").arg(warningColor));
  m_ArrangeHint->setText(maximized
    ? tr("A window is maximized: only its plate is shown, and the hidden windows cannot be "
         "arranged until the grid is restored.")
    : tr("Select windows on their plates in the display, then drag them onto a group - or drop "
         "a group onto a window. Press F1 for all gestures."));
}

QStringList QmitkMxNLayoutEditorWidget::SelectedWindowIds() const
{
  return m_MultiWidget.isNull() ? QStringList() : m_MultiWidget->GetArrangeMode()->GetSelectedWindowIds();
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
    // a fresh matrix build when the advanced face is up.
    m_MatrixCellIds.clear();
    m_MatrixGroupIds.clear();
  }

  if (m_MultiWidget.isNull())
  {
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

  // Refresh the kept cards' contents and the secondary surfaces (grid buttons,
  // and the advanced matrix if its structure changed); RefreshCards
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

  // The group perspective speaks for several windows at once, so it marks that
  // an offset exists somewhere in the group but leaves the number to the
  // per-window surfaces, which can say whose it is.
  const auto stateSlot = [total](QmitkMxNAxisGlyph glyph, const QColor& groupHue, int linked,
                                 bool anyOffset,
                                 const QString& label) -> QmitkMxNSyncBarcodeWidget::AxisSlot
  {
    QmitkMxNSyncBarcodeWidget::AxisSlot slot;
    slot.glyph = glyph;
    if (linked > 0 && groupHue.isValid())
    {
      slot.color = groupHue;
      slot.partial = linked < total;
      slot.hasOffset = anyOffset;
      slot.tooltip = slot.partial ? QObject::tr("%1 - %2 of %3 windows").arg(label).arg(linked).arg(total)
                                   : QObject::tr("%1 - all %2 windows").arg(label).arg(total);
      if (anyOffset)
      {
        slot.tooltip += QObject::tr(", some offset from the group");
      }
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
    bool anyOffset = false;
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
          anyOffset = anyOffset
                      || !QmitkMxNMultiWidget::FormatSyncOffset(dimension, link->offset).isEmpty();
        }
      }
    }
    result.append(stateSlot(GlyphFor(dimension), hue, linked, anyOffset,
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
  // Data selection has no offset to carry, so it never takes the footnote mark.
  result.append(
    stateSlot(QmitkMxNAxisGlyph::Selection, hue, selectionLinked, false, tr("Data selection")));

  return result;
}

void QmitkMxNLayoutEditorWidget::HighlightGroupAxis(const QString& group, QmitkMxNSyncAxis axis)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  const QStringList members = m_MultiWidget->CellsSharingAxis(group, axis);
  QColor hue;
  try
  {
    hue = m_MultiWidget->GetSyncGroupColor(group.toStdString());
  }
  catch (const mitk::Exception&)
  {
  }
  this->SetMatrixHighlight(members, QmitkMxNSyncAxisToSlot(axis));
  m_MultiWidget->GetArrangeMode()->SetHighlight(QmitkMxNArrangeMode::HighlightSource::Editor, axis,
                                                members, hue);
}

void QmitkMxNLayoutEditorWidget::HighlightCellAxis(const QString& windowId, QmitkMxNSyncAxis axis)
{
  if (m_MultiWidget.isNull())
  {
    this->ClearSyncHighlight();
    return;
  }

  const std::string group = m_MultiWidget->ResolveCellAxisGroup(windowId, axis);
  if (group.empty())
  {
    this->ClearSyncHighlight();  // the cell syncs nothing on this axis
    return;
  }
  this->HighlightGroupAxis(QString::fromStdString(group), axis);
}

void QmitkMxNLayoutEditorWidget::ClearSyncHighlight()
{
  this->SetMatrixHighlight(QStringList(), -1);
  if (!m_MultiWidget.isNull())
  {
    m_MultiWidget->GetArrangeMode()->ClearHighlight(QmitkMxNArrangeMode::HighlightSource::Editor);
  }
}

void QmitkMxNLayoutEditorWidget::SetMatrixHighlight(const QStringList& windowIds, int column)
{
  if (nullptr == m_Matrix)
  {
    return;
  }

  // Touch only what changes: hover moves arrive per mouse move, and repainting
  // the whole grid for each would be work the eye never sees.
  for (const auto& [row, column] : m_MatrixHighlighted)
  {
    if (auto* item = m_Matrix->item(row, column))
    {
      item->setData(ChipHighlightRole, false);
    }
  }
  m_MatrixHighlighted.clear();

  if (column < 0 || column >= m_Matrix->columnCount())
  {
    return;
  }
  for (int row = 0; row < m_Matrix->rowCount(); ++row)
  {
    const auto id = static_cast<std::size_t>(row) < m_MatrixCellIds.size()
                      ? m_MatrixCellIds[static_cast<std::size_t>(row)]
                      : QString();
    if (windowIds.contains(id))
    {
      if (auto* item = m_Matrix->item(row, column))
      {
        item->setData(ChipHighlightRole, true);
        m_MatrixHighlighted.emplace_back(row, column);
      }
    }
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
  card->setToolTip(tr("Drop windows here to add them to this group, or drag the card onto a "
                      "window"));
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
  // current window selection.
  connect(menu, &QMenu::aboutToShow, this, [this, groupId, menu]()
  {
    menu->clear();
    const bool hasSelection = !this->SelectedWindowIds().isEmpty();
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
      this->AssignCellsToGroup(this->SelectedWindowIds(), groupId);
    });
    auto* removeSelected = menu->addAction(tr("Remove selected windows"));
    removeSelected->setEnabled(hasSelection);
    connect(removeSelected, &QAction::triggered, this, [this, groupId]()
    {
      for (const auto& windowId : this->SelectedWindowIds())
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
    auto* reinit = menu->addAction(tr("Fit views to visible data"));
    reinit->setEnabled(hasMembers);
    connect(reinit, &QAction::triggered, this, [this, groupId]() { this->ReinitGroupGeometry(groupId); });

    // The default group is every cell's fallback and cannot be removed; all other
    // groups can. Removing a populated group unsynchronizes its windows, so
    // confirm that case (an empty group carries no state, so it goes quietly).
    // While the default group's name cannot be resolved (mid-layout-change),
    // conservatively omit the action rather than risk offering to delete it.
    bool canDeleteGroup = false;
    try
    {
      canDeleteGroup = (groupId != m_MultiWidget->GetDefaultSyncGroupName());
    }
    catch (const mitk::Exception&)
    {
    }
    if (canDeleteGroup)
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
  // advanced matrix. A group's members are the windows it links on any axis, so
  // an empty group has nothing to homogenize: there, clicking an axis only
  // records which axes the first windows assigned to it will get.
  auto* strip = new QmitkMxNSyncBarcodeWidget(card);
  strip->SetAxisClickable(true);
  strip->setFixedHeight(24);
  strip->setToolTip(tr("The dimensions this group synchronizes. Click an axis to link or "
                       "unlink it for the whole group"));
  strip->SetSlots(this->BuildGroupBarcodeSlots(groupId));
  connect(strip, &QmitkMxNSyncBarcodeWidget::AxisClicked, this, [this, groupId](int index)
  {
    if (const auto axis = QmitkMxNSyncAxisFromSlot(index))
    {
      this->ToggleGroupAxis(groupId, *axis);
    }
  });
  connect(strip, &QmitkMxNSyncBarcodeWidget::AxisHovered, this, [this, groupId](int index)
  {
    if (const auto axis = QmitkMxNSyncAxisFromSlot(index))
    {
      this->HighlightGroupAxis(QString::fromStdString(groupId), *axis);
    }
    else
    {
      this->ClearSyncHighlight();
    }
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

    // The picker applies through the hosting view, which guards the paths that
    // rebuild every window (data-based, preset or file load) against silently
    // discarding a non-trivial configuration. Whatever the guard decides, the
    // picker's own controls have finished their gesture, so close the modal
    // when one fires.
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

  // An edit can empty a group out of existence, which is a structural change and
  // so a full rebuild - mid-gesture. Carrying the selection by (window, axis)
  // rather than by (row, column) keeps it over any such rebuild.
  const auto selection = this->MatrixSelection();

  this->RebuildMatrix(infos, descriptors);

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

  this->RefreshMatrixCells();

  // Measure the columns against the content once, then hold them: the chips are
  // repainted on every engine change, and a width that tracked their text would
  // shift the grid under the pointer mid-interaction.
  const int columnCount = m_Matrix->columnCount();
  m_Matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_Matrix->resizeColumnsToContents();
  std::vector<int> widths;
  widths.reserve(static_cast<std::size_t>(columnCount));
  for (int column = 0; column < columnCount; ++column)
  {
    widths.push_back(std::max(m_Matrix->columnWidth(column), 34));
  }
  m_Matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  for (int column = 0; column < columnCount; ++column)
  {
    m_Matrix->setColumnWidth(column, widths[static_cast<std::size_t>(column)]);
  }

  QItemSelection restored;
  for (const auto& [windowId, axis] : selection)
  {
    const auto row = std::find(m_MatrixCellIds.begin(), m_MatrixCellIds.end(), windowId);
    const int column = QmitkMxNSyncAxisToSlot(axis);
    if (row != m_MatrixCellIds.end() && column < columnCount)
    {
      const auto index = m_Matrix->model()->index(
        static_cast<int>(std::distance(m_MatrixCellIds.begin(), row)), column);
      restored.select(index, index);
    }
  }
  if (!restored.isEmpty())
  {
    m_Matrix->selectionModel()->select(restored, QItemSelectionModel::ClearAndSelect);
  }
  this->UpdateMatrixActionBar();
}

bool QmitkMxNLayoutEditorWidget::AdvancedFaceIsCurrent() const
{
  // The raised tab, not widget visibility: a docked-away view hides every child,
  // and reading that as "matrix not shown" would skip the rebuilds that keep it
  // current, leaving a stale matrix when the view comes back.
  return nullptr != m_FacesTab && nullptr != m_MatrixPane
      && m_FacesTab->currentWidget() == m_MatrixPane;
}

void QmitkMxNLayoutEditorWidget::RefreshAdvancedMatrixIfVisible()
{
  // Only while the matrix face is up. A changed cell set (a grid resize or a
  // layout load changes the rows) or group set needs the whole table back; for
  // everything else repainting the chips in place is enough, and it leaves the
  // measured column widths and the selection alone.
  if (!this->AdvancedFaceIsCurrent() || m_MultiWidget.isNull())
  {
    return;
  }

  std::vector<QString> cellIds;
  std::vector<std::string> groupIds;
  try
  {
    for (const auto& descriptor : m_MultiWidget->ListWindowDescriptors())
    {
      cellIds.push_back(descriptor.id);
    }
    for (const auto& info : m_MultiWidget->GetSyncGroupInfos())
    {
      groupIds.push_back(info.id);
    }
  }
  catch (const mitk::Exception&)
  {
    return;
  }

  if (cellIds != m_MatrixCellIds || groupIds != m_MatrixGroupIds)
  {
    this->RebuildMatrixNow();
  }
  else
  {
    this->RefreshMatrixCells();
    this->UpdateMatrixActionBar();
  }
}

void QmitkMxNLayoutEditorWidget::RebuildMatrix(
  const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>&,
  const std::vector<QmitkMxNMultiWidget::WindowDescriptor>& descriptors)
{
  // Eight axes: the seven QmitkMxNSyncDimension links plus the data-selection axis
  // in the last column, matching the plates and group headers.
  const auto dimensionCount = static_cast<int>(QmitkMxNAllSyncDimensions.size());
  m_Matrix->clear();
  m_Matrix->setColumnCount(dimensionCount + 1);
  m_Matrix->setRowCount(static_cast<int>(descriptors.size()));

  // Glyph-only column headers: the same axis glyph the plates and barcodes use,
  // named in the tooltip. Eight spelled-out dimension names do not fit a docked
  // view, and the glyphs are the language the rest of the editor speaks.
  const QColor headerInk = this->palette().color(QPalette::Text);
  int headerColumn = 0;
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    auto* headerItem = new QTableWidgetItem();
    const QPixmap glyph = QmitkMxNRenderAxisGlyph(GlyphFor(dimension), headerInk, 16);
    if (!glyph.isNull())
    {
      headerItem->setIcon(QIcon(glyph));
    }
    headerItem->setToolTip(QString::fromUtf8(DimensionLabel(dimension)));
    m_Matrix->setHorizontalHeaderItem(headerColumn++, headerItem);
  }
  auto* selectionHeader = new QTableWidgetItem();
  const QPixmap selectionGlyph = QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph::Selection, headerInk, 16);
  if (!selectionGlyph.isNull())
  {
    selectionHeader->setIcon(QIcon(selectionGlyph));
  }
  selectionHeader->setToolTip(tr("Data selection"));
  m_Matrix->setHorizontalHeaderItem(dimensionCount, selectionHeader);

  QStringList rowLabels;
  for (const auto& descriptor : descriptors)
  {
    rowLabels.append(CellLabel(descriptor));
  }
  m_Matrix->setVerticalHeaderLabels(rowLabels);

  // The recorded highlight indexes point at items that are about to be replaced.
  m_MatrixHighlighted.clear();

  // Empty items now; RefreshMatrixCells fills every chip from the engine, on
  // this build and on every later edit.
  for (int row = 0; row < static_cast<int>(descriptors.size()); ++row)
  {
    for (int column = 0; column <= dimensionCount; ++column)
    {
      m_Matrix->setItem(row, column, new QTableWidgetItem());
    }
  }
}

void QmitkMxNLayoutEditorWidget::RefreshMatrixCells()
{
  if (m_MultiWidget.isNull() || nullptr == m_Matrix)
  {
    return;
  }

  const auto dimensionCount = static_cast<int>(QmitkMxNAllSyncDimensions.size());
  for (int row = 0; row < m_Matrix->rowCount(); ++row)
  {
    if (row >= static_cast<int>(m_MatrixCellIds.size()))
    {
      break;
    }
    const auto windowId = m_MatrixCellIds[static_cast<std::size_t>(row)];
    for (int column = 0; column <= dimensionCount && column < m_Matrix->columnCount(); ++column)
    {
      auto* item = m_Matrix->item(row, column);
      if (nullptr == item)
      {
        continue;
      }

      std::string group;
      QString offset;
      QString axisName;
      try
      {
        if (column == dimensionCount)
        {
          // Data selection always resolves to a group - it has no unlinked
          // state - so this column is never empty.
          group = m_MultiWidget->GetCellSelectionGroup(windowId);
          axisName = tr("Data selection");
        }
        else
        {
          const auto dimension = QmitkMxNAllSyncDimensions[static_cast<std::size_t>(column)];
          axisName = QString::fromUtf8(DimensionLabel(dimension));
          if (const auto link = m_MultiWidget->GetSyncLink(windowId, dimension))
          {
            group = link->group;
            offset = QmitkMxNMultiWidget::FormatSyncOffset(dimension, link->offset);
          }
        }
      }
      catch (const mitk::Exception&)
      {
        // Transient mid-layout-change state; the next engine signal refreshes.
        continue;
      }

      if (group.empty())
      {
        item->setData(ChipGroupRole, QString());
        item->setData(ChipNameRole, QString());
        item->setData(ChipHueRole, QColor());
        item->setData(ChipOffsetRole, QString());
        item->setText(QString());
        item->setToolTip(tr("%1: not linked").arg(axisName));
        continue;
      }

      QString displayName = QString::fromStdString(group);
      QColor hue;
      try
      {
        displayName = QString::fromStdString(m_MultiWidget->GetSyncGroupDisplayName(group));
        hue = m_MultiWidget->GetSyncGroupColor(group);
      }
      catch (const mitk::Exception&)
      {
      }
      if (!hue.isValid())
      {
        hue = this->palette().color(QPalette::Mid);
      }

      item->setData(ChipGroupRole, QString::fromStdString(group));
      item->setData(ChipNameRole, displayName);
      item->setData(ChipHueRole, hue);
      item->setData(ChipOffsetRole, offset);
      // Unelided, so the column measurement and anything reading the model
      // rather than the painted chip see the whole text.
      item->setText(offset.isEmpty() ? displayName
                                     : QStringLiteral("%1 %2").arg(displayName, offset));
      item->setToolTip(offset.isEmpty()
                         ? tr("%1: %2").arg(axisName, displayName)
                         : tr("%1: %2, offset %3").arg(axisName, displayName, offset));
    }
  }

  // Grow a column a chip has outgrown, never shrink one. Growing keeps group
  // names whole - a name is only ever shortened by a width the user chose
  // themselves - while never shrinking keeps the grid from shifting under the
  // pointer between two edits. resizeColumnsToContents measures synchronously
  // against the current chips, so widening is its result taken as a floor.
  const int columnCount = m_Matrix->columnCount();
  std::vector<int> held;
  held.reserve(static_cast<std::size_t>(columnCount));
  for (int column = 0; column < columnCount; ++column)
  {
    held.push_back(m_Matrix->columnWidth(column));
  }
  m_Matrix->resizeColumnsToContents();
  for (int column = 0; column < columnCount; ++column)
  {
    m_Matrix->setColumnWidth(
      column, std::max(held[static_cast<std::size_t>(column)], m_Matrix->columnWidth(column)));
  }
}

QmitkMxNLayoutEditorWidget::MatrixCellContent
QmitkMxNLayoutEditorWidget::AdvancedMatrixCell(const QString& windowId, QmitkMxNSyncAxis axis) const
{
  MatrixCellContent content;
  if (nullptr == m_Matrix)
  {
    return content;
  }
  const int column = QmitkMxNSyncAxisToSlot(axis);
  const auto row = std::find(m_MatrixCellIds.begin(), m_MatrixCellIds.end(), windowId);
  if (row == m_MatrixCellIds.end() || column >= m_Matrix->columnCount())
  {
    return content;
  }
  if (const auto* item = m_Matrix->item(
        static_cast<int>(std::distance(m_MatrixCellIds.begin(), row)), column))
  {
    content.group = item->data(ChipGroupRole).toString().toStdString();
    content.offset = item->data(ChipOffsetRole).toString();
    content.highlighted = item->data(ChipHighlightRole).toBool();
  }
  return content;
}

QWidget* QmitkMxNLayoutEditorWidget::BuildMatrixActionBar()
{
  auto* bar = new QFrame(m_MatrixPane);
  bar->setFrameShape(QFrame::StyledPanel);
  auto* barLayout = new QVBoxLayout(bar);
  barLayout->setContentsMargins(6, 6, 6, 6);
  barLayout->setSpacing(4);

  // Doubles as the face's standing explanation while nothing is selected, so
  // the hint costs no permanent space once the user is working.
  auto* descriptionRow = new QHBoxLayout();
  descriptionRow->setContentsMargins(0, 0, 0, 0);
  descriptionRow->setSpacing(6);
  m_MatrixAxisIconLabel = new QLabel(bar);
  m_MatrixAxisIconLabel->setObjectName(QStringLiteral("mxnMatrixAxisIcon"));
  m_MatrixAxisIconLabel->setVisible(false);
  descriptionRow->addWidget(m_MatrixAxisIconLabel, 0, Qt::AlignTop);
  m_MatrixSelectionLabel = new QLabel(bar);
  m_MatrixSelectionLabel->setWordWrap(true);
  descriptionRow->addWidget(m_MatrixSelectionLabel, 1);
  barLayout->addLayout(descriptionRow);

  auto* groupRow = new QHBoxLayout();
  groupRow->addWidget(new QLabel(tr("Group"), bar));
  m_MatrixGroupPicker = new QComboBox(bar);
  m_MatrixGroupPicker->setToolTip(tr("Assign the selected cells to a group. '%1' removes the "
                                     "link; on the data-selection axis, which is always in some "
                                     "group, it returns the window to the default one. Groups "
                                     "are created with '+ Group'.").arg(NotLinkedEntry));
  // activated, not currentIndexChanged: repopulating the picker for a new
  // selection must not write that selection's own group back to the engine.
  connect(m_MatrixGroupPicker, &QComboBox::activated, this, [this](int index)
  {
    this->ApplyGroupToMatrixSelection(
      m_MatrixGroupPicker->itemData(index).toString().toStdString());
  });
  groupRow->addWidget(m_MatrixGroupPicker, 1);
  m_MatrixClearButton = new QToolButton(bar);
  m_MatrixClearButton->setText(tr("Clear"));
  m_MatrixClearButton->setToolTip(tr("Unlink the selected cells"));
  connect(m_MatrixClearButton, &QToolButton::clicked, this,
          [this]() { this->ApplyGroupToMatrixSelection({}); });
  groupRow->addWidget(m_MatrixClearButton);
  barLayout->addLayout(groupRow);

  // One row holding every offset editor; UpdateMatrixActionBar shows the pair
  // that fits the selected dimension and hides the row entirely when none does.
  m_MatrixOffsetRow = new QWidget(bar);
  auto* offsetLayout = new QHBoxLayout(m_MatrixOffsetRow);
  offsetLayout->setContentsMargins(0, 0, 0, 0);

  m_MatrixOffsetLabel = new QLabel(tr("Offset"), m_MatrixOffsetRow);
  offsetLayout->addWidget(m_MatrixOffsetLabel);

  // Each editor's range reaches one step past its domain, and that extra value
  // is the marker for "the selected cells disagree" (see MixedSliceOffset).
  // Writing the marker is refused, so a mixed selection is never flattened onto
  // one cell's value by accident.
  m_SliceOffsetEdit = new QSpinBox(m_MatrixOffsetRow);
  m_SliceOffsetEdit->setObjectName(QStringLiteral("mxnMatrixSliceOffset"));
  m_SliceOffsetEdit->setRange(MixedSliceOffset, 9999);
  m_SliceOffsetEdit->setSpecialValueText(tr("multiple"));
  m_SliceOffsetEdit->setToolTip(tr("Slice offset in shown slices, relative to the group's seed"));
  connect(m_SliceOffsetEdit, &QAbstractSpinBox::editingFinished, this, [this]()
  {
    // Detaching the editor disables it, which moves focus out of whichever spin
    // box holds it and so arrives here with nothing left to write to.
    if (m_MultiWidget.isNull() || MixedSliceOffset == m_SliceOffsetEdit->value())
    {
      return;
    }
    for (const auto& [windowId, axis] : this->MatrixSelection())
    {
      this->WriteCellDimensionOffset(windowId, QmitkMxNSyncDimension::Slice,
                                     m_SliceOffsetEdit->value());
    }
    m_MultiWidget->RefreshSyncControls();
  });
  offsetLayout->addWidget(m_SliceOffsetEdit);

  m_ZoomOffsetEdit = new QDoubleSpinBox(m_MatrixOffsetRow);
  m_ZoomOffsetEdit->setObjectName(QStringLiteral("mxnMatrixZoomOffset"));
  m_ZoomOffsetEdit->setRange(MixedZoomOffset, 100.0);
  m_ZoomOffsetEdit->setSingleStep(0.1);
  m_ZoomOffsetEdit->setSpecialValueText(tr("multiple"));
  m_ZoomOffsetEdit->setToolTip(tr("Zoom factor relative to the group's seed"));
  connect(m_ZoomOffsetEdit, &QAbstractSpinBox::editingFinished, this, [this]()
  {
    if (m_MultiWidget.isNull() || m_ZoomOffsetEdit->value() <= MixedZoomOffset)
    {
      return;
    }
    for (const auto& [windowId, axis] : this->MatrixSelection())
    {
      this->WriteCellDimensionOffset(windowId, QmitkMxNSyncDimension::Zoom,
                                     m_ZoomOffsetEdit->value());
    }
    m_MultiWidget->RefreshSyncControls();
  });
  offsetLayout->addWidget(m_ZoomOffsetEdit);

  m_PanOffsetLabel = new QLabel(tr("mm"), m_MatrixOffsetRow);
  m_PanOffsetXEdit = new QDoubleSpinBox(m_MatrixOffsetRow);
  m_PanOffsetXEdit->setObjectName(QStringLiteral("mxnMatrixPanOffsetX"));
  m_PanOffsetYEdit = new QDoubleSpinBox(m_MatrixOffsetRow);
  for (auto* box : { m_PanOffsetXEdit, m_PanOffsetYEdit })
  {
    box->setRange(MixedPanOffset, 1.0e5);
    box->setSpecialValueText(tr("multiple"));
    box->setToolTip(tr("Pan offset in world mm, relative to the group's seed"));
    connect(box, &QAbstractSpinBox::editingFinished, this, [this]()
    {
      if (m_MultiWidget.isNull() || m_PanOffsetXEdit->value() <= MixedPanOffset
          || m_PanOffsetYEdit->value() <= MixedPanOffset)
      {
        return;
      }
      mitk::Vector2D pan;
      pan[0] = m_PanOffsetXEdit->value();
      pan[1] = m_PanOffsetYEdit->value();
      for (const auto& [windowId, axis] : this->MatrixSelection())
      {
        this->WriteCellDimensionOffset(windowId, QmitkMxNSyncDimension::Pan, pan);
      }
      m_MultiWidget->RefreshSyncControls();
    });
  }
  offsetLayout->addWidget(m_PanOffsetXEdit);
  offsetLayout->addWidget(m_PanOffsetLabel);
  offsetLayout->addWidget(m_PanOffsetYEdit);

  m_RampLabel = new QLabel(tr("Ramp from"), m_MatrixOffsetRow);
  m_RampFromEdit = new QSpinBox(m_MatrixOffsetRow);
  m_RampFromEdit->setRange(-9999, 9999);
  m_RampFromEdit->setValue(-1);
  m_RampStepLabel = new QLabel(tr("by"), m_MatrixOffsetRow);
  m_RampStepEdit = new QSpinBox(m_MatrixOffsetRow);
  m_RampStepEdit->setRange(-9999, 9999);
  m_RampStepEdit->setValue(1);
  m_RampApplyButton = new QToolButton(m_MatrixOffsetRow);
  m_RampApplyButton->setObjectName(QStringLiteral("mxnMatrixRampApply"));
  m_RampApplyButton->setText(tr("Set"));
  m_RampApplyButton->setToolTip(tr("Spread slice offsets over the selected cells, in the order "
                                   "the windows are listed above. This is how a -1 / 0 / +1 "
                                   "movie frame is authored."));
  connect(m_RampApplyButton, &QToolButton::clicked, this, [this]()
  {
    QStringList windowIds;
    for (const auto& [windowId, axis] : this->MatrixSelection())
    {
      windowIds.append(windowId);
    }
    this->ApplySliceOffsetRamp(windowIds, m_RampFromEdit->value(), m_RampStepEdit->value());
  });
  // The offsets the ramp would write, in the order it writes them, so its effect
  // is readable against the window list above before it is applied.
  m_RampPreviewLabel = new QLabel(m_MatrixOffsetRow);
  m_RampPreviewLabel->setObjectName(QStringLiteral("mxnMatrixRampPreview"));
  for (auto* box : { m_RampFromEdit, m_RampStepEdit })
  {
    connect(box, &QSpinBox::valueChanged, this, [this](int) { this->UpdateRampPreview(); });
  }
  offsetLayout->addWidget(m_RampLabel);
  offsetLayout->addWidget(m_RampFromEdit);
  offsetLayout->addWidget(m_RampStepLabel);
  offsetLayout->addWidget(m_RampStepEdit);
  offsetLayout->addWidget(m_RampApplyButton);
  offsetLayout->addWidget(m_RampPreviewLabel);
  offsetLayout->addStretch();

  barLayout->addWidget(m_MatrixOffsetRow);

  this->UpdateMatrixActionBar();
  return bar;
}

std::vector<std::pair<QString, QmitkMxNSyncAxis>> QmitkMxNLayoutEditorWidget::MatrixSelection() const
{
  std::vector<std::pair<QString, QmitkMxNSyncAxis>> selection;
  if (nullptr == m_Matrix || nullptr == m_Matrix->selectionModel())
  {
    return selection;
  }

  auto indexes = m_Matrix->selectionModel()->selectedIndexes();
  // Row then column: row order is the layout's pre-order, which is the order a
  // ramp spreads over and the order the description reads windows in.
  std::sort(indexes.begin(), indexes.end(), [](const QModelIndex& a, const QModelIndex& b)
  {
    return a.row() != b.row() ? a.row() < b.row() : a.column() < b.column();
  });
  for (const auto& index : indexes)
  {
    const auto row = static_cast<std::size_t>(index.row());
    const auto axis = QmitkMxNSyncAxisFromSlot(index.column());
    if (index.row() >= 0 && row < m_MatrixCellIds.size() && axis.has_value())
    {
      selection.emplace_back(m_MatrixCellIds[row], *axis);
    }
  }
  return selection;
}

void QmitkMxNLayoutEditorWidget::UpdateMatrixActionBar()
{
  if (nullptr == m_MatrixSelectionLabel)
  {
    return;
  }

  const auto selection = this->MatrixSelection();

  if (selection.empty() || m_MultiWidget.isNull())
  {
    m_MatrixSelectionLabel->setText(
      tr("Select cells to link them - drag across the grid, or click an axis or window header "
         "for a whole line. This is where a partial ('some windows') group state and the "
         "per-window offsets are authored."));
    m_MatrixAxisIconLabel->setVisible(false);
    m_MatrixGroupPicker->clear();
    m_MatrixGroupPicker->setEnabled(false);
    m_MatrixClearButton->setEnabled(false);
    m_MatrixOffsetRow->setVisible(false);
    return;
  }

  // Which axes and which windows the selection spans, for the description and
  // for deciding whether a single dimension's offset editors apply.
  std::vector<QmitkMxNSyncAxis> axes;
  QStringList windowLabels;
  for (const auto& [windowId, axis] : selection)
  {
    if (std::find(axes.begin(), axes.end(), axis) == axes.end())
    {
      axes.push_back(axis);
    }
    // The row header already carries the label the plates use for the window.
    const auto row = std::find(m_MatrixCellIds.begin(), m_MatrixCellIds.end(), windowId);
    const auto* header = row == m_MatrixCellIds.end()
                           ? nullptr
                           : m_Matrix->verticalHeaderItem(
                               static_cast<int>(std::distance(m_MatrixCellIds.begin(), row)));
    const auto label = nullptr != header ? header->text() : windowId;
    if (!windowLabels.contains(label))
    {
      windowLabels.append(label);
    }
  }

  QString axisPart;
  QmitkMxNAxisGlyph axisGlyph = QmitkMxNAxisGlyph::Selection;
  const bool singleAxis = 1 == axes.size();
  if (singleAxis)
  {
    if (const auto axisDimension = QmitkMxNSyncAxisDimension(axes.front()))
    {
      axisPart = QString::fromUtf8(DimensionLabel(*axisDimension));
      axisGlyph = GlyphFor(*axisDimension);
    }
    else
    {
      axisPart = tr("Data selection");
      axisGlyph = QmitkMxNAxisGlyph::Selection;
    }
  }
  else
  {
    axisPart = tr("%n dimension(s)", nullptr, static_cast<int>(axes.size()));
  }

  // Name the axis in the bar with the same glyph the column header and the plates
  // carry, so the bar is visibly about the column the user is working in. Only
  // for a single axis: a glyph for "3 dimensions" would name none of them.
  const QPixmap axisIcon =
    singleAxis ? QmitkMxNRenderAxisGlyph(axisGlyph, this->palette().color(QPalette::Text),
                                         m_MatrixAxisIconLabel->fontMetrics().height())
               : QPixmap();
  m_MatrixAxisIconLabel->setPixmap(axisIcon);
  m_MatrixAxisIconLabel->setVisible(!axisIcon.isNull());

  constexpr int maxListedWindows = 4;
  QString windowPart = windowLabels.mid(0, maxListedWindows).join(QStringLiteral(", "));
  if (windowLabels.size() > maxListedWindows)
  {
    windowPart += tr(", and %n more", nullptr,
                     static_cast<int>(windowLabels.size()) - maxListedWindows);
  }
  m_MatrixSelectionLabel->setText(tr("%n cell(s)", nullptr, static_cast<int>(selection.size()))
                                  + QStringLiteral(" - ") + axisPart
                                  + QStringLiteral(" - ") + windowPart);

  // The group every selected cell already shares, if they share one; otherwise
  // the picker rests on nothing so it cannot read as a false common value.
  bool common = true;
  std::string commonGroup;
  bool allLinked = true;
  for (std::size_t i = 0; i < selection.size(); ++i)
  {
    const auto content = this->AdvancedMatrixCell(selection[i].first, selection[i].second);
    if (content.group.empty())
    {
      allLinked = false;
    }
    if (0 == i)
    {
      commonGroup = content.group;
    }
    else if (content.group != commonGroup)
    {
      common = false;
    }
  }

  m_MatrixGroupPicker->setEnabled(true);
  m_MatrixClearButton->setEnabled(true);
  m_MatrixGroupPicker->clear();
  m_MatrixGroupPicker->addItem(NotLinkedEntry, QString());
  try
  {
    for (const auto& info : m_MultiWidget->GetSyncGroupInfos())
    {
      m_MatrixGroupPicker->addItem(SwatchFor(info.color),
                                   QString::fromStdString(info.displayName),
                                   QString::fromStdString(info.id));
    }
  }
  catch (const mitk::Exception&)
  {
  }
  const int currentIndex =
    common && !commonGroup.empty()
      ? m_MatrixGroupPicker->findData(QString::fromStdString(commonGroup))
      : (common ? 0 : -1);
  m_MatrixGroupPicker->setCurrentIndex(currentIndex);

  // Offsets are per dimension and relative to a group seed, so they apply only
  // to a selection wholly on one offset-bearing dimension whose cells are all
  // linked - an offset on an unlinked cell would have nothing to be relative to.
  std::optional<QmitkMxNSyncDimension> dimension;
  if (1 == axes.size())
  {
    dimension = QmitkMxNSyncAxisDimension(axes.front());
  }
  const bool slice = QmitkMxNSyncDimension::Slice == dimension;
  const bool zoom = QmitkMxNSyncDimension::Zoom == dimension;
  const bool pan = QmitkMxNSyncDimension::Pan == dimension;

  m_MatrixOffsetRow->setVisible(allLinked && (slice || zoom || pan));
  m_MatrixOffsetLabel->setVisible(slice || zoom || pan);
  m_SliceOffsetEdit->setVisible(slice);
  m_ZoomOffsetEdit->setVisible(zoom);
  m_PanOffsetXEdit->setVisible(pan);
  m_PanOffsetYEdit->setVisible(pan);
  m_PanOffsetLabel->setVisible(pan);
  // Slice only: zoom composes multiplicatively and a pan ramp has no
  // unambiguous direction in two dimensions. One group only: a ramp lays out
  // positions within a single series, and offsets in different groups are
  // measured from different seeds, so spreading across them means nothing.
  const bool ramp = slice && selection.size() > 1 && common && !commonGroup.empty();
  m_RampLabel->setVisible(ramp);
  m_RampFromEdit->setVisible(ramp);
  m_RampStepLabel->setVisible(ramp);
  m_RampStepEdit->setVisible(ramp);
  m_RampApplyButton->setVisible(ramp);
  m_RampPreviewLabel->setVisible(ramp);
  this->UpdateRampPreview();

  // Seed the editors from the selection: its shared value where the cells agree,
  // otherwise the "multiple" marker, so a mixed selection reads as mixed instead
  // of as whichever cell happened to come first.
  if (allLinked && dimension.has_value())
  {
    std::vector<QmitkMxNMultiWidget::SyncOffset> offsets;
    try
    {
      for (const auto& [windowId, axis] : selection)
      {
        if (const auto link = m_MultiWidget->GetSyncLink(windowId, *dimension))
        {
          offsets.push_back(link->offset);
        }
      }
    }
    catch (const mitk::Exception&)
    {
      offsets.clear();
    }

    const bool uniform =
      !offsets.empty()
      && std::all_of(offsets.begin(), offsets.end(),
                     [&offsets](const QmitkMxNMultiWidget::SyncOffset& offset)
                     { return offset == offsets.front(); });

    if (slice)
    {
      QSignalBlocker blocker(m_SliceOffsetEdit);
      m_SliceOffsetEdit->setValue(uniform && std::holds_alternative<int>(offsets.front())
                                    ? std::get<int>(offsets.front())
                                    : MixedSliceOffset);
    }
    else if (zoom)
    {
      QSignalBlocker blocker(m_ZoomOffsetEdit);
      m_ZoomOffsetEdit->setValue(uniform && std::holds_alternative<double>(offsets.front())
                                   ? std::get<double>(offsets.front())
                                   : MixedZoomOffset);
    }
    else if (pan)
    {
      QSignalBlocker blockX(m_PanOffsetXEdit);
      QSignalBlocker blockY(m_PanOffsetYEdit);
      const bool known = uniform && std::holds_alternative<mitk::Vector2D>(offsets.front());
      const auto offset = known ? std::get<mitk::Vector2D>(offsets.front()) : mitk::Vector2D();
      m_PanOffsetXEdit->setValue(known ? offset[0] : MixedPanOffset);
      m_PanOffsetYEdit->setValue(known ? offset[1] : MixedPanOffset);
    }
  }
}

void QmitkMxNLayoutEditorWidget::UpdateRampPreview()
{
  if (nullptr == m_RampPreviewLabel || !m_RampPreviewLabel->isVisibleTo(m_MatrixOffsetRow))
  {
    return;
  }

  // Long selections would push the action bar wider than the matrix; the first
  // few values already show the direction and the spacing.
  constexpr int maxPreviewed = 6;
  const auto selection = this->MatrixSelection();
  QStringList steps;
  for (int position = 0;
       position < static_cast<int>(selection.size()) && position < maxPreviewed; ++position)
  {
    steps.append(SliceOffsetLabel(m_RampFromEdit->value() + position * m_RampStepEdit->value()));
  }
  if (static_cast<int>(selection.size()) > maxPreviewed)
  {
    steps.append(QStringLiteral("..."));
  }
  m_RampPreviewLabel->setText(QStringLiteral("= ") + steps.join(QStringLiteral(", ")));
}

void QmitkMxNLayoutEditorWidget::ApplyGroupToMatrixSelection(const std::string& group)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  for (const auto& [windowId, axis] : this->MatrixSelection())
  {
    if (group.empty())
    {
      this->WriteCellAxisCleared(windowId, axis);
    }
    else
    {
      this->WriteCellAxisGroup(windowId, axis, group);
    }
  }
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::ShowMatrixGroupMenu(const QPoint& globalPos)
{
  if (m_MultiWidget.isNull() || this->MatrixSelection().empty())
  {
    return;
  }

  QMenu menu(this);
  try
  {
    for (const auto& info : m_MultiWidget->GetSyncGroupInfos())
    {
      const auto id = info.id;
      connect(menu.addAction(SwatchFor(info.color), QString::fromStdString(info.displayName)),
              &QAction::triggered, this, [this, id]() { this->ApplyGroupToMatrixSelection(id); });
    }
  }
  catch (const mitk::Exception&)
  {
    return;
  }
  menu.addSeparator();
  connect(menu.addAction(NotLinkedEntry), &QAction::triggered, this,
          [this]() { this->ApplyGroupToMatrixSelection({}); });
  menu.exec(globalPos);
}

void QmitkMxNLayoutEditorWidget::MirrorSelectionToMatrix(const QStringList& windowIds)
{
  if (m_MirroringSelection || nullptr == m_Matrix || nullptr == m_Matrix->selectionModel())
  {
    return;
  }

  // A window selection names windows, not axes, so it takes the whole row -
  // the matrix's reading of "these windows".
  QItemSelection selection;
  const int lastColumn = m_Matrix->columnCount() - 1;
  for (int row = 0; row < m_Matrix->rowCount() && lastColumn >= 0; ++row)
  {
    const auto id = static_cast<std::size_t>(row) < m_MatrixCellIds.size()
                      ? m_MatrixCellIds[static_cast<std::size_t>(row)]
                      : QString();
    if (windowIds.contains(id))
    {
      selection.select(m_Matrix->model()->index(row, 0),
                       m_Matrix->model()->index(row, lastColumn));
    }
  }

  m_MirroringSelection = true;
  m_Matrix->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
  m_MirroringSelection = false;
  this->UpdateMatrixActionBar();
}

void QmitkMxNLayoutEditorWidget::SetCellAxisGroup(const QString& windowId, QmitkMxNSyncAxis axis,
                                                  const std::string& group)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  this->WriteCellAxisGroup(windowId, axis, group);
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::WriteCellAxisGroup(const QString& windowId, QmitkMxNSyncAxis axis,
                                                    const std::string& group)
{
  if (m_MultiWidget.isNull() || group.empty())
  {
    return;
  }

  try
  {
    if (const auto dimension = QmitkMxNSyncAxisDimension(axis); !dimension.has_value())
    {
      m_MultiWidget->SetCellSelectionGroup(windowId, group);
    }
    else
    {
      // Carry any offset over: re-grouping a cell keeps the relationship the
      // user authored, it only changes what that relationship is measured from.
      const auto link = m_MultiWidget->GetSyncLink(windowId, *dimension);
      m_MultiWidget->SetSyncLink(windowId, *dimension, group,
                                 link.has_value() ? link->offset
                                                  : QmitkMxNMultiWidget::SyncOffset{});
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: link change for '" << windowId.toStdString()
              << "' ignored: " << e.GetDescription();
  }
}

void QmitkMxNLayoutEditorWidget::ClearCellAxis(const QString& windowId, QmitkMxNSyncAxis axis)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  this->WriteCellAxisCleared(windowId, axis);
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::WriteCellAxisCleared(const QString& windowId, QmitkMxNSyncAxis axis)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  try
  {
    if (const auto dimension = QmitkMxNSyncAxisDimension(axis); !dimension.has_value())
    {
      m_MultiWidget->ClearCellSelectionGroup(windowId);
    }
    else
    {
      m_MultiWidget->ClearSyncLink(windowId, *dimension);
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: unlink of '" << windowId.toStdString()
              << "' ignored: " << e.GetDescription();
  }
}

void QmitkMxNLayoutEditorWidget::SetCellDimensionOffset(
  const QString& windowId, QmitkMxNSyncDimension dimension,
  const QmitkMxNMultiWidget::SyncOffset& offset)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  this->WriteCellDimensionOffset(windowId, dimension, offset);
  m_MultiWidget->RefreshSyncControls();
}

void QmitkMxNLayoutEditorWidget::WriteCellDimensionOffset(
  const QString& windowId, QmitkMxNSyncDimension dimension,
  const QmitkMxNMultiWidget::SyncOffset& offset)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  try
  {
    const auto link = m_MultiWidget->GetSyncLink(windowId, dimension);
    if (!link.has_value())
    {
      return;
    }
    m_MultiWidget->SetSyncLink(windowId, dimension, link->group, offset);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: offset change for '" << windowId.toStdString()
              << "' ignored: " << e.GetDescription();
  }
}

void QmitkMxNLayoutEditorWidget::ApplySliceOffsetRamp(const QStringList& windowIds, int from,
                                                      int step)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  int position = 0;
  for (const auto& windowId : windowIds)
  {
    this->WriteCellDimensionOffset(windowId, QmitkMxNSyncDimension::Slice,
                                   from + position * step);
    ++position;
  }
  m_MultiWidget->RefreshSyncControls();
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
  std::string defaultGroup;
  try
  {
    infos = m_MultiWidget->GetSyncGroupInfos();
    descriptors = m_MultiWidget->ListWindowDescriptors();
    defaultGroup = m_MultiWidget->GetDefaultSyncGroupName();
  }
  catch (const mitk::Exception&)
  {
    // Mid-layout-change state: treat as trivial rather than warn spuriously.
    return false;
  }

  // Any group beyond the default one is a user-built structure a layout
  // replace would discard - including an empty group the user created but has not
  // populated yet.
  for (const auto& info : infos)
  {
    if (info.id != defaultGroup)
    {
      return true;
    }
  }

  // Otherwise the only group is the default one. The configuration is trivial
  // only when every cell rests as a clean Mono(default) - the fresh-cell
  // state, where windowing/LUT/selection are on the default group and nothing
  // else is linked. A cell that spans groups (Complex) or sits wholly on some
  // non-default group means the user linked synchronization the warning must
  // cover.
  QColor defaultHue;
  try
  {
    defaultHue = m_MultiWidget->GetSyncGroupColor(defaultGroup);
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
    if (defaultHue.isValid() && identity.hue != defaultHue)
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
