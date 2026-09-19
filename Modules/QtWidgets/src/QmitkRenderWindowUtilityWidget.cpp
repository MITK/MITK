/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRenderWindowUtilityWidget.h>

#include <QFontMetrics>
#include <QIcon>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
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
  // One rhythm for the row: every control is flat over the strip's own backing,
  // and the icon buttons take the height of the text beside them - which is also
  // the barcode's band height - so the row reads as one object and follows the
  // user's font scaling instead of a fixed size that only suits one DPI.
  const int iconExtent = QFontMetrics(this->font()).height();

  // The style's own hover and checked states paint an opaque box, which reads as
  // a chip cut out of the translucent backing. These tint it instead, so the
  // image still shows through, and mark state with the theme's accent rather
  // than the style's blue. The backing is dark whatever the theme, so the tints
  // are white.
  this->setStyleSheet(QStringLiteral(
    "QToolButton {"
    "  background: transparent;"
    "  border: 1px solid transparent;"
    "  border-radius: 3px;"
    "  padding: 1px 3px;"
    "  color: palette(bright-text);"
    "}"
    "QToolButton:hover { background: rgba(255, 255, 255, 26); }"
    "QToolButton:checked { background: rgba(255, 255, 255, 38); border-color: %1; }"
    "QToolButton:focus { border-color: %1; }"
    "QToolButton::menu-indicator { image: none; }").arg(QmitkStyleManager::GetIconAccentColor()));

  auto* dataButton = new QToolButton(this);
  dataButton->setAutoRaise(true);
  dataButton->setText("Data");
  dataButton->setToolTip(tr("Select the data shown in this render window"));
  dataButton->setPopupMode(QToolButton::InstantPopup);
  auto* dataMenu = new QMenu(dataButton);
  auto* dataAction = new QWidgetAction(dataMenu);
  dataAction->setDefaultWidget(m_NodeSelectionWidget);
  dataMenu->addAction(dataAction);
  dataButton->setMenu(dataMenu);
  // The menu grabs the pointer, which reads to the cell as the pointer leaving
  // and would collapse the strip the user just clicked; report it so the owner
  // can hold the furniture open for as long as the popup lives.
  connect(dataMenu, &QMenu::aboutToShow, this, [this]() { emit PopupVisibilityChanged(true); });
  connect(dataMenu, &QMenu::aboutToHide, this, [this]() { emit PopupVisibilityChanged(false); });
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


  // Navigator mode is editor-wide (like clean-view): this per-cell toggle is
  // the affordance, the owning multi widget applies and mirrors it back.
  m_NavigatorToggleButton = new QToolButton(this);
  m_NavigatorToggleButton->setAutoRaise(true);
  m_NavigatorToggleButton->setIconSize(QSize(iconExtent, iconExtent));
  m_NavigatorToggleButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/mxn-navigator.svg")));
  m_NavigatorToggleButton->setCheckable(true);
  m_NavigatorToggleButton->setToolTip(tr("Expanded navigator: show the full 3D crosshair and "
                                         "coordinate entry in every cell instead of the compact "
                                         "slice slider"));
  connect(m_NavigatorToggleButton, &QToolButton::toggled, this, [this](bool checked) {
    emit NavigatorToggled(checked);
  });

  // Per-axis sync barcode for this cell; filled by the multi widget. The strip
  // doubles as the entry point to the layout editor, replacing the former
  // "Sync" button.
  m_SyncBarcode = new QmitkMxNSyncBarcodeWidget(this);
  connect(m_SyncBarcode, &QmitkMxNSyncBarcodeWidget::Clicked, this, [this]() {
    emit LayoutEditorRequested();
  });
  // The barcode takes the row's slack rather than a spacer: given the width it
  // switches from color slots to legible glyphs (ComputeLayout), and expanding
  // it is also what pushes the presentation group to the far edge.
  layout->addWidget(m_SyncBarcode, 1);

  // What the window presents rather than what it shows: the navigator and
  // crosshair overlays, then clean view and maximize. Crosshair and maximize
  // come from the built-in render-window menu this strip covers, and hold its
  // top-right corner so the gesture stays where the Standard Display taught
  // users to look.
  layout->addWidget(m_NavigatorToggleButton);

  m_CrosshairButton = new QToolButton(this);
  m_CrosshairButton->setAutoRaise(true);
  m_CrosshairButton->setIconSize(QSize(iconExtent, iconExtent));
  m_CrosshairButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/mxn-crosshair.svg")));
  m_CrosshairButton->setCheckable(true);
  m_CrosshairButton->setToolTip(tr("Show the crosshair in every render window"));
  connect(m_CrosshairButton, &QToolButton::toggled, this, [this](bool checked) {
    emit CrosshairToggled(checked);
  });
  layout->addWidget(m_CrosshairButton);

  m_CleanViewButton = new QToolButton(this);
  m_CleanViewButton->setAutoRaise(true);
  m_CleanViewButton->setIconSize(QSize(iconExtent, iconExtent));
  m_CleanViewButton->setCheckable(true);
  m_CleanViewButton->setToolTip(tr("Clean view: hide all viewport furniture in every render window "
                                   "(readouts, ribbons), e.g. for screenshots"));
  connect(m_CleanViewButton, &QToolButton::toggled, this, [this](bool checked) {
    this->UpdateCleanViewIcon();
    emit CleanViewToggled(checked);
  });
  this->UpdateCleanViewIcon();
  layout->addWidget(m_CleanViewButton);

  m_MaximizeButton = new QToolButton(this);
  m_MaximizeButton->setAutoRaise(true);
  m_MaximizeButton->setIconSize(QSize(iconExtent, iconExtent));
  m_MaximizeButton->setCheckable(true);
  m_MaximizeButton->setToolTip(tr("Give this window the whole editor area; toggle off to bring "
                                  "the other windows back"));
  connect(m_MaximizeButton, &QToolButton::toggled, this, [this](bool checked) {
    this->UpdateMaximizeIcon();
    emit MaximizeToggled(checked);
  });
  this->UpdateMaximizeIcon();
  layout->addWidget(m_MaximizeButton);
}

void QmitkRenderWindowUtilityWidget::UpdateMaximizeIcon()
{
  // The enter/leave pair the built-in menu used, so the icon states what the
  // next click does rather than what the window currently is.
  m_MaximizeButton->setIcon(QmitkStyleManager::ThemeIcon(
    m_MaximizeButton->isChecked() ? QStringLiteral(":/Qmitk/mxn-restore.svg")
                                  : QStringLiteral(":/Qmitk/mxn-maximize.svg")));
}

void QmitkRenderWindowUtilityWidget::UpdateCleanViewIcon()
{
  // Clean view states the outcome rather than the action: the furniture the
  // frame is about to lose, or the bare image left once it is gone.
  m_CleanViewButton->setIcon(QmitkStyleManager::ThemeIcon(
    m_CleanViewButton->isChecked() ? QStringLiteral(":/Qmitk/mxn-clean-on.svg")
                                   : QStringLiteral(":/Qmitk/mxn-clean-off.svg")));
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
  // Blocking the signal also suppresses the icon swap the toggle path does.
  this->UpdateCleanViewIcon();
}

void QmitkRenderWindowUtilityWidget::SetNavigatorChecked(bool expanded)
{
  const QSignalBlocker blocker(m_NavigatorToggleButton);
  m_NavigatorToggleButton->setChecked(expanded);
}

void QmitkRenderWindowUtilityWidget::SetCrosshairChecked(bool visible)
{
  const QSignalBlocker blocker(m_CrosshairButton);
  m_CrosshairButton->setChecked(visible);
}

void QmitkRenderWindowUtilityWidget::SetMaximizeChecked(bool maximized)
{
  const QSignalBlocker blocker(m_MaximizeButton);
  m_MaximizeButton->setChecked(maximized);
  // Blocking the signal also suppresses the icon swap the toggle path does.
  this->UpdateMaximizeIcon();
}

void QmitkRenderWindowUtilityWidget::SetSyncBarcodeSlots(const QList<QmitkMxNSyncBarcodeWidget::AxisSlot>& axisSlots)
{
  m_SyncBarcode->SetSlots(axisSlots);
}
