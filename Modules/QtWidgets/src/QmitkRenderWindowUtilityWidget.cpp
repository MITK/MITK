/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRenderWindowUtilityWidget.h>

#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
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
  , m_CleanViewButton(nullptr)
  , m_NavigatorToggleButton(nullptr)
  , m_SyncBarcode(nullptr)
{
  this->setParent(parent);

  // A quiet translucent backing so the strip reads as one panel floating over
  // the image rather than buttons pasted onto the canvas. The backing is drawn
  // in paintEvent: a stylesheet background-color is silently dropped on a plain
  // QWidget subclass (it would need Qt::WA_StyledBackground), and painting it
  // against a translucent widget is the recipe the sibling viewport furniture
  // already uses to composite correctly over the render window.
  this->setAttribute(Qt::WA_TranslucentBackground);

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

  // Data-selection group membership is no longer a per-cell combobox: it is one
  // axis among the others, assigned in the layout editor and shown in the sync
  // barcode. The cell's selection group is stored on the node selection widget
  // (see GetSyncGroup), which stays the authoritative store.

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

  // Per-axis sync barcode for this cell; filled by the multi widget. The strip
  // doubles as the entry point to the layout editor, replacing the former
  // "Sync" button.
  m_SyncBarcode = new QmitkMxNSyncBarcodeWidget(this);
  connect(m_SyncBarcode, &QmitkMxNSyncBarcodeWidget::Clicked, this, [this]() {
    emit LayoutEditorRequested();
  });
  layout->addWidget(m_SyncBarcode);
}

QmitkRenderWindowUtilityWidget::~QmitkRenderWindowUtilityWidget()
{
}

void QmitkRenderWindowUtilityWidget::paintEvent(QPaintEvent*)
{
  // The translucent rounded backing (see the constructor): a plain QWidget
  // subclass paints no background of its own, so drawing it here is what makes
  // the strip read as one panel over the image instead of loose controls.
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(0, 0, 0, 150));
  painter.drawRoundedRect(QRectF(this->rect()), 3.0, 3.0);
}

QmitkRenderWindowUtilityWidget::GroupSyncIndexType QmitkRenderWindowUtilityWidget::GetSyncGroup() const
{
  // The node selection widget is the authoritative store of the cell's data-
  // selection group (written by MxN::SetSynchronizationGroup); read it directly
  // now that the mirroring combobox is gone. Serialization
  // (MakeWindowDescriptor) and the sync barcode both read through here.
  return m_NodeSelectionWidget->GetSyncGroup();
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

void QmitkRenderWindowUtilityWidget::SetSyncBarcodeSlots(const QList<QmitkMxNSyncBarcodeWidget::AxisSlot>& axisSlots)
{
  m_SyncBarcode->SetSlots(axisSlots);
}
