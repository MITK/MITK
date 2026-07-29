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

#include <mitkExceptionMacro.h>
#include <mitkLog.h>

#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QMouseEvent>
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

  constexpr std::array<QmitkMxNSyncDimension, 4> NavigationBundle{
    QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair
  };

  const QString NotLinkedEntry = QStringLiteral("(not linked)");

  void ClearLayout(QLayout* layout)
  {
    while (auto* item = layout->takeAt(0))
    {
      delete item->widget();
      delete item;
    }
  }

  /**
   * Group card frame that accepts cell drops from the map (no Q_OBJECT
   * machinery needed - it only forwards to a callback).
   */
  class GroupCardFrame : public QFrame
  {
  public:
    explicit GroupCardFrame(std::function<void(const QStringList&)> onCellsDropped,
                            QWidget* parent = nullptr)
      : QFrame(parent)
      , m_OnCellsDropped(std::move(onCellsDropped))
    {
      this->setAcceptDrops(true);
    }

  protected:
    void dragEnterEvent(QDragEnterEvent* event) override
    {
      if (event->mimeData()->hasFormat(QmitkMxNCellMapWidget::CellsMimeType))
      {
        event->acceptProposedAction();
      }
    }

    void dropEvent(QDropEvent* event) override
    {
      const auto ids = QString::fromUtf8(
        event->mimeData()->data(QmitkMxNCellMapWidget::CellsMimeType));
      m_OnCellsDropped(ids.split(QStringLiteral("\n"), Qt::SkipEmptyParts));
      event->acceptProposedAction();
    }

  private:
    std::function<void(const QStringList&)> m_OnCellsDropped;
  };

  /**
   * Color swatch that doubles as the drag source for "drop this group onto a
   * tile"; a plain press-and-release still emits clicked() for the color
   * dialog.
   */
  class GroupSwatchButton : public QPushButton
  {
  public:
    GroupSwatchButton(QString groupId, QWidget* parent = nullptr)
      : QPushButton(parent)
      , m_GroupId(std::move(groupId))
    {
    }

  protected:
    void mousePressEvent(QMouseEvent* event) override
    {
      m_PressPosition = event->pos();
      QPushButton::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
      if (event->buttons().testFlag(Qt::LeftButton)
          && (event->pos() - m_PressPosition).manhattanLength() >= QApplication::startDragDistance())
      {
        this->setDown(false);
        auto* mimeData = new QMimeData();
        mimeData->setData(QmitkMxNCellMapWidget::GroupMimeType, m_GroupId.toUtf8());
        auto* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
        return;
      }
      QPushButton::mouseMoveEvent(event);
    }

  private:
    QString m_GroupId;
    QPoint m_PressPosition;
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
  m_CellMap->setToolTip(tr("Select render windows by click, Ctrl-click, or rubber band; "
                           "assign them by dropping them onto a group (or a group's color "
                           "onto a window). The bottom stripe of each window is its sync "
                           "barcode: one slot per dimension (pan, zoom, slice, crosshair, "
                           "orientation, windowing, LUT), colored by group, gap = unsynced, "
                           "notch = offset."));
  connect(m_CellMap, &QmitkMxNCellMapWidget::AssignRequested, this,
          [this](const QString& group, const QStringList& windowIds)
          {
            this->AssignCellsToGroup(windowIds, group.toStdString());
          });
  connect(m_CellMap, &QmitkMxNCellMapWidget::SelectionChanged, this,
          [this](const QStringList&) { this->ScheduleRebuild(); });
  mainFaceLayout->addWidget(m_CellMap);

  auto* groupsContainer = new QWidget(mainContainer);
  m_GroupsLayout = new QVBoxLayout(groupsContainer);
  m_GroupsLayout->setContentsMargins(0, 0, 0, 0);
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

  if (!m_MultiWidget.isNull())
  {
    connect(m_MultiWidget, &QmitkMxNMultiWidget::SyncLinksChanged,
            this, &QmitkMxNLayoutEditorWidget::ScheduleRebuild);
    connect(m_MultiWidget, &QmitkMxNMultiWidget::LayoutChanged,
            this, &QmitkMxNLayoutEditorWidget::ScheduleRebuild);
    connect(m_MultiWidget, &QmitkMxNMultiWidget::SyncGroupAdded,
            this, [this]() { this->ScheduleRebuild(); });
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
                                                    const std::string& group)
{
  for (const auto& windowId : windowIds)
  {
    this->SetCellMembership(windowId, group, true);
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

bool QmitkMxNLayoutEditorWidget::GroupSelectionEnabled(const std::string& group) const
{
  if (m_MultiWidget.isNull())
  {
    return false;
  }
  for (const auto& windowId : this->GroupMembers(group))
  {
    if (m_MultiWidget->GetCellSelectionGroup(windowId) == group)
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

  try
  {
    if (member)
    {
      auto dimensions = this->GroupDimensions(group);
      if (dimensions.empty())
      {
        // Clever default: joining an empty group means "navigate together".
        dimensions.assign(NavigationBundle.begin(), NavigationBundle.end());
      }
      for (const auto dimension : dimensions)
      {
        m_MultiWidget->SetSyncLink(windowId, dimension, group);
      }
      // Selection is an axis too: a cell joining a group that already shares
      // data selection joins that selection group as well.
      if (this->GroupSelectionEnabled(group))
      {
        m_MultiWidget->SetCellSelectionGroup(windowId, group);
      }
    }
    else
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
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Layout editor: membership change for '" << windowId.toStdString()
              << "' ignored: " << e.GetDescription();
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

  // Deferred: engine signals arrive synchronously from mutations triggered
  // by this widget's own controls; rebuilding immediately would delete the
  // emitting control out from under its slot.
  m_RebuildPending = true;
  QTimer::singleShot(0, this, [this]()
  {
    m_RebuildPending = false;
    this->Rebuild();
  });
}

void QmitkMxNLayoutEditorWidget::Rebuild()
{
  ClearLayout(m_GroupsLayout);
  m_Matrix->clear();
  m_Matrix->setRowCount(0);
  m_Matrix->setColumnCount(0);

  if (m_MultiWidget.isNull())
  {
    m_CellMap->Rebuild();
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
  }
  m_GroupsLayout->addStretch();

  this->RebuildMatrix(infos, descriptors);
}

QWidget* QmitkMxNLayoutEditorWidget::BuildGroupCard(const QmitkMxNMultiWidget::SyncGroupInfo& info)
{
  const auto groupId = info.id;

  auto* card = new GroupCardFrame(
    [this, groupId](const QStringList& windowIds)
    {
      this->AssignCellsToGroup(windowIds, groupId);
    },
    this);
  card->setFrameShape(QFrame::StyledPanel);
  card->setToolTip(tr("Drop render windows from the map here to add them to this group"));
  auto* cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(6, 6, 6, 6);

  // Identity row: hue swatch (draggable onto map tiles) + editable display
  // name + member count + assign-selection action.
  auto* identityRow = new QHBoxLayout();
  auto* colorButton = new GroupSwatchButton(QString::fromStdString(groupId), card);
  colorButton->setFixedSize(20, 20);
  colorButton->setStyleSheet(QStringLiteral("background-color: %1;").arg(info.color.name()));
  colorButton->setToolTip(tr("Group color (persisted with the layout). Click to change; "
                             "drag onto a render window in the map to assign the group."));
  connect(colorButton, &QPushButton::clicked, this, [this, groupId]()
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
  identityRow->addWidget(colorButton);

  auto* nameEdit = new QLineEdit(QString::fromStdString(info.displayName), card);
  nameEdit->setToolTip(tr("Display name shown on every surface; the group id stays '%1'")
                         .arg(QString::fromStdString(groupId)));
  connect(nameEdit, &QLineEdit::editingFinished, this, [this, groupId, nameEdit]()
  {
    if (m_MultiWidget.isNull())
    {
      return;
    }
    try
    {
      m_MultiWidget->SetSyncGroupDisplayName(groupId, nameEdit->text().trimmed().toStdString());
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Layout editor: display-name change ignored: " << e.GetDescription();
    }
  });
  identityRow->addWidget(nameEdit, 1);

  const auto members = this->GroupMembers(groupId);
  identityRow->addWidget(new QLabel(tr("%n window(s)", nullptr, static_cast<int>(members.size())), card));

  auto* assignButton = new QPushButton(tr("Assign selection"), card);
  assignButton->setToolTip(tr("Add the render windows selected in the map to this group"));
  assignButton->setEnabled(!m_CellMap->GetSelectedWindowIds().isEmpty());
  connect(assignButton, &QPushButton::clicked, this, [this, groupId]()
  {
    this->AssignCellsToGroup(m_CellMap->GetSelectedWindowIds(), groupId);
  });
  identityRow->addWidget(assignButton);

  auto* removeButton = new QPushButton(tr("Remove selection"), card);
  removeButton->setToolTip(tr("Remove the render windows selected in the map from this group"));
  removeButton->setEnabled(!m_CellMap->GetSelectedWindowIds().isEmpty());
  connect(removeButton, &QPushButton::clicked, this, [this, groupId]()
  {
    for (const auto& windowId : m_CellMap->GetSelectedWindowIds())
    {
      this->SetCellMembership(windowId, groupId, false);
    }
  });
  identityRow->addWidget(removeButton);
  cardLayout->addLayout(identityRow);

  // Dimension row
  auto* dimensionRow = new QHBoxLayout();
  dimensionRow->addWidget(new QLabel(tr("Synchronizes:"), card));
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    auto* box = new QCheckBox(DimensionLabel(dimension), card);
    const auto memberIt = info.members.find(dimension);
    box->setChecked(memberIt != info.members.end() && !memberIt->second.empty());
    connect(box, &QCheckBox::toggled, this, [this, groupId, dimension](bool checked)
    {
      this->ApplyDimensionToGroup(groupId, dimension, checked);
    });
    dimensionRow->addWidget(box);
  }
  // Data selection is one axis among the others (a distinct engine under the
  // hood, single-valued per cell): managed here uniformly, not via a separate
  // per-cell control.
  auto* selectionBox = new QCheckBox(tr("Data"), card);
  selectionBox->setChecked(this->GroupSelectionEnabled(groupId));
  connect(selectionBox, &QCheckBox::toggled, this, [this, groupId](bool checked)
  {
    this->ApplySelectionToGroup(groupId, checked);
  });
  dimensionRow->addWidget(selectionBox);
  dimensionRow->addStretch();
  cardLayout->addLayout(dimensionRow);

  // Action row
  auto* actionRow = new QHBoxLayout();
  auto* navButton = new QPushButton(tr("Link navigation"), card);
  navButton->setToolTip(tr("Link pan, zoom, slice, and crosshair for all member windows"));
  navButton->setEnabled(!members.empty());
  connect(navButton, &QPushButton::clicked, this, [this, groupId]()
  {
    this->LinkNavigationBundle(groupId);
  });
  actionRow->addWidget(navButton);

  auto* reconvergeButton = new QPushButton(tr("Re-converge"), card);
  reconvergeButton->setToolTip(tr("Re-establish reference + offset for the group's "
                                  "slice/zoom/pan members"));
  reconvergeButton->setEnabled(!members.empty());
  connect(reconvergeButton, &QPushButton::clicked, this, [this, groupId]()
  {
    this->ReconvergeGroup(groupId);
  });
  actionRow->addWidget(reconvergeButton);

  auto* reinitButton = new QPushButton(tr("Reinit geometry"), card);
  reinitButton->setToolTip(tr("Re-initialize the shared geometry of the group's "
                              "slice/orientation component"));
  reinitButton->setEnabled(!members.empty());
  connect(reinitButton, &QPushButton::clicked, this, [this, groupId]()
  {
    this->ReinitGroupGeometry(groupId);
  });
  actionRow->addWidget(reinitButton);
  actionRow->addStretch();
  cardLayout->addLayout(actionRow);

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

std::vector<QString> QmitkMxNLayoutEditorWidget::GroupMembers(const std::string& group) const
{
  std::vector<QString> members;
  if (m_MultiWidget.isNull())
  {
    return members;
  }

  for (const auto& descriptor : m_MultiWidget->ListWindowDescriptors())
  {
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      const auto link = m_MultiWidget->GetSyncLink(descriptor.id, dimension);
      if (link.has_value() && link->group == group)
      {
        members.push_back(descriptor.id);
        break;
      }
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
