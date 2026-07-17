/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRenderWindowUtilityWidget.h>

#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// mitk qt widgets
#include <QmitkRenderWindow.h>
#include <QmitkStyleManager.h>

QmitkRenderWindowUtilityWidget::QmitkRenderWindowUtilityWidget(
  QWidget* parent/* = nullptr */,
  QmitkRenderWindow* renderWindow/* = nullptr */,
  mitk::DataStorage* dataStorage/* = nullptr */)
  : m_NodeSelectionWidget(nullptr)
  , m_SyncGroupSelector(nullptr)
  , m_NewSyncGroupButton(nullptr)
  , m_CleanViewButton(nullptr)
  , m_NavigatorToggleButton(nullptr)
  , m_SyncBarcode(nullptr)
{
  this->setParent(parent);

  // A quiet translucent backing so the strip reads as one panel floating over
  // the image rather than buttons pasted onto the canvas (matching the
  // built-in render-window menu's dark backing).
  this->setAutoFillBackground(false);
  this->setStyleSheet(
    QStringLiteral("QmitkRenderWindowUtilityWidget { background-color: rgba(0, 0, 0, 150); "
                   "border-radius: 3px; }"));

  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(4, 2, 4, 2);

  mitk::NodePredicateAnd::Pointer noHelperObjects = mitk::NodePredicateAnd::New();
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  m_BaseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow());

  m_NodeSelectionWidget = new QmitkSynchronizedNodeSelectionWidget(parent);
  m_NodeSelectionWidget->SetBaseRenderer(m_BaseRenderer);
  m_NodeSelectionWidget->SetDataStorage(dataStorage);
  m_NodeSelectionWidget->SetNodePredicate(noHelperObjects);
  connect(this, &QmitkRenderWindowUtilityWidget::SetDataSelection, m_NodeSelectionWidget, &QmitkSynchronizedNodeSelectionWidget::SetSelection);
  // Mirror authoritative group changes (e.g. via 'MxN::SetSynchronizationGroup'
  // from the '+' button or layout load) back into the combobox.
  connect(m_NodeSelectionWidget, &QmitkSynchronizedNodeSelectionWidget::SyncGroupIndexChanged,
    this, &QmitkRenderWindowUtilityWidget::OnNodeSelectionWidgetSyncGroupChanged);

  // A plain tool button, not a QMenuBar entry: a menu bar collapses its
  // entries behind an extension popup once the utility row gets narrow,
  // which costs an extra click to reach the data selection.
  auto* dataButton = new QToolButton(this);
  dataButton->setText("Data");
  dataButton->setToolTip(tr("Select the data shown in this render window"));
  dataButton->setPopupMode(QToolButton::InstantPopup);
  auto* dataMenu = new QMenu(dataButton);
  auto* dataAction = new QWidgetAction(dataMenu);
  dataAction->setDefaultWidget(m_NodeSelectionWidget);
  dataMenu->addAction(dataAction);
  dataButton->setMenu(dataMenu);
  layout->addWidget(dataButton);

  auto* layoutEditorButton = new QToolButton(this);
  layoutEditorButton->setText("Sync");
  layoutEditorButton->setToolTip(tr("Open the MxN layout editor (groups, synchronization, layout)"));
  connect(layoutEditorButton, &QToolButton::clicked, this, [this]() {
    emit LayoutEditorRequested();
  });
  layout->addWidget(layoutEditorButton);

  m_SyncGroupSelector = new QComboBox(this);
  // The combobox starts empty and is populated reactively via 'OnSyncGroupAdded'.
  // Each row carries its group index as userData (QVariant) so sparse /
  // non-monotonic group indices map correctly. Row position is never used as
  // a proxy for the group number.
  m_SyncGroupSelector->setMinimumContentsLength(8);
  connect(m_SyncGroupSelector, &QComboBox::currentIndexChanged,
    this, &QmitkRenderWindowUtilityWidget::OnSyncGroupSelectionChanged);
  layout->addWidget(m_SyncGroupSelector);

  // The combobox is a passive view of existing groups. New-group creation goes
  // through a separate button so the combobox no longer drives lifecycle.
  m_NewSyncGroupButton = new QToolButton(this);
  m_NewSyncGroupButton->setText("+");
  m_NewSyncGroupButton->setToolTip(tr("Create a new synchronization group"));
  connect(m_NewSyncGroupButton, &QToolButton::clicked, this, [this]() {
    emit CreateNewSyncGroupRequested(m_NodeSelectionWidget);
  });
  layout->addWidget(m_NewSyncGroupButton);

  // The per-cell slice scrub control lives in the viewport navigator, and
  // reorientation is driven by clicking the cell's plane label; the utility
  // row no longer hosts a slice slider or a view-direction combobox. The
  // direction controller stays: it is how a reorientation (or a relayed
  // orientation-group change) is applied to this cell's renderer, via
  // 'SetViewDirectionSelection'.
  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer{ m_BaseRenderer };
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetDataStorage(dataStorage);

  m_CleanViewButton = new QToolButton(this);
  m_CleanViewButton->setText("Clean");
  m_CleanViewButton->setCheckable(true);
  m_CleanViewButton->setToolTip(tr("Clean view: hide all viewport furniture in every render window "
                                   "(readouts, ribbons), e.g. for screenshots"));
  connect(m_CleanViewButton, &QToolButton::toggled, this, [this](bool checked) {
    emit CleanViewToggled(checked);
  });
  layout->addWidget(m_CleanViewButton);

  // Navigator mode is editor-wide (like clean-view): this per-cell toggle is
  // the affordance, the owning multi widget applies and mirrors it back.
  m_NavigatorToggleButton = new QToolButton(this);
  m_NavigatorToggleButton->setText("Nav");
  m_NavigatorToggleButton->setCheckable(true);
  m_NavigatorToggleButton->setToolTip(tr("Expanded navigator: show the full 3D crosshair and "
                                         "coordinate entry in every cell instead of the compact "
                                         "slice slider"));
  connect(m_NavigatorToggleButton, &QToolButton::toggled, this, [this](bool checked) {
    emit NavigatorToggled(checked);
  });
  layout->addWidget(m_NavigatorToggleButton);

  // Per-dimension sync barcode for this cell; filled by the multi widget.
  m_SyncBarcode = new QmitkMxNSyncBarcodeWidget(this);
  layout->addWidget(m_SyncBarcode);
}

QmitkRenderWindowUtilityWidget::~QmitkRenderWindowUtilityWidget()
{
}

void QmitkRenderWindowUtilityWidget::SetSyncGroup(const GroupSyncIndexType index)
{
  if (index < 1)
  {
    mitkThrow() << "Invalid synchronization group index '" << index
                << "'. Group index must be >= 1.";
  }
  // Locate the combobox row that carries this group index in its userData.
  // The group must already be registered with this widget (via a prior
  // 'OnSyncGroupAdded'); silently de-selecting on a missing index would mask
  // a contract violation by the caller.
  const int row = m_SyncGroupSelector->findData(QVariant(index));
  if (row < 0)
  {
    mitkThrow() << "Synchronization group '" << index
                << "' is not registered with this widget. Ensure the owning "
                   "multi widget has emitted 'SyncGroupAdded(" << index
                << ")' before calling 'SetSyncGroup'.";
  }
  m_SyncGroupSelector->setCurrentIndex(row);
}

QmitkRenderWindowUtilityWidget::GroupSyncIndexType QmitkRenderWindowUtilityWidget::GetSyncGroup() const
{
  // Read the group index from the selected row's userData rather than from the
  // row position (which would conflate combobox layout with the group's logical
  // identifier when groups are sparse).
  const QVariant data = m_SyncGroupSelector->currentData();
  return data.isValid() ? data.toInt() : -1;
}

void QmitkRenderWindowUtilityWidget::OnSyncGroupSelectionChanged(int index)
{
  // Pure follower: report the selection. Group creation goes through the
  // '+' button (CreateNewSyncGroupRequested), not through the combobox.
  if (index < 0)
  {
    return;
  }
  const QVariant data = m_SyncGroupSelector->itemData(index);
  if (!data.isValid())
  {
    return;
  }
  emit SyncGroupChanged(m_NodeSelectionWidget, data.toInt());
}

void QmitkRenderWindowUtilityWidget::OnNodeSelectionWidgetSyncGroupChanged(int index)
{
  // The node selection widget is the authoritative source of the cell's group
  // assignment; mirror it into the combobox. The signals emitted by the
  // combobox change are blocked to terminate the round-trip
  // (combobox -> SyncGroupChanged -> MxN -> SetSyncGroup -> here -> combobox).
  // We do not throw on a missing row: this slot is a notification follower and
  // must not raise into the Qt event loop. A missing row only happens when
  // 'SyncGroupAdded' has not been delivered yet; the corresponding
  // 'OnSyncGroupAdded' will land shortly and the next 'SetSyncGroup' will
  // settle the combobox.
  const int row = m_SyncGroupSelector->findData(QVariant(index));
  if (row < 0)
  {
    return;
  }
  const QSignalBlocker blocker(m_SyncGroupSelector);
  m_SyncGroupSelector->setCurrentIndex(row);
}

void QmitkRenderWindowUtilityWidget::SetViewDirectionSelection(mitk::AnatomicalPlane viewDirection)
{
  // Apply the plane to this cell's renderer. Both entry points - the source
  // cell's plane-label picker (via MxN::SetViewDirection) and an
  // orientation-group relay (via MxN::PropagateOrientation) - funnel through
  // here, so it stays the single application path.
  if (mitk::AnatomicalPlane::Axial != viewDirection
      && mitk::AnatomicalPlane::Coronal != viewDirection
      && mitk::AnatomicalPlane::Sagittal != viewDirection)
  {
    return;  // 'Original' has no propagation semantics
  }
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection, m_BaseRenderer);
}

QmitkSynchronizedNodeSelectionWidget* QmitkRenderWindowUtilityWidget::GetNodeSelectionWidget() const
{
  return m_NodeSelectionWidget;
}

void QmitkRenderWindowUtilityWidget::SetCleanViewChecked(bool checked)
{
  // Follower path: the editor-wide state is authoritative; blocking the
  // signal terminates the toggle -> editor -> mirror round-trip.
  const QSignalBlocker blocker(m_CleanViewButton);
  m_CleanViewButton->setChecked(checked);
}

void QmitkRenderWindowUtilityWidget::SetNavigatorChecked(bool expanded)
{
  const QSignalBlocker blocker(m_NavigatorToggleButton);
  m_NavigatorToggleButton->setChecked(expanded);
}

void QmitkRenderWindowUtilityWidget::SetSyncBarcodeSlots(const QList<QColor>& slotColors)
{
  m_SyncBarcode->SetSlots(slotColors);
}

void QmitkRenderWindowUtilityWidget::OnSyncGroupAdded(const GroupSyncIndexType index, const QString& label)
{
  // Reactive growth: append a row carrying this group index as userData. We
  // de-dupe by data (not by position) so sparse / non-monotonic group indices
  // map correctly.
  if (m_SyncGroupSelector->findData(QVariant(index)) >= 0)
  {
    return;
  }
  // The combobox is a passive view; populating it must never (re)assign the
  // cell's group. The first addItem() on an empty combobox moves currentIndex
  // from -1 to 0 and fires currentIndexChanged, which would bind this cell to
  // the lowest-numbered group mid-construction - violating the explicit-id
  // contract that a freshly created cell stays unattached until its caller
  // places it. Block the combobox's signals so the authoritative assignment
  // via SetSynchronizationGroup remains the only path that changes the group.
  const QSignalBlocker blocker(m_SyncGroupSelector);
  m_SyncGroupSelector->addItem(label, QVariant(index));
}

void QmitkRenderWindowUtilityWidget::OnSyncGroupLabelChanged(const GroupSyncIndexType index, const QString& label)
{
  const int row = m_SyncGroupSelector->findData(QVariant(index));
  if (row < 0)
  {
    return;
  }
  m_SyncGroupSelector->setItemText(row, label);
}
