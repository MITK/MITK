/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeTransferFunctionEditor.h"

#include <mitkImage.h>
#include <mitkProperties.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionTransform.h>

#include <QmitkCombinedTransferFunctionCanvas.h>
#include <QmitkTransferFunctionWidget.h>

#include <ui_QmitkVolumeTransferFunctionEditorControls.h>

#include <ctkDoubleSlider.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

#include <algorithm>
#include <fstream>

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
   * absent on a function that came from a file or from outside this widget.
   */
  constexpr const char *TF_PRESET_PROPERTY = "volumerendering.transferfunction.preset";
  constexpr const char *TF_OPACITY_SHIFT_PROPERTY = "volumerendering.transferfunction.opacityshift";
  constexpr const char *TF_OPACITY_HEIGHT_PROPERTY = "volumerendering.transferfunction.opacityheight";
  constexpr const char *TF_COLOR_SHIFT_PROPERTY = "volumerendering.transferfunction.colorshift";
  constexpr const char *TF_COLOR_WIDTH_PROPERTY = "volumerendering.transferfunction.colorwidth";

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
    m_Controls(std::make_unique<Ui::QmitkVolumeTransferFunctionEditor>())
{
  m_Controls->setupUi(this);

  m_Controls->tfControlPanelsWidget->ShowGradientOpacityFunction(false);

  for (const auto &name : m_Presets.GetPresetNames())
  {
    m_Controls->presetComboBox->addItem(QString::fromStdString(name));
  }

  // A freshly filled combo lands on its first entry, which would name a preset
  // nothing has applied. -1 shows the placeholder instead.
  m_Controls->presetComboBox->setCurrentIndex(-1);

  m_Controls->opacityShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->opacityHeightSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorWidthSlider->setOrientation(Qt::Horizontal);

  m_Controls->advancedTfPanel->setVisible(false);

  connect(m_Controls->presetComboBox, &QComboBox::textActivated,
    this, &QmitkVolumeTransferFunctionEditor::OnPresetSelected);

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
  connect(m_Controls->saveUserTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnSaveCustom);
}

QmitkVolumeTransferFunctionEditor::~QmitkVolumeTransferFunctionEditor() = default;

void QmitkVolumeTransferFunctionEditor::SetDataNode(mitk::DataNode *node)
{
  // Authoring targets the node that was current when it started, and the
  // pre-edit snapshot is the only way back to that node's previous appearance.
  // This has to happen before the function is dropped below, or the snapshot
  // goes with it and the edits are stranded on a node nothing points at.
  this->OnCancelCustom();

  m_DataNode = node;
  m_AppliedTransferFunction = nullptr;

  this->AdoptTransferFunctionFromNode();
}

void QmitkVolumeTransferFunctionEditor::EnsureTransferFunction()
{
  if (m_AppliedTransferFunction.IsNotNull() || m_Controls->presetComboBox->count() == 0)
    return;

  m_Controls->presetComboBox->setCurrentIndex(0);
  this->OnPresetSelected(m_Controls->presetComboBox->itemText(0));
}

void QmitkVolumeTransferFunctionEditor::OnPresetSelected(const QString &presetName)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  const auto name = presetName.toStdString();
  auto preset = m_Presets.CreateTransferFunction(name);

  if (preset.IsNull())
    return;

  m_AppliedTransferFunction = preset;
  node->SetStringProperty(TF_PRESET_PROPERTY, name.c_str());

  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeTransferFunctionEditor::AdoptTransferFunctionFromNode()
{
  auto node = m_DataNode.Lock();

  std::string presetName;

  if (node.IsNotNull())
  {
    node->GetStringProperty(TF_PRESET_PROPERTY, presetName);

    // A recorded preset is the most direct evidence that this node was set up
    // here, and it survives the rendering flag being switched off. The flag
    // covers what predates the recipe: nodes configured before it existed, or by
    // the v1 view. What cannot serve as evidence is the TransferFunction
    // property itself - see IsVolumeRenderingOn. Adopting the mapper's default
    // would show a curve nobody chose and would also suppress
    // EnsureTransferFunction, which fires only while no function is held.
    if (!presetName.empty() || IsVolumeRenderingOn(node.GetPointer()))
    {
      if (const auto *tfProperty =
            dynamic_cast<const mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction")))
      {
        m_AppliedTransferFunction = tfProperty->GetValue();
      }
    }
  }

  // findText yields -1 for the empty name left by a node that records no preset,
  // and -1 is the placeholder the combo should show in exactly that case.
  const int presetIndex = m_Controls->presetComboBox->findText(QString::fromStdString(presetName));
  m_Controls->presetComboBox->setCurrentIndex(presetIndex);

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
  // meaning of "no recipe", which is what the restore path reads.
  auto *properties = node->GetPropertyList();

  properties->DeleteProperty(TF_PRESET_PROPERTY);
  properties->DeleteProperty(TF_OPACITY_SHIFT_PROPERTY);
  properties->DeleteProperty(TF_OPACITY_HEIGHT_PROPERTY);
  properties->DeleteProperty(TF_COLOR_SHIFT_PROPERTY);
  properties->DeleteProperty(TF_COLOR_WIDTH_PROPERTY);
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

    m_Controls->combinedTfCanvas->SetHistogram(histogram);
    m_Controls->combinedTfCanvas->SetColorTransferFunction(m_AppliedTransferFunction->GetColorTransferFunction());
    m_Controls->combinedTfCanvas->SetPiecewiseFunction(m_AppliedTransferFunction->GetScalarOpacityFunction());

    if (histogram != nullptr)
    {
      // Use the image scalar range as the visible x-axis so histogram, gradient
      // and curve line up. (SetPiecewiseFunction defaulted these to the function's
      // own range, so this must come after it.)
      m_DataRange = { histogram->GetMin(), histogram->GetMax() };
      m_Controls->combinedTfCanvas->SetMin(histogram->GetMin());
      m_Controls->combinedTfCanvas->SetMax(histogram->GetMax());
    }

    m_Controls->combinedTfCanvas->SnapshotOpacityBaseline();
  }

  this->SnapshotAppliedTransferFunction();
  this->ResetAdjustSliders();

  // The adjust controls and the canvas mean nothing without a function to act
  // on, and the preset combo and authoring buttons need a node to write to.
  const bool hasNode = node.IsNotNull();
  const bool adjustable = hasNode && m_AppliedTransferFunction.IsNotNull();

  m_Controls->presetComboBox->setEnabled(hasNode);
  m_Controls->createTfButton->setEnabled(hasNode);
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
  // DeepCopy preserves the color space (HSV) and clamping, so windowing
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

  m_AppliedTransferFunction->SetRGBPoints(
    mitk::ResampleColorWindow(m_BaseColorFn, m_DataRange[0], m_DataRange[1],
      m_Controls->colorShiftSlider->value(), m_Controls->colorWidthSlider->value()));

  m_AppliedTransferFunction->Modified();
  this->RecordAdjustOffsets();
  m_Controls->combinedTfCanvas->update();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::OnCanvasOpacityChanged()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  // The canvas edited the scalar opacity function in place, which leaves the
  // transfer function's own modification time untouched - and that is what the
  // mapper compares before it re-uploads. So bump it before asking for a render,
  // or the render draws the previous curve.
  m_AppliedTransferFunction->Modified();
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
  m_Controls->advancedTfPanel->setVisible(active);

  // Preset selection and the sliders that adjust it both live in this box, and
  // authoring supersedes both. Hiding the box takes the collapsible section
  // with it, so its expanded/collapsed state is left untouched and survives a
  // trip through authoring.
  m_Controls->transferFunctionGroupBox->setVisible(!active);

  emit CustomModeChanged(active);
}

void QmitkVolumeTransferFunctionEditor::OnCreateCustom()
{
  auto node = m_DataNode.Lock();

  if (node.IsNotNull() && m_AppliedTransferFunction.IsNotNull() && m_BaseColorFn != nullptr)
  {
    // Snapshot of the preset function so Cancel can restore it verbatim, which
    // matters once the adjust sliders have moved.
    m_PreEditTransferFunction = m_AppliedTransferFunction->Clone();

    // A colour window bakes itself into 256 evenly spaced RGB points, which
    // would swamp the per-point editor. Restoring the baseline first keeps the
    // handles countable.
    m_AppliedTransferFunction->GetColorTransferFunction()->DeepCopy(m_BaseColorFn);
    m_AppliedTransferFunction->Modified();

    m_Controls->tfControlPanelsWidget->SetDataNode(node);

    emit TransferFunctionChanged();
  }

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

  auto transferFunction = mitk::TransferFunctionPresets::LoadTransferFunction(stream);

  if (transferFunction.IsNull())
  {
    QMessageBox::warning(this, "Load transfer function", "The file does not contain a valid transfer function.");
    return;
  }

  // Enable rendering so the loaded function is visible immediately. The host
  // learns of it through TransferFunctionChanged.
  node->SetProperty("volumerendering", mitk::BoolProperty::New(true));

  m_AppliedTransferFunction = transferFunction;

  // A loaded custom function came from no preset, so there is no baseline any
  // recorded offsets could be measured from either.
  this->ForgetTransferFunctionRecipe(node);
  m_Controls->presetComboBox->setCurrentIndex(-1);

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
    m_AppliedTransferFunction->Modified();
    m_PreEditTransferFunction = nullptr;

    m_Controls->combinedTfCanvas->update();

    emit TransferFunctionChanged();
  }

  this->SetCustomModeActive(false);
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
      !mitk::TransferFunctionPresets::SaveTransferFunction(stream, name, m_AppliedTransferFunction.GetPointer()))
  {
    QMessageBox::warning(this, "Save transfer function", "Could not save the transfer function.");
  }
}
