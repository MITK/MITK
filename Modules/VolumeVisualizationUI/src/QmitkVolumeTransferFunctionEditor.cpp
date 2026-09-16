/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeTransferFunctionEditor.h"

#include <mitkImage.h>
#include <mitkLevelWindow.h>
#include <mitkProperties.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionTransform.h>

#include <QmitkCombinedTransferFunctionCanvas.h>
#include <QmitkStyleManager.h>
#include <QmitkTransferFunctionWidget.h>
#include <QmitkVolumeThumbnailRenderer.h>

#include <ui_QmitkVolumeTransferFunctionEditorControls.h>

#include <ctkDoubleSlider.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <QComboBox>
#include <QColor>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QIcon>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRect>
#include <QSignalBlocker>
#include <QTimer>
#include <QToolButton>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <optional>

namespace
{
  /** Keys recording how the node's transfer function was arrived at: the preset
   * it started from, and the four offsets the adjust sliders then applied.
   *
   * Together they are a recipe this widget can re-execute, which is what lets
   * the sliders come back showing where they were left. The curve alone cannot
   * serve that purpose - the colour window is baked into 256 evenly spaced
   * points, and an offset is only meaningful next to the baseline it was
   * measured from. Same reasoning as
   * mitk::VolumeRenderingLightingModel::MODEL_PROPERTY: record the choice, do
   * not infer it.
   *
   * The offsets are meaningful only alongside the preset name, and all five are
   * absent on a curve no preset describes. TF_CUSTOM_PROPERTY is what then tells
   * a curve chosen here apart from one that came from outside this widget.
   */
  constexpr const char *TF_PRESET_PROPERTY = "volumerendering.transferfunction.preset";
  constexpr const char *TF_OPACITY_SHIFT_PROPERTY = "volumerendering.transferfunction.opacityshift";
  constexpr const char *TF_OPACITY_HEIGHT_PROPERTY = "volumerendering.transferfunction.opacityheight";
  constexpr const char *TF_COLOR_SHIFT_PROPERTY = "volumerendering.transferfunction.colorshift";
  constexpr const char *TF_COLOR_WIDTH_PROPERTY = "volumerendering.transferfunction.colorwidth";

  /** Records that the node's curve was authored here or loaded from a file.
   *
   * The keys above cannot say so: what makes such a curve custom is precisely
   * that no recipe describes it. What is recorded is therefore only that the
   * curve was chosen here at all, which is what AdoptTransferFunctionFromNode
   * needs to take it back over rather than leave it to be replaced. Written true
   * or removed, never false, so absence keeps the single meaning the keys above
   * already give it.
   */
  constexpr const char *TF_CUSTOM_PROPERTY = "volumerendering.transferfunction.custom";

  /** \brief A preview's shape.
   *
   * Held constant so that the render and the cell it is drawn into never
   * disagree about the aspect ratio, whatever width the panel allows.
   */
  constexpr double PREVIEW_ASPECT = 86.0 / 110.0;

  /** \brief How many previews stand side by side however narrow the panel.
   *
   * There is no panel width to size cells against: the workbench gives the
   * panel a fraction of the window, so it is far narrower on a laptop than on
   * a workstation, and any fixed cell width means a different number of
   * previews per row on each. Fixing the count instead, and deriving the cells
   * from it, is what makes the grid read the same on both.
   */
  constexpr int MIN_COLUMNS = 3;

  /** \brief The cell width worth having, once there is room for it.
   *
   * Not a minimum: MIN_COLUMNS wins on a narrow panel. This only decides when
   * a wide panel has earned another column rather than larger previews.
   */
  constexpr int PREFERRED_CELL_WIDTH = 118;

  /** \brief The margin between a preview and the edges of its cell. */
  constexpr int CELL_PADDING = 4;

  /** \brief How strongly the stand-in for a missing preview is drawn. */
  constexpr double PLACEHOLDER_OPACITY = 0.4;

  /** \brief Width left to the view to lay the cells out in.
   *
   * Determined by observation: the view fits one cell fewer than the viewport
   * would hold when the cells add up to all of it, and needs two pixels over
   * that to fit them all. A couple more than two is invisible and leaves room
   * for a style that reserves more.
   */
  constexpr int VIEWPORT_RESERVE = 4;

  /** \brief The width every preview is drawn at, once.
   *
   * Derived rather than chosen: a cell only grows to just under twice
   * PREFERRED_CELL_WIDTH before the panel is wide enough for another column,
   * so this is the widest a preview can ever be asked to appear at. Drawing at
   * that width means previews are only ever scaled down, and never have to be
   * drawn a second time because the panel was resized.
   */
  constexpr int PREVIEW_RENDER_WIDTH = 2 * PREFERRED_CELL_WIDTH - 2 * CELL_PADDING;

  /** \brief A preview's size given the width its cell allows. */
  QSize PreviewSize(int width)
  {
    return QSize(width, static_cast<int>(std::lround(width * PREVIEW_ASPECT)));
  }

  /** \brief A stand-in for a preview not drawn yet.
   *
   * Entries are sized to what they hold, so one that has only its name is
   * shorter than one with a preview. Filling them all in at the final size
   * from the start keeps the grid from being laid out twice - once for the
   * names, and again, entry by entry, as previews arrive.
   *
   * Drawn rather than left blank so that a cell without a preview reads as one
   * still to come. Nothing distinguishes "no image selected" from "previews are
   * being drawn" here, and nothing needs to: the view disables the list in the
   * first case, and Qt fades a disabled item's icon of its own accord.
   *
   * \param[in] size  The pixel size the cells reserve for a preview.
   * \param[in] color The theme's icon colour. Drawn at part opacity, since the
   *                  mark stands in for content rather than being content.
   */
  QIcon PlaceholderPreview(const QSize &size, const QColor &color)
  {
    QPixmap placeholder(size);
    placeholder.fill(Qt::transparent);

    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(PLACEHOLDER_OPACITY);
    painter.setPen(color);

    // Half a pixel in on every side: a one-pixel pen straddles the path it is
    // given, so a frame on the pixmap's own edge would lose its outer half.
    const QRectF frame(0.5, 0.5, size.width() - 1.0, size.height() - 1.0);

    painter.drawRoundedRect(frame, 3.0, 3.0);
    painter.drawLine(frame.topRight(), frame.bottomLeft());

    return QIcon(placeholder);
  }

  /** \brief Whether the node is rendered as a volume at all.
   *
   * Nothing outside the volume visualization controls writes the property, so it
   * doubles as the marker that someone deliberately configured this node there.
   * "TransferFunction" cannot serve that purpose: mitk::VolumeMapperVtkSmart3D
   * registers a default one on every image node, so its presence says nothing.
   */
  bool IsVolumeRenderingOn(const mitk::DataNode *node)
  {
    if (node == nullptr)
      return false;

    bool volumeRenderingOn = false;
    node->GetBoolProperty("volumerendering", volumeRenderingOn);

    return volumeRenderingOn;
  }

  /** \brief Whether the node's curve was authored here or loaded from a file.
   *
   * The other half of the evidence that this node was configured here, for the
   * curves the recorded preset name cannot cover. See TF_CUSTOM_PROPERTY.
   */
  bool IsCustomTransferFunction(const mitk::DataNode *node)
  {
    if (node == nullptr)
      return false;

    bool customTransferFunction = false;
    node->GetBoolProperty(TF_CUSTOM_PROPERTY, customTransferFunction);

    return customTransferFunction;
  }

  /** \brief The blend mode a node renders in, for callers that need a value
   *         even where mitk::GetVolumeBlendMode has none: a null node, or one
   *         naming a mode MITK does not offer. Both read as composite here.
   */
  mitk::VolumeBlendMode BlendModeOrComposite(const mitk::DataNode *node)
  {
    return mitk::GetVolumeBlendMode(node).value_or(mitk::VolumeBlendMode::Composite);
  }

  /** \brief The intensity band the editor is scaled to.
   *
   * Not the image's outermost values: one saturated voxel is enough to set the
   * axis the canvas draws and all four sliders are sized against, leaving the
   * curve a pixel wide and a single slider step wider than the curve itself.
   * mitk::LevelWindow::SetAuto answers with the band instead, substituting the
   * second extreme for one that too few voxels carry to matter, and is what the
   * 2D views show the same image over.
   *
   * \return The band, the histogram's own bounds where the image names none, or
   *         nothing where neither can.
   */
  std::optional<std::array<double, 2>> WorkingRange(const mitk::Image *image,
                                                    const mitk::SimpleHistogram *histogram)
  {
    if (image != nullptr && image->IsInitialized())
    {
      mitk::LevelWindow band;

      // Over the whole volume rather than its central slice, which an outlier
      // can miss. Only the third argument carries meaning here; SetAuto documents
      // the second as unused.
      band.SetAuto(image, true, false);

      double lower = band.GetLowerWindowBound();
      double upper = band.GetUpperWindowBound();

      // SetAuto pads the range before deciding what to discard from it, and on
      // an image holding two distinct values the padding is all that survives:
      // the band then reaches past every voxel in the image. Bounded by the
      // histogram, which is the axis this had before the band was consulted, so
      // the worst the band can do is fail to narrow it.
      if (histogram != nullptr)
      {
        lower = std::max(lower, histogram->GetMin());
        upper = std::min(upper, histogram->GetMax());
      }

      if (lower < upper)
        return std::array{ lower, upper };
    }

    if (histogram != nullptr)
      return std::array{ histogram->GetMin(), histogram->GetMax() };

    return std::nullopt;
  }

  /** The arrow is the only cue that a section folds away, so it is drawn by the
   * style from arrowType rather than taken from a pixmap.
   */
  void SetSectionExpanded(QToolButton *header, QWidget *panel, bool expanded)
  {
    header->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    panel->setVisible(expanded);
  }
}

QmitkVolumeTransferFunctionEditor::QmitkVolumeTransferFunctionEditor(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_Controls(std::make_unique<Ui::QmitkVolumeTransferFunctionEditor>()),
    // Drawn once at the width the widest cell could want, and scaled down to
    // whatever the current one allows, so that resizing the panel relays the
    // grid out rather than drawing every preview again. Built here rather than
    // in the body because InvalidateThumbnails releases the volume through it,
    // and the body calls that before it is done setting the widget up.
    m_ThumbnailRenderer(std::make_unique<QmitkVolumeThumbnailRenderer>(PreviewSize(PREVIEW_RENDER_WIDTH)))
{
  m_Controls->setupUi(this);

  m_Controls->tfControlPanelsWidget->ShowGradientOpacityFunction(false);

  auto *presetList = m_Controls->presetListWidget;

  presetList->setViewMode(QListView::IconMode);

  // The entries state the size they occupy themselves, so there is nothing
  // left here for the view to measure and hold on to. Leaving this off is what
  // has it read that size again after every re-measure, rather than the one it
  // happened to see first.
  presetList->setUniformItemSizes(false);

  // Without wrapping the entries stay one per row, which is the plain list the
  // previews are meant to replace.
  presetList->setWrapping(true);
  presetList->setResizeMode(QListView::Adjust);

  // Icon mode lets entries be dragged around by default. This is a menu.
  presetList->setMovement(QListView::Static);

  // Wrapped rather than cut short with an ellipsis, since hardly any preset
  // name fits a cell on one line.
  presetList->setWordWrap(true);
  presetList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  presetList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

  for (const auto &name : m_Presets.GetPresetNames())
    presetList->addItem(QString::fromStdString(name));

  // Cells are measured from the names they have to hold, so the entries come
  // first, and the stand-in previews after them, since their size is what the
  // measurement settles.
  //
  // The panel is only ever as wide as the workbench window makes it, so the
  // cells are measured from it rather than fixed, and measured again whenever
  // it changes. Watching the viewport rather than overriding this widget's own
  // resizeEvent: the layout has not necessarily handed the list its new
  // geometry by the time that event arrives, and a stale width would be
  // measured.
  this->UpdatePresetGrid();
  this->InvalidateThumbnails();

  presetList->viewport()->installEventFilter(this);

  // A freshly filled list lands on its first entry, which would name a preset
  // nothing has applied.
  this->ClearPresetSelection();

  m_Controls->opacityShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->opacityHeightSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorWidthSlider->setOrientation(Qt::Horizontal);

  m_Controls->advancedTfPanel->setVisible(false);

  // Identified by their stable ids rather than by row, so that reordering the
  // modes, or putting a separator between them the way the presets grid does,
  // cannot silently change what a row selects.
  for (const auto &description : mitk::VolumeBlendModeDescription::GetAll())
  {
    m_Controls->blendModeComboBox->addItem(
      QString::fromStdString(description.label), QString::fromStdString(description.id));
    m_Controls->blendModeComboBox->setItemData(
      m_Controls->blendModeComboBox->count() - 1,
      QString::fromStdString(description.description), Qt::ToolTipRole);
  }

  connect(m_Controls->blendModeComboBox, &QComboBox::currentIndexChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnBlendModeChanged);

  // A click rather than the current entry changing: the current entry is also
  // set from what a node records, and reacting to that would re-apply the
  // preset and discard the curve the node was carrying.
  connect(m_Controls->presetListWidget, &QListWidget::itemClicked, this,
    [this](QListWidgetItem *item)
    {
      if (item != nullptr)
        this->OnPresetSelected(item->text());
    });

  connect(m_Controls->presetExpandButton, &QToolButton::toggled, this,
    [this](bool expanded)
    {
      SetSectionExpanded(m_Controls->presetExpandButton, m_Controls->presetPanel, expanded);
      this->StartThumbnailGeneration();
    });

  connect(m_Controls->adjustPresetExpandButton, &QToolButton::toggled, this,
    [this](bool expanded)
    {
      SetSectionExpanded(m_Controls->adjustPresetExpandButton, m_Controls->adjustPresetPanel, expanded);
    });

  connect(m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::OpacityChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnCanvasOpacityChanged);
  connect(m_Controls->opacityShiftSlider, &ctkDoubleSlider::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetOpacityShift);
  connect(m_Controls->opacityHeightSlider, &ctkDoubleSlider::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetOpacityHeight);
  connect(m_Controls->colorShiftSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnColorWindowChanged);
  connect(m_Controls->colorWidthSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnColorWindowChanged);
  connect(m_Controls->resetTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnResetAdjustments);

  connect(m_Controls->createTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnCreateCustom);
  connect(m_Controls->loadTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnImportCustom);
  connect(m_Controls->cancelTfCreationButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnCancelCustom);

  // What a dialog's close box is to its Cancel button: the same way out, said
  // twice, so that one of them is on screen whatever the panel is scrolled to.
  connect(m_Controls->backToPresetsButton, &QToolButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnCancelCustom);
  connect(m_Controls->doneTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnDoneCustom);
  connect(m_Controls->saveUserTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnSaveCustom);
}

QmitkVolumeTransferFunctionEditor::~QmitkVolumeTransferFunctionEditor() = default;

bool QmitkVolumeTransferFunctionEditor::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::Resize && watched == m_Controls->presetListWidget->viewport())
    this->UpdatePresetGrid();

  return QWidget::eventFilter(watched, event);
}

void QmitkVolumeTransferFunctionEditor::changeEvent(QEvent *event)
{
  QWidget::changeEvent(event);

  // Nothing else announces that the previews became worth drawing: switching
  // volume rendering on deliberately does not re-bind the node.
  if (event->type() == QEvent::EnabledChange && this->isEnabled())
    this->StartThumbnailGeneration();
}

void QmitkVolumeTransferFunctionEditor::UpdatePresetGrid()
{
  auto *presetList = m_Controls->presetListWidget;

  // The viewport rather than the list: it is what remains once the frame and
  // the vertical scroll bar are accounted for, and that bar is a column of
  // pixels on Windows but nothing at all on macOS, where it floats over the
  // content.
  const int available = presetList->viewport()->width();

  if (available <= 0)
    return;

  // The view keeps a margin of its own inside the viewport, so cells adding up
  // to the whole of it are one too many to fit: the last wraps and leaves most
  // of a cell empty. Two pixels is what it takes here, and a couple more than
  // that costs nothing visible while covering styles that take more.
  const int usable = available - VIEWPORT_RESERVE;

  const int columns = std::max(MIN_COLUMNS, usable / PREFERRED_CELL_WIDTH);

  // Dividing the width by the columns rather than the other way around is what
  // has the cells share out what there is, instead of leaving whatever the
  // last column did not need unused at the right.
  const int cellWidth = usable / columns;

  const QSize previewSize = PreviewSize(cellWidth - 2 * CELL_PADDING);

  // How many lines a name takes is not something to assume: the catalogued
  // ones are hyphenated and Qt breaks a line at a hyphen, so the count follows
  // from the width a cell has and from how wide the platform's interface font
  // draws the characters. The rect overload constrains wrapping by its width
  // and leaves the height free, which is the measurement the delegate itself
  // makes when it paints a name. The tallest of them keeps every cell the same
  // height and none of them too short.
  const QFontMetrics metrics = presetList->fontMetrics();
  int nameHeight = metrics.lineSpacing();

  for (int i = 0; i < presetList->count(); ++i)
  {
    const QRect nameBounds = metrics.boundingRect(QRect(0, 0, previewSize.width(), 0),
                                                  Qt::TextWordWrap,
                                                  presetList->item(i)->text());

    nameHeight = std::max(nameHeight, nameBounds.height());
  }

  const QSize cellSize(cellWidth, previewSize.height() + nameHeight + 2 * CELL_PADDING);

  // The grid decides where a cell goes only for entries that fill it. Left to
  // size themselves, they come out a couple of pixels narrower - the padding
  // here is wider than the margin the delegate keeps of its own accord - and
  // the view then packs each row from its entries' widths rather than from the
  // grid, so the columns of one row do not line up with those of the next.
  // Stating the size every entry is going to occupy is what holds them in step.
  for (int i = 0; i < presetList->count(); ++i)
    presetList->item(i)->setSizeHint(cellSize);

  presetList->setIconSize(previewSize);
  presetList->setGridSize(cellSize);

  // The size just settled on is the one the stand-ins were built for, and on
  // the first pass they were built for a viewport no layout had sized yet.
  this->RefreshPlaceholders();
}

void QmitkVolumeTransferFunctionEditor::SetDataNode(mitk::DataNode *node)
{
  // Authoring targets the node that was current when it started, and the
  // pre-edit snapshot is the only way back to that node's previous appearance.
  // This has to happen before the function is dropped below, or the snapshot
  // goes with it and the edits are stranded on a node nothing points at.
  this->OnCancelCustom();

  m_DataNode = node;
  m_AppliedTransferFunction = nullptr;

  // Previews belong to the image they were drawn from, so a different one
  // leaves them describing nothing that is on screen.
  if (m_ThumbnailImage != (node != nullptr ? node->GetDataAs<mitk::Image>() : nullptr))
    this->InvalidateThumbnails();

  this->AdoptTransferFunctionFromNode();

  this->StartThumbnailGeneration();
}

void QmitkVolumeTransferFunctionEditor::EnsureTransferFunction()
{
  if (m_AppliedTransferFunction.IsNotNull())
    return;

  auto node = m_DataNode.Lock();

  const auto presetName = QString::fromStdString(m_Presets.GetDefaultPresetName(
    node.IsNotNull() ? node->GetDataAs<mitk::Image>() : nullptr));

  // The grid was filled from the same catalog, so a name it vouches for has a
  // row. Nothing matches the empty name an empty catalog returns, which is the
  // one case this guards.
  const auto matches = m_Controls->presetListWidget->findItems(presetName, Qt::MatchExactly);

  if (matches.isEmpty())
    return;

  m_Controls->presetListWidget->setCurrentRow(m_Controls->presetListWidget->row(matches.first()));

  this->OnPresetSelected(presetName);
}

void QmitkVolumeTransferFunctionEditor::OnPresetSelected(const QString &presetName)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  const auto name = presetName.toStdString();

  auto blendMode = mitk::VolumeBlendMode::Composite;
  auto preset = m_Presets.CreateTransferFunction(name, blendMode);

  if (preset.IsNull())
    return;

  // Picking a preset starts a new recipe: the offsets the previous one left are
  // measured from a baseline this curve does not have. The name goes with them
  // and is written again immediately below.
  this->ForgetTransferFunctionRecipe(node);

  m_AppliedTransferFunction = preset;
  node->SetStringProperty(TF_PRESET_PROPERTY, name.c_str());

  // The mode travels with the curve: a window authored for MIP renders as a
  // white shell under composite, and a tissue classifier projected flat says
  // nothing. Applying it unconditionally is what makes the grid mean one thing
  // - the preset as its author intended it - rather than depending on the mode
  // the node happened to be left in.
  this->ApplyBlendMode(blendMode);

  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeTransferFunctionEditor::ApplyBlendMode(mitk::VolumeBlendMode blendMode)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  mitk::SetVolumeBlendMode(node.GetPointer(), blendMode);

  this->ShowNodeBlendMode();
}

void QmitkVolumeTransferFunctionEditor::ShowNodeBlendMode()
{
  const auto *description =
    mitk::VolumeBlendModeDescription::FromMode(BlendModeOrComposite(m_DataNode.Lock().GetPointer()));

  if (description == nullptr)
    return;

  // Blocked because this reports what the node already says; letting it through
  // would write that same value straight back and, on the way, emit a change
  // nobody made.
  const QSignalBlocker blocker(m_Controls->blendModeComboBox);

  m_Controls->blendModeComboBox->setCurrentIndex(
    m_Controls->blendModeComboBox->findData(QString::fromStdString(description->id)));
}

void QmitkVolumeTransferFunctionEditor::OnBlendModeChanged(int index)
{
  const auto *description =
    mitk::VolumeBlendModeDescription::FromId(m_Controls->blendModeComboBox->itemData(index).toString().toStdString());

  if (description == nullptr)
    return;

  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  mitk::SetVolumeBlendMode(node.GetPointer(), description->mode);

  // The curve itself did not change, but what the render window makes of it
  // did, and the host has no other signal to redraw on.
  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::AdoptTransferFunctionFromNode()
{
  auto node = m_DataNode.Lock();

  std::string presetName;

  if (node.IsNotNull())
  {
    node->GetStringProperty(TF_PRESET_PROPERTY, presetName);

    // A recorded preset and the custom marker are the direct evidence that this
    // node was set up here, and both survive the rendering flag being switched
    // off. Between them they cover every curve this widget applies. The flag
    // covers what predates them: nodes configured before the recipe existed, or
    // by the v1 view. What cannot serve as evidence is the TransferFunction
    // property itself - see IsVolumeRenderingOn. Adopting the mapper's default
    // would show a curve nobody chose and would also suppress
    // EnsureTransferFunction, which fires only while no function is held.
    if (!presetName.empty() || IsCustomTransferFunction(node.GetPointer()) ||
        IsVolumeRenderingOn(node.GetPointer()))
    {
      if (const auto *tfProperty =
            dynamic_cast<const mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction")))
      {
        m_AppliedTransferFunction = tfProperty->GetValue();
      }
    }
  }

  // Nothing matches the empty name a node that records no preset leaves behind,
  // and that absence is the state the section header describes rather than a row.
  const auto matches =
    m_Controls->presetListWidget->findItems(QString::fromStdString(presetName), Qt::MatchExactly);

  const int presetIndex = matches.isEmpty() ? -1 : m_Controls->presetListWidget->row(matches.first());

  if (presetIndex < 0)
    this->ClearPresetSelection();
  else
    m_Controls->presetListWidget->setCurrentRow(presetIndex);

  // Replaying needs a preset the catalog still offers, since that is the
  // baseline the recorded offsets are measured from.
  if (presetIndex >= 0 && this->ReplayAdjustOffsets(presetName))
    return;

  this->ShowAppliedTransferFunction();
}

bool QmitkVolumeTransferFunctionEditor::ReplayAdjustOffsets(const std::string &presetName)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return false;

  float opacityShift = 0.0f;
  float opacityHeight = 0.0f;
  float colorShift = 0.0f;
  float colorWidth = 0.0f;

  // All four or none. The neutral colour width is the preset's own span rather
  // than zero, so a missing value cannot be told apart from a deliberate one and
  // a partial recipe cannot be completed with defaults.
  if (!node->GetFloatProperty(TF_OPACITY_SHIFT_PROPERTY, opacityShift) ||
      !node->GetFloatProperty(TF_OPACITY_HEIGHT_PROPERTY, opacityHeight) ||
      !node->GetFloatProperty(TF_COLOR_SHIFT_PROPERTY, colorShift) ||
      !node->GetFloatProperty(TF_COLOR_WIDTH_PROPERTY, colorWidth))
  {
    return false;
  }

  // Re-apply the preset first, so the baselines the offsets are measured from
  // are the pristine curves again. Showing an offset against an already-adjusted
  // baseline would make the next drag apply the whole offset a second time.
  //
  // The recipe therefore wins over the stored curve. The two agree wherever the
  // curve was produced here, and where they do not - the function was edited
  // through the Properties view, or by the v1 view - the edit is what gets
  // discarded. That is the price of being able to keep adjusting a preset, and
  // the same trade the lighting model's recorded id already makes.
  this->OnPresetSelected(QString::fromStdString(presetName));

  // Left to the sliders' own signals rather than applied directly: they are
  // already wired to the canvas and to OnColorWindowChanged, and each of those
  // reads both of its pair, so the order here does not matter.
  m_Controls->opacityShiftSlider->setValue(opacityShift);
  m_Controls->opacityHeightSlider->setValue(opacityHeight);
  m_Controls->colorShiftSlider->setValue(colorShift);
  m_Controls->colorWidthSlider->setValue(colorWidth);

  return true;
}

void QmitkVolumeTransferFunctionEditor::RecordAdjustOffsets()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  // An offset is only meaningful next to the preset it was measured from, so a
  // node naming none has no baseline these four could describe. Writing them
  // anyway would leave a recipe nothing can re-execute on the node, and in every
  // scene saved from it.
  std::string presetName;

  if (!node->GetStringProperty(TF_PRESET_PROPERTY, presetName))
    return;

  node->SetFloatProperty(TF_OPACITY_SHIFT_PROPERTY,
    static_cast<float>(m_Controls->opacityShiftSlider->value()));
  node->SetFloatProperty(TF_OPACITY_HEIGHT_PROPERTY,
    static_cast<float>(m_Controls->opacityHeightSlider->value()));
  node->SetFloatProperty(TF_COLOR_SHIFT_PROPERTY,
    static_cast<float>(m_Controls->colorShiftSlider->value()));
  node->SetFloatProperty(TF_COLOR_WIDTH_PROPERTY,
    static_cast<float>(m_Controls->colorWidthSlider->value()));
}

void QmitkVolumeTransferFunctionEditor::ForgetTransferFunctionRecipe(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  // Removing the keys rather than blanking them keeps absence as the single
  // meaning each of them carries, which is what the restore path reads.
  auto *properties = node->GetPropertyList();

  properties->DeleteProperty(TF_PRESET_PROPERTY);
  properties->DeleteProperty(TF_OPACITY_SHIFT_PROPERTY);
  properties->DeleteProperty(TF_OPACITY_HEIGHT_PROPERTY);
  properties->DeleteProperty(TF_COLOR_SHIFT_PROPERTY);
  properties->DeleteProperty(TF_COLOR_WIDTH_PROPERTY);
  properties->DeleteProperty(TF_CUSTOM_PROPERTY);
}

void QmitkVolumeTransferFunctionEditor::RecordCustomTransferFunction(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  this->ForgetTransferFunctionRecipe(node);

  node->SetBoolProperty(TF_CUSTOM_PROPERTY, true);
}

void QmitkVolumeTransferFunctionEditor::ClearPresetSelection()
{
  // Which preset is named, if any, is said by the section header, and saying it
  // there means it is legible while the grid is folded away.
  m_Controls->presetListWidget->setCurrentRow(-1);
}

void QmitkVolumeTransferFunctionEditor::ApplyCurrentTransferFunction()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull() || m_AppliedTransferFunction.IsNull())
    return;

  // Preserve property identity: reuse the existing property, create it only once.
  auto *tfProperty = dynamic_cast<mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction"));

  if (tfProperty != nullptr)
  {
    tfProperty->SetValue(m_AppliedTransferFunction);
  }
  else
  {
    node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(m_AppliedTransferFunction));
  }

  this->ShowAppliedTransferFunction();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::ShowAppliedTransferFunction()
{
  auto node = m_DataNode.Lock();

  if (m_AppliedTransferFunction.IsNull())
  {
    // Not merely cosmetic: the canvas keeps the functions as raw pointers owned
    // by the transfer function just dropped, so leaving them in place leaves
    // them reachable after the node that owned them is deleted.
    m_Controls->combinedTfCanvas->Clear();
    m_DataRange = { 0.0, 0.0 };
  }
  else
  {
    auto *image = node.IsNotNull() ? node->GetDataAs<mitk::Image>() : nullptr;
    mitk::SimpleHistogram *histogram = (image != nullptr) ? m_HistogramCache[image] : nullptr;

    // One that failed to compute answers 0 and 1 for its bounds rather than
    // reporting the failure, and those would pass for a data range and collapse
    // the axis and all four sliders onto a one-unit span. Taken as absent, which
    // everything downstream already reads as "no range known".
    if (histogram != nullptr && !histogram->GetValid())
      histogram = nullptr;

    m_Controls->combinedTfCanvas->SetHistogram(histogram);
    m_Controls->combinedTfCanvas->SetColorTransferFunction(m_AppliedTransferFunction->GetColorTransferFunction());
    m_Controls->combinedTfCanvas->SetPiecewiseFunction(m_AppliedTransferFunction->GetScalarOpacityFunction());

    if (const auto range = WorkingRange(image, histogram); range.has_value())
    {
      // The visible x-axis, so histogram, gradient and curve line up.
      // (SetPiecewiseFunction defaulted these to the function's own range, so
      // this must come after it.)
      m_DataRange = *range;
      m_Controls->combinedTfCanvas->SetMin(m_DataRange[0]);
      m_Controls->combinedTfCanvas->SetMax(m_DataRange[1]);
    }

    m_Controls->combinedTfCanvas->SnapshotOpacityBaseline();
  }

  this->SnapshotAppliedTransferFunction();
  this->ResetAdjustSliders();

  // The adjust controls and the canvas mean nothing without a function to act
  // on, and neither does authoring, which starts from the applied curve.
  // Picking a preset or loading one from file gets by with a node alone.
  const bool hasNode = node.IsNotNull();
  const bool adjustable = hasNode && m_AppliedTransferFunction.IsNotNull();

  // Named on the header, so that it reads while the grid is folded away. A
  // curve loaded from a file or authored here answers to no preset name, which
  // is worth telling apart from carrying no curve at all.
  const auto *currentPreset = m_Controls->presetListWidget->currentItem();

  m_Controls->presetExpandButton->setText(currentPreset != nullptr
    ? QString("Preset: %1").arg(currentPreset->text())
    : (m_AppliedTransferFunction.IsNotNull() ? "Preset: Custom" : "Select a preset"));

  m_Controls->presetExpandButton->setEnabled(hasNode);
  m_Controls->presetListWidget->setEnabled(hasNode);
  m_Controls->createTfButton->setEnabled(adjustable);
  m_Controls->loadTfButton->setEnabled(hasNode);
  m_Controls->adjustPresetExpandButton->setEnabled(adjustable);
  m_Controls->adjustPresetPanel->setEnabled(adjustable);
  m_Controls->combinedTfCanvas->setEnabled(adjustable);
}

void QmitkVolumeTransferFunctionEditor::SnapshotAppliedTransferFunction()
{
  if (m_AppliedTransferFunction.IsNull())
  {
    m_BaseColorFn = nullptr;
    return;
  }

  // Keep the untouched copy of the color function to resample from:
  // DeepCopy preserves the color space and clamping, so windowing
  // stays faithful to the preset. Sampling bare RGB points instead would
  // interpolate in the wrong color space and shift the colors on the
  // first slider move.
  m_BaseColorFn = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_BaseColorFn->DeepCopy(m_AppliedTransferFunction->GetColorTransferFunction());
}

double QmitkVolumeTransferFunctionEditor::NeutralColorWidth() const
{
  // The width at which the colour window reproduces the baseline unchanged, and
  // so the value the width slider resets to. Falls back to the image's range for
  // a baseline that names none of its own.
  const double dataWidth = std::max(1.0, m_DataRange[1] - m_DataRange[0]);

  if (m_BaseColorFn == nullptr || m_BaseColorFn->GetSize() == 0)
    return dataWidth;

  const double *colorRange = m_BaseColorFn->GetRange();

  return std::max(1.0, colorRange[1] - colorRange[0]);
}

void QmitkVolumeTransferFunctionEditor::ResetAdjustSliders()
{
  const QSignalBlocker blockOpacityShift(m_Controls->opacityShiftSlider);
  const QSignalBlocker blockOpacityWidth(m_Controls->opacityHeightSlider);
  const QSignalBlocker blockShift(m_Controls->colorShiftSlider);
  const QSignalBlocker blockWidth(m_Controls->colorWidthSlider);

  // Scale the sliders to the image's own value range -- the same axis the canvas
  // draws -- so the window can be moved and sized across everything you see.
  const double dataWidth = std::max(1.0, m_DataRange[1] - m_DataRange[0]);
  const double colorSpan = this->NeutralColorWidth();

  // Slide the curve along the intensity range, reaches half the data span either side
  m_Controls->opacityShiftSlider->setMinimum(-0.5 * dataWidth);
  m_Controls->opacityShiftSlider->setMaximum(0.5 * dataWidth);
  m_Controls->opacityShiftSlider->setSingleStep(dataWidth / 1000.0);
  m_Controls->opacityShiftSlider->setValue(0.0);

  // Signed offset in [-1, 1]; small range needs a fine step (default is 1.0).
  m_Controls->opacityHeightSlider->setMinimum(-1.0);
  m_Controls->opacityHeightSlider->setMaximum(1.0);
  m_Controls->opacityHeightSlider->setSingleStep(0.01);
  m_Controls->opacityHeightSlider->setValue(0.0);

  // Shift moves the window center (level); 0 keeps the preset's own center.
  // Sized to the preset's color span, so shift reaches half the preset span either side.
  m_Controls->colorShiftSlider->setMinimum(-0.5 * colorSpan);
  m_Controls->colorShiftSlider->setMaximum(0.5 * colorSpan);
  m_Controls->colorShiftSlider->setValue(0.0);

  // Width is the window size in intensity units; default to the preset's color
  // span so a fresh preset maps 1:1, and allow narrowing/widening around it.
  m_Controls->colorWidthSlider->setMinimum(1.0);
  m_Controls->colorWidthSlider->setMaximum(2.0 * colorSpan);
  m_Controls->colorWidthSlider->setValue(colorSpan);
}

void QmitkVolumeTransferFunctionEditor::OnColorWindowChanged()
{
  auto node = m_DataNode.Lock();

  if (m_AppliedTransferFunction.IsNull() || node.IsNull() || m_BaseColorFn == nullptr)
    return;

  if (m_DataRange[1] <= m_DataRange[0]) // no valid histogram range to span
    return;

  auto colorTable = mitk::ResampleColorWindow(m_BaseColorFn, m_DataRange[0], m_DataRange[1],
    m_Controls->colorShiftSlider->value(), m_Controls->colorWidthSlider->value());

  if (colorTable.empty())
    return;

  // Sorts the function once for the whole table, where handing the nodes over
  // one at a time re-sorted it on every insert. Clears what was there first, so
  // the table stands for the curve entire.
  m_AppliedTransferFunction->GetColorTransferFunction()->BuildFunctionFromTable(
    m_DataRange[0], m_DataRange[1], static_cast<int>(colorTable.size() / 3), colorTable.data());

  this->RecordAdjustOffsets();
  m_Controls->combinedTfCanvas->update();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::OnCanvasOpacityChanged()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  // Nothing to notify here: the canvas edited the scalar opacity function in
  // place, and what the ray caster re-uploads against is that function's own
  // modification time, which the edit already moved.
  this->RecordAdjustOffsets();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::OnResetAdjustments()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  // Reset returns the four offsets to neutral rather than reloading the preset.
  // The sliders are measured from whatever baseline the editor holds - a pristine
  // preset, or a loaded file's own curve - so neutral restores that baseline
  // either way and needs no preset to be named.
  //
  // Driven through the sliders rather than by rebuilding the function, so that
  // the canvas and the colour window follow and the handles end up where the
  // curve says they are. ResetAdjustSliders suppresses exactly these signals, by
  // design, which is why it cannot stand in here.
  m_Controls->opacityShiftSlider->setValue(0.0);
  m_Controls->opacityHeightSlider->setValue(0.0);
  m_Controls->colorShiftSlider->setValue(0.0);
  m_Controls->colorWidthSlider->setValue(this->NeutralColorWidth());
}

void QmitkVolumeTransferFunctionEditor::SetCustomModeActive(bool active)
{
  // SetDataNode cancels authoring on every selection change, whether or not any
  // was in progress. Everything below is a transition - it moves focus and has
  // the host lay its panel out again - so a request for the mode already in
  // force stops here, or selecting an image scrolls the host's panel back to
  // its top and takes focus off whatever the user was working in.
  if (active == m_CustomModeActive)
    return;

  m_CustomModeActive = active;

  m_Controls->advancedTfPanel->setVisible(active);

  // Preset selection and the sliders that adjust it both live on this panel,
  // and authoring supersedes both. Hiding the panel takes the collapsible
  // section with it, so its expanded/collapsed state is left untouched and
  // survives a trip through authoring.
  m_Controls->transferFunctionPanel->setVisible(!active);

  // The button that was clicked has just been hidden, so focus is about to be
  // handed back to the window unless it is given somewhere. Both targets sit at
  // the top of the page they belong to, which is where the panel is scrolled to.
  if (active)
    m_Controls->backToPresetsButton->setFocus(Qt::OtherFocusReason);
  else
    m_Controls->presetExpandButton->setFocus(Qt::OtherFocusReason);

  emit CustomModeChanged(active);
}

void QmitkVolumeTransferFunctionEditor::OnCreateCustom()
{
  auto node = m_DataNode.Lock();

  // Authoring is what hands the per-point editor the node it writes to, so
  // entering the mode with nothing to hand over would leave it bound to the
  // node it was given last while the page names this one.
  if (node.IsNull() || m_AppliedTransferFunction.IsNull() || m_BaseColorFn == nullptr)
    return;

  // Snapshot of the preset function so Cancel can restore it verbatim, which
  // matters once the adjust sliders have moved.
  m_PreEditTransferFunction = m_AppliedTransferFunction->Clone();
  m_PreEditBlendMode = BlendModeOrComposite(node.GetPointer());

  // A colour window bakes itself into 256 evenly spaced RGB points, which
  // would swamp the per-point editor. Restoring the baseline first keeps the
  // handles countable.
  m_AppliedTransferFunction->GetColorTransferFunction()->DeepCopy(m_BaseColorFn);

  m_Controls->tfControlPanelsWidget->SetDataNode(node);

  emit TransferFunctionChanged();

  // The control is only on screen in authoring mode, so this is where it has to
  // catch up with whatever the preset the node came from left behind.
  this->ShowNodeBlendMode();

  // The page covers the image selector, so it has to name the image itself.
  // Escaped, because the label reads its text as markup and a node is named by
  // whoever loaded it.
  m_Controls->customHintLabel->setText(
    QString("<small>Editing <b>%1</b>. The 3D window follows every change.</small>")
      .arg(QString::fromStdString(node->GetName()).toHtmlEscaped()));

  this->SetCustomModeActive(true);
}

void QmitkVolumeTransferFunctionEditor::OnImportCustom()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  auto fileName =
    QFileDialog::getOpenFileName(this, "Load transfer function", QString(), "Transfer function (*.json)");

  if (fileName.isEmpty())
    return;

  std::ifstream stream(fileName.toStdString());

  if (!stream.is_open())
  {
    QMessageBox::warning(this, "Load transfer function", "Could not open the file.");
    return;
  }

  auto blendMode = mitk::VolumeBlendMode::Composite;
  auto transferFunction = mitk::TransferFunctionPresets::LoadTransferFunction(stream, blendMode);

  if (transferFunction.IsNull())
  {
    QMessageBox::warning(this, "Load transfer function", "The file does not contain a valid transfer function.");
    return;
  }

  // A curve authored for a projection mode carries that mode in the file, and
  // reading it back without would render it in a mode it says nothing under.
  this->ApplyBlendMode(blendMode);

  // Enable rendering so the loaded function is visible immediately. The host
  // learns of it through TransferFunctionChanged.
  node->SetProperty("volumerendering", mitk::BoolProperty::New(true));

  m_AppliedTransferFunction = transferFunction;

  // A loaded custom function came from no preset, so there is no baseline any
  // recorded offsets could be measured from either.
  this->RecordCustomTransferFunction(node);
  this->ClearPresetSelection();

  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeTransferFunctionEditor::OnCancelCustom()
{
  if (m_AppliedTransferFunction.IsNotNull() && m_PreEditTransferFunction.IsNotNull())
  {
    // Discard the authoring edits: copy the pre-edit functions back into the
    // live ones in place, so the canvas and node pointers stay valid and the
    // opacity baseline, unchanged, still matches the restored curve.
    m_AppliedTransferFunction->GetColorTransferFunction()->DeepCopy(
      m_PreEditTransferFunction->GetColorTransferFunction());
    m_AppliedTransferFunction->GetScalarOpacityFunction()->DeepCopy(
      m_PreEditTransferFunction->GetScalarOpacityFunction());
    m_AppliedTransferFunction->GetGradientOpacityFunction()->DeepCopy(
      m_PreEditTransferFunction->GetGradientOpacityFunction());
    m_PreEditTransferFunction = nullptr;

    this->ApplyBlendMode(m_PreEditBlendMode);

    m_Controls->combinedTfCanvas->update();

    emit TransferFunctionChanged();
  }

  this->SetCustomModeActive(false);
}

void QmitkVolumeTransferFunctionEditor::OnDoneCustom()
{
  auto node = m_DataNode.Lock();

  // Before anything else can cancel: SetDataNode cancels authoring on every
  // selection change, and a snapshot left behind would copy the pre-authoring
  // curve back over the one just kept.
  m_PreEditTransferFunction = nullptr;

  if (node.IsNull() || m_AppliedTransferFunction.IsNull())
  {
    this->SetCustomModeActive(false);
    return;
  }

  // Authored point by point, so the preset name and the four offsets no longer
  // describe the curve, and replaying them on the next selection would rebuild
  // the preset and discard it. Same reasoning as a function loaded from a file.
  this->RecordCustomTransferFunction(node);
  this->ClearPresetSelection();

  // Enable rendering so the authored curve is visible immediately. The host
  // learns of it through TransferFunctionChanged.
  node->SetProperty("volumerendering", mitk::BoolProperty::New(true));

  this->SetCustomModeActive(false);

  // Re-seeds the combined canvas and re-snapshots both baselines, so the adjust
  // sliders now measure from the authored curve rather than from the preset it
  // started out as.
  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeTransferFunctionEditor::OnSaveCustom()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  auto fileName =
    QFileDialog::getSaveFileName(this, "Save transfer function", QString(), "Transfer function (*.json)");

  if (fileName.isEmpty())
    return;

  if (!fileName.endsWith(".json", Qt::CaseInsensitive))
    fileName += ".json";

  std::ofstream stream(fileName.toStdString());
  const auto name = QFileInfo(fileName).completeBaseName().toStdString();

  if (!stream.is_open() ||
      !mitk::TransferFunctionPresets::SaveTransferFunction(stream, name, m_AppliedTransferFunction.GetPointer(),
        BlendModeOrComposite(m_DataNode.Lock().GetPointer())))
  {
    QMessageBox::warning(this, "Save transfer function", "Could not save the transfer function.");
  }
}

void QmitkVolumeTransferFunctionEditor::InvalidateThumbnails()
{
  // Bumping the version is what abandons a generation already under way: its
  // next step finds the number changed and stops without queuing another.
  ++m_ThumbnailRun;

  // Before zero, because the volume has to be bound before anything is drawn,
  // and that is a step of its own.
  m_NextThumbnailIndex = -1;
  m_ThumbnailImage = nullptr;

  // The renderer keeps the volume it has bound alive, and nothing here applies
  // to it any more - including the case where the node it came with has just
  // been removed, which nothing else would free it on.
  m_ThumbnailRenderer->SetImage(nullptr);

  // Back to the stand-in rather than to nothing, so that clearing the previews
  // does not resize every entry and scatter the grid.
  this->RefreshPlaceholders();
}

void QmitkVolumeTransferFunctionEditor::RefreshPlaceholders()
{
  auto *presetList = m_Controls->presetListWidget;

  const QIcon placeholder = PlaceholderPreview(presetList->iconSize(),
                                               QColor(QmitkStyleManager::GetIconColor()));

  // Previews are filled in one after another from the front, so the index of
  // the next one is also the first entry still showing a stand-in. Below zero
  // no volume is bound yet and every entry is one.
  for (int i = std::max(0, m_NextThumbnailIndex); i < presetList->count(); ++i)
    presetList->item(i)->setIcon(placeholder);
}

void QmitkVolumeTransferFunctionEditor::StartThumbnailGeneration()
{
  auto node = m_DataNode.Lock();
  auto *image = node.IsNotNull() ? node->GetDataAs<mitk::Image>() : nullptr;

  if (image == nullptr)
    return;

  // Either the previews already describe this image or they are being drawn
  // for it; neither wants starting again.
  if (m_ThumbnailImage == image)
    return;

  // Whether previews can be drawn is asked of every image rather than once of
  // the machine: the ray caster refuses some volumes it is handed, RGB ones
  // among them, and one such refusal must not write off the images after it.
  // SetImage reports its own refusal, and GenerateNextThumbnail acts on it.
  this->InvalidateThumbnails();
  m_ThumbnailImage = image;

  const int run = m_ThumbnailRun;
  QTimer::singleShot(0, this, [this, run] { this->GenerateNextThumbnail(run); });
}

void QmitkVolumeTransferFunctionEditor::GenerateNextThumbnail(int run)
{
  if (run != m_ThumbnailRun)
    return;

  // Asked here rather than where generation is requested: a selection change
  // binds the node first and settles whether the editor applies to it second,
  // so only a turn of the event loop later is the answer the current one.
  if (!this->isEnabled() || !m_Controls->presetExpandButton->isChecked())
  {
    this->InvalidateThumbnails();
    return;
  }

  auto *presetList = m_Controls->presetListWidget;

  if (m_NextThumbnailIndex < 0)
  {
    // Uploading the volume costs far more than drawing from it, so it waits
    // for the preset section to be on screen rather than delaying its
    // appearance.
    if (!m_ThumbnailRenderer->SetImage(m_ThumbnailImage.Lock().GetPointer()))
    {
      // Clearing the bound image matters: it is what lets a later attempt
      // start, rather than reading as a generation already finished.
      this->InvalidateThumbnails();
      return;
    }

    m_NextThumbnailIndex = 0;
  }
  else
  {
    auto *presetItem = presetList->item(m_NextThumbnailIndex);
    const auto presetName = presetItem->text().toStdString();

    // Drawn in the mode the preset names, or the two MIP presets would preview
    // as the white shells their windows composite into, and the grid would
    // misrepresent exactly the entries hardest to picture.
    auto blendMode = mitk::VolumeBlendMode::Composite;

    if (auto transferFunction = m_Presets.CreateTransferFunction(presetName, blendMode);
        transferFunction.IsNotNull())
    {
      const auto thumbnail = m_ThumbnailRenderer->Render(transferFunction.GetPointer(), blendMode);

      if (!thumbnail.isNull())
        presetItem->setIcon(QIcon(thumbnail));
    }

    ++m_NextThumbnailIndex;
  }

  if (m_NextThumbnailIndex >= presetList->count())
    return;

  // Queued rather than looped: returning to the event loop between previews is
  // what keeps the panel responsive and lets the grid fill in while it is open.
  QTimer::singleShot(0, this, [this, run] { this->GenerateNextThumbnail(run); });
}
