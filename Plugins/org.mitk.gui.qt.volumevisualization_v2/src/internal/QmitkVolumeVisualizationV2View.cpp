/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeVisualizationV2View.h"

#include <mitkImage.h>

#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionTransform.h>
#include <mitkVtkPropRenderer.h>
#include <QmitkCombinedTransferFunctionCanvas.h>
#include <QmitkRenderWindow.h>

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

#include <mitkProperties.h>

#include <ui_QmitkVolumeVisualizationV2View.h>

#include <ctkDoubleSlider.h>
#include <ctkSliderWidget.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>

const std::string QmitkVolumeVisualizationV2View::VIEW_ID = "org.mitk.views.volumevisualization_v2";

namespace
{
  // Mirrors mitk::VolumeMapperVtkSmart3D::SetDefaultProperties, which only runs
  // via the IOExt object factory; the view has no guarantee that it did.
  constexpr bool DEFAULT_SHADE = true;
  constexpr float DEFAULT_AMBIENT = 0.1f;
  constexpr float DEFAULT_DIFFUSE = 0.50f;
  constexpr float DEFAULT_SPECULAR = 0.40f;
  constexpr float DEFAULT_SPECULAR_POWER = 16.0f;

  constexpr float DEFAULT_SCATTERING_BLEND = 0.0f;
  constexpr float DEFAULT_SCATTERING_REACH = 0.0f;

  /** A lighting model is a fixed point in VTK's parameter space, not a slider
   * position, and it spans the light rig as well as the mapper.
   *
   * Blend does not add occlusion to Phong shading, it mixes Phong out and a
   * phase-function scattering model in:
   * finalColor = (1 - c) * phong + c * scattering.
   *
   * Below 1.0 the uniform is blend/2 and c = 2 * (blend/2) * exp(...), so c
   * never exceeds blend itself: the value reads directly as the largest share
   * of the shading the shadow ray can ever account for. That is the strength
   * dial, and it is the one a viewer actually wants - reach only decides how
   * far a shadow reaches, not how heavily it lands. At 1.0 there is no cap left
   * and scattering can replace Phong outright; 2.0 pins c to 1.0 and discards
   * N.L and specular everywhere.
   *
   * The two models are rigs, not strengths of one effect. Headlight is the only
   * configuration the ray caster compiles its default lighting path for, which
   * is the one that multiplies ambient by the sample colour; it is also, being
   * at the camera, the one that casts no visible shadow, so it carries no
   * scattering. Key light gives that up to put a light off-axis, which is what
   * makes a shadow ray describe shape rather than depth.
   *
   * Reach bounds that shadow ray, and the traced fraction of the volume
   * diagonal is 1 - (1 - reach)^0.33 - front-loaded against short values: 0.10
   * traces 3%, 0.30 traces 11%, 0.50 traces 20%, 0.82 traces 43%.
   *
   * Reach is held short deliberately. Long rays do integrate more smoothly -
   * below roughly 20% a sample's shadow is decided by one or two jittered
   * steps, so neighbouring pixels disagree and the shadow carries visible
   * grain - but what a long ray buys is one structure casting onto another,
   * and that describes the light rather than the patient. A short ray darkens
   * only contacts and recesses, which is the depth cue worth having. The grain
   * cannot be traded away by sampling finer: the shadow ray steps with the
   * primary ray, and vtkSmartVolumeMapper turns LockSampleDistanceToInputSpacing
   * on without exposing it, so an explicit SampleDistance is discarded whenever
   * it differs from the spacing-derived value.
   *
   * Ambient is the one field the two models disagree on, because the shader
   * computes it differently for each. Under the headlight it is multiplied by
   * the sample colour and behaves as a fill worth having. Under any rig with
   * more than one light it is scaled by the light's ambient colour and never by
   * the sample colour, making it an additive grey over the whole image that
   * desaturates the render long before it rescues an occluded voxel - so the
   * key light rig sets it to zero and fills from the fill light instead, whose
   * contribution does carry the sample colour and does get its own shadow ray.
   *
   * Diffuse rises to compensate where ambient is zero, and the key rig can
   * afford it: its key plus fill sum above a single light's intensity.
   *
   * Specular is written explicitly rather than left at the Phong default, which
   * no preset used to touch. The shader adds the specular term without the
   * sample colour, so it is white light laid over the render, and a light at
   * the camera puts its lobe over everything visible at once rather than off to
   * one side. At the old default of 0.4 that clips bright tissue to white.
   *
   * Anisotropy stays at 0. VTK's Henyey-Greenstein phase function carries no
   * 1/4pi normalisation, so it is exactly 1.0 at 0 but swings between 0.56 and
   * 1.88 at 0.2 depending on the light and view geometry - a brightness change
   * rather than a shape cue.
   */
  struct CinematicPreset
  {
    const char *label;
    float blend;
    float reach;
    float anisotropy;
    bool normalsFromOpacity;
    float ambient;
    float diffuse;
    float specular;
    float specularPower;
    mitk::VtkPropRenderer::LightingMode lightingMode;
  };

  using LightingMode = mitk::VtkPropRenderer::LightingMode;

  constexpr std::array<CinematicPreset, 2> CINEMATIC_PRESETS { {
    //                   blend  reach  aniso  nFromOp ambient diffuse specular power  rig
    {"Headlight",        0.00f, 0.00f, 0.0f,  false,  0.20f,  0.70f,  0.10f,   30.0f, LightingMode::Headlight },
    {"Key light",        0.40f, 0.12f, 0.0f,  false,  0.00f,  0.80f,  0.10f,   30.0f, LightingMode::KeyLight  },
  } };

  const CinematicPreset &PresetFromComboIndex(int index)
  {
    return index > 0 && index < static_cast<int>(CINEMATIC_PRESETS.size())
             ? CINEMATIC_PRESETS[index]
             : CINEMATIC_PRESETS.front();
  }

  int ComboIndexFromNode(const mitk::DataNode *node)
  {
    float blend = DEFAULT_SCATTERING_BLEND;
    float reach = DEFAULT_SCATTERING_REACH;
    node->GetFloatProperty("volumerendering.scattering.blend", blend);
    node->GetFloatProperty("volumerendering.scattering.reach", reach);

    // Blend gates the whole scattering path, so a zero blend is Off whatever
    // the reach happens to say.
    if (blend <= 0.0f)
      return 0;

    for (std::size_t i = 1; i < CINEMATIC_PRESETS.size(); ++i)
    {
      const auto &preset = CINEMATIC_PRESETS[i];

      if (std::abs(preset.blend - blend) < 1e-4f && std::abs(preset.reach - reach) < 1e-4f)
        return static_cast<int>(i);
    }

    // Scattering is on at a combination no preset describes - an older scene,
    // or hand-edited properties. Report no selection, as presetComboBox does
    // for an unrecognised transfer function, rather than naming a preset whose
    // values differ from what the mapper will render.
    return -1;
  }

  void ConfigureSlider(ctkSliderWidget *slider, int decimals, double minimum, double maximum, double step)
  {
    slider->setDecimals(decimals);
    slider->setRange(minimum, maximum);
    slider->setSingleStep(step);
  }

  void LoadSliderFromNode(const mitk::DataNode *node, const char *propertyKey, float fallback, ctkSliderWidget *slider)
  {
    float value = fallback;
    node->GetFloatProperty(propertyKey, value);

    const QSignalBlocker blocker(slider);
    slider->setValue(value);
  }
}

QmitkVolumeVisualizationV2View::QmitkVolumeVisualizationV2View() 
{
  m_Controls = std::make_unique<Ui::QmitkVolumeVisualizationV2View>();
}

QmitkVolumeVisualizationV2View::~QmitkVolumeVisualizationV2View()
{
}

void QmitkVolumeVisualizationV2View::SetFocus() 
{
}

void QmitkVolumeVisualizationV2View::CreateQtPartControl(QWidget *parent)
{
  m_Controls->setupUi(parent);
  m_Controls->volumeSelectionWidget->SetDataStorage(this->GetDataStorage());
  m_Controls->volumeSelectionWidget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Image>::New(),
    mitk::NodePredicateOr::New(mitk::NodePredicateDimension::New(3), mitk::NodePredicateDimension::New(4)),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
  m_Controls->volumeSelectionWidget->SetSelectionIsOptional(true);
  m_Controls->volumeSelectionWidget->SetEmptyInfo(QString("Please select a 3D / 4D image volume"));
  m_Controls->volumeSelectionWidget->SetPopUpTitel(QString("Select image volume"));

  m_Controls->tfControlPanelsWidget->ShowGradientOpacityFunction(false);

  for (const auto &name : m_Presets.GetPresetNames())
  {
    m_Controls->presetComboBox->addItem(QString::fromStdString(name));
  }

  //select the applied preset, or -1 for "none":
  m_Controls->presetComboBox->setCurrentIndex(-1);
  m_Controls->presetComboBox->setEnabled(false);
  m_Controls->enableRenderingCB->setEnabled(false);

  m_Controls->createTfButton->setEnabled(false);
  m_Controls->loadTfButton->setEnabled(false);

  m_Controls->opacityShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->opacityHeightSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorWidthSlider->setOrientation(Qt::Horizontal);

  ConfigureSlider(m_Controls->ambientSlider, 2, 0.0, 1.0, 0.01);
  ConfigureSlider(m_Controls->diffuseSlider, 2, 0.0, 1.0, 0.01);
  ConfigureSlider(m_Controls->specularSlider, 2, 0.0, 1.0, 0.01);
  ConfigureSlider(m_Controls->specularPowerSlider, 1, 1.0, 128.0, 1.0);

  for (const auto& preset : CINEMATIC_PRESETS)
  {
    m_Controls->cinematicModeComboBox->addItem(QString(preset.label));
  }

  m_Controls->lightingPanel->setVisible(false);

  m_Controls->tfControlPanelsWidget->setVisible(false);
  m_Controls->cancelTfCreationButton->setVisible(false);
  m_Controls->saveUserTfButton->setVisible(false);

  connect(m_Controls->volumeSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
      this, &QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged);
    
  connect(m_Controls->enableRenderingCB, &QCheckBox::toggled,
    this, &QmitkVolumeVisualizationV2View::OnEnabledRendering);
  
  connect(m_Controls->presetComboBox, &QComboBox::textActivated,
    this, &QmitkVolumeVisualizationV2View::OnTransferFunctionPresetSelected);

  // Transfer Function Adjustments
  connect(m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::OpacityChanged,
    this, &QmitkVolumeVisualizationV2View::OnCanvasOpacityChanged);
  connect(m_Controls->opacityShiftSlider, &ctkDoubleSlider::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetOpacityShift);
  connect(m_Controls->opacityHeightSlider, &ctkDoubleSlider::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetOpacityHeight);
  connect(m_Controls->colorShiftSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnColorWindowChanged);
  connect(m_Controls->colorWidthSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnColorWindowChanged);
  connect(m_Controls->resetTfButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnResetTransferFunction);

  // Lighting Option Controls
  connect(m_Controls->lightingExpandButton, &ctkExpandButton::toggled,
    m_Controls->lightingPanel, &QWidget::setVisible);
  connect(m_Controls->shadeCheckBox, &QCheckBox::toggled,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);
  connect(m_Controls->ambientSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);
  connect(m_Controls->diffuseSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);
  connect(m_Controls->specularSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);
  connect(m_Controls->specularPowerSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);
  connect(m_Controls->cinematicModeComboBox, &QComboBox::currentIndexChanged,
    this, &QmitkVolumeVisualizationV2View::OnCinematicModeChanged);
  connect(m_Controls->resetLightingButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnResetLighting);

  // Transfer Function User Creation Mode
  connect(m_Controls->createTfButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnCreateUserTransferFunction);
  connect(m_Controls->loadTfButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnImportUserTransferFunction);
  connect(m_Controls->cancelTfCreationButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnCancelTfAdvancedMode);
  connect(m_Controls->saveUserTfButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnSaveUserTransferFunction);

  m_Controls->volumeSelectionWidget->SetAutoSelectNewNodes(true);
}

void QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_SelectedNode = nullptr;
  m_AppliedTransferFunction = nullptr;

  if (nodes.empty() || nodes.front().IsNull())
  {
    this->UpdateInterface();
    return;
  }

  auto selectedNode = nodes.front();
  
  if (selectedNode->GetDataAs<mitk::Image>() != nullptr)
    m_SelectedNode = selectedNode;

  this->UpdateInterface();
}

void QmitkVolumeVisualizationV2View::OnEnabledRendering(bool state)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  selectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(state));

  if (state && m_AppliedTransferFunction.IsNull() && m_Controls->presetComboBox->count() > 0)
  {
    m_Controls->presetComboBox->setCurrentIndex(0);
    this->OnTransferFunctionPresetSelected(m_Controls->presetComboBox->itemText(0));

    // The mapper's registered defaults predate the lighting work and describe no
    // model this view offers, so the combo would name a preset whose values were
    // never written. Take the lighting over at the same point as the transfer
    // function rather than changing what the mapper registers for every plugin.
    this->OnCinematicModeChanged(0);
  }

  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::OnTransferFunctionPresetSelected(const QString &presetName)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  auto preset = m_Presets.CreateTransferFunction(presetName.toStdString());

  if (preset.IsNull())
    return;

  m_AppliedTransferFunction = preset;
  m_EffectiveRange = m_Presets.GetEffectiveRange(presetName.toStdString());

  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeVisualizationV2View::ApplyCurrentTransferFunction()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull() || m_AppliedTransferFunction.IsNull())
    return;

  // Preserve property identity: reuse the existing property, create it only once.
  auto *tfProperty = dynamic_cast<mitk::TransferFunctionProperty *>(selectedNode->GetProperty("TransferFunction"));

  if(tfProperty != nullptr)
  {
    tfProperty->SetValue(m_AppliedTransferFunction);
  } 
  else
  {
    selectedNode->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(m_AppliedTransferFunction));
  }

  //m_Controls->tfControlPanelsWidget->SetDataNode(selectedNode);

  auto *image = selectedNode->GetDataAs<mitk::Image>();
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

  this->SnapshotAppliedTransferFunction();
  this->ResetAdjustSliders();
  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::SnapshotAppliedTransferFunction()
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

void QmitkVolumeVisualizationV2View::ResetAdjustSliders()
{
  const QSignalBlocker blockOpacityShift(m_Controls->opacityShiftSlider);
  const QSignalBlocker blockOpacityWidth(m_Controls->opacityHeightSlider);
  const QSignalBlocker blockShift(m_Controls->colorShiftSlider);
  const QSignalBlocker blockWidth(m_Controls->colorWidthSlider);

  // Scale the sliders to the image's own value range -- the same axis the canvas
  // draws -- so the window can be moved and sized across everything you see.
  const double dataWidth = std::max(1.0, m_DataRange[1] - m_DataRange[0]);

  double colorSpan = dataWidth;
  if (m_BaseColorFn != nullptr && m_BaseColorFn->GetSize() > 0)
  {
    const double *colorRange = m_BaseColorFn->GetRange();
    colorSpan = std::max(1.0, colorRange[1] - colorRange[0]);
  }

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

void QmitkVolumeVisualizationV2View::OnColorWindowChanged()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (m_AppliedTransferFunction.IsNull() || selectedNode.IsNull() || m_BaseColorFn == nullptr)
    return;

  if (m_DataRange[1] <= m_DataRange[0]) // no valid histogram range to span
    return;

  m_AppliedTransferFunction->SetRGBPoints(
    mitk::ResampleColorWindow(m_BaseColorFn, m_DataRange[0], m_DataRange[1], m_Controls->colorShiftSlider->value(), m_Controls->colorWidthSlider->value()));

  m_AppliedTransferFunction->Modified();
  this->RequestRenderWindowUpdate();
  //m_Controls->tfControlPanelsWidget->OnUpdateCanvas();
  m_Controls->combinedTfCanvas->update();
}

void QmitkVolumeVisualizationV2View::OnLightingChanged()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  selectedNode->SetBoolProperty("volumerendering.shade", m_Controls->shadeCheckBox->isChecked());
  selectedNode->SetFloatProperty("volumerendering.ambient", static_cast<float>(m_Controls->ambientSlider->value()));
  selectedNode->SetFloatProperty("volumerendering.diffuse", static_cast<float>(m_Controls->diffuseSlider->value()));
  selectedNode->SetFloatProperty("volumerendering.specular", static_cast<float>(m_Controls->specularSlider->value()));
  selectedNode->SetFloatProperty("volumerendering.specular.power", static_cast<float>(m_Controls->specularPowerSlider->value()));

  this->UpdateLightingControls();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::OnCinematicModeChanged(int index)
{
  auto selectedNode = m_SelectedNode.Lock();

  // UpdateLightingControls reports -1 for a configuration no model describes,
  // but does so behind a QSignalBlocker, so a signal always names a real entry.
  if (selectedNode.IsNull() || index < 0)
    return;

  const auto &preset = PresetFromComboIndex(index);

  // VTK ignores both scattering parameters unless shading is on.
  if (preset.blend > 0.0f)
    selectedNode->SetBoolProperty("volumerendering.shade", true);

  selectedNode->SetFloatProperty("volumerendering.scattering.blend", preset.blend);
  selectedNode->SetFloatProperty("volumerendering.scattering.reach", preset.reach);
  selectedNode->SetFloatProperty("volumerendering.scattering.anisotropy", preset.anisotropy);
  selectedNode->SetBoolProperty("volumerendering.normalsFromOpacity", preset.normalsFromOpacity);
  selectedNode->SetFloatProperty("volumerendering.ambient", preset.ambient);
  selectedNode->SetFloatProperty("volumerendering.diffuse", preset.diffuse);
  selectedNode->SetFloatProperty("volumerendering.specular", preset.specular);
  selectedNode->SetFloatProperty("volumerendering.specular.power", preset.specularPower);

  // The rig is not applied here: UpdateLightingControls derives it from the
  // properties just written, so it stays the single place that installs one.
  this->UpdateLightingControls();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::ApplyLightingMode(mitk::VtkPropRenderer::LightingMode mode)
{
  auto *renderWindowPart = this->GetRenderWindowPart();

  if (renderWindowPart == nullptr)
    return;

  auto *renderWindow = renderWindowPart->GetQmitkRenderWindow("3d");

  if (renderWindow == nullptr)
    return;

  if (auto *renderer = renderWindow->GetRenderer(); renderer != nullptr)
    renderer->SetLightingMode(mode);
}

void QmitkVolumeVisualizationV2View::OnResetLighting()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  selectedNode->SetBoolProperty("volumerendering.shade", DEFAULT_SHADE);

  // Reset means the baseline this view offers, which is the Off model - not the
  // mapper's registered defaults, which describe no model in the combo. Sharing
  // the write keeps the two from drifting apart.
  this->OnCinematicModeChanged(0);
}

void QmitkVolumeVisualizationV2View::UpdateLightingControls()
{
  auto selectedNode = m_SelectedNode.Lock();

  bool volumeRenderingOn = false;

  if (selectedNode.IsNotNull())
    selectedNode->GetBoolProperty("volumerendering", volumeRenderingOn);

  m_Controls->lightingExpandButton->setEnabled(volumeRenderingOn);
  m_Controls->lightingPanel->setEnabled(volumeRenderingOn);

  if (!volumeRenderingOn)
  {
    // Nothing here is being rendered volumetrically, so nothing needs the
    // directional rig.
    this->ApplyLightingMode(mitk::VtkPropRenderer::LightingMode::Studio);
    return;
  }

  bool shade = DEFAULT_SHADE;
  selectedNode->GetBoolProperty("volumerendering.shade", shade);

  // Phong parameters; with shading off they do nothing.
  m_Controls->ambientSlider->setEnabled(shade);
  m_Controls->diffuseSlider->setEnabled(shade);
  m_Controls->specularSlider->setEnabled(shade);
  m_Controls->specularPowerSlider->setEnabled(shade);
  m_Controls->cinematicModeComboBox->setEnabled(shade);

  const QSignalBlocker blockShade(m_Controls->shadeCheckBox);
  m_Controls->shadeCheckBox->setChecked(shade);

  LoadSliderFromNode(selectedNode.GetPointer(), "volumerendering.ambient", DEFAULT_AMBIENT, m_Controls->ambientSlider);
  LoadSliderFromNode(selectedNode.GetPointer(), "volumerendering.diffuse", DEFAULT_DIFFUSE, m_Controls->diffuseSlider);
  LoadSliderFromNode(selectedNode.GetPointer(), "volumerendering.specular", DEFAULT_SPECULAR, m_Controls->specularSlider);
  LoadSliderFromNode(selectedNode.GetPointer(), "volumerendering.specular.power", DEFAULT_SPECULAR_POWER, m_Controls->specularPowerSlider);

  // Derive the mode from the node rather than storing it, so the node stays the
  // single source of truth.
  const int cinematicIndex = ComboIndexFromNode(selectedNode.GetPointer());

  const QSignalBlocker blockCinematic(m_Controls->cinematicModeComboBox);
  m_Controls->cinematicModeComboBox->setCurrentIndex(cinematicIndex);

  // The light rig follows the node as well, so selecting a different volume or
  // reloading a scene lands on the rig the stored properties need. A -1 index
  // is scattering at values no model describes; leave the even rig in place
  // rather than guess which directional one was meant.
  this->ApplyLightingMode(cinematicIndex >= 0
    ? PresetFromComboIndex(cinematicIndex).lightingMode
    : mitk::VtkPropRenderer::LightingMode::Studio);
}

void QmitkVolumeVisualizationV2View::SetTfAdvancedMode(bool active)
{
  // Controls for the Transfer Function Creation Mode
  m_Controls->tfControlPanelsWidget->setVisible(active);
  m_Controls->cancelTfCreationButton->setVisible(active);
  m_Controls->saveUserTfButton->setVisible(active);

  // Controls for the Preset Transfer Function Selection
  m_Controls->combinedTfCanvas->setVisible(!active);
  m_Controls->adjustPresetPanel->setVisible(!active);
  m_Controls->createTfButton->setVisible(!active);
  m_Controls->loadTfButton->setVisible(!active);
  m_Controls->tfAdvancedLabel->setVisible(!active);
}

void QmitkVolumeVisualizationV2View::OnCreateUserTransferFunction()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNotNull() && m_AppliedTransferFunction.IsNotNull() && m_BaseColorFn != nullptr)
  {
    // Snapshot of the "normal-mode" preset function so Cancel can restore it verbatim
    // Relevant if preset was adjusted using the sliders
    m_PreEditTransferFunction = m_AppliedTransferFunction->Clone();

    // Preset selection bakes the color window into 256 evenly spaced RGB point
    // which would swamp the per-point color editor in the advanced mode.
    // Currently does not reflect slider adjusted colors when entering advanced mode
    m_AppliedTransferFunction->GetColorTransferFunction()->DeepCopy(m_BaseColorFn);
    m_AppliedTransferFunction->Modified();

    m_Controls->tfControlPanelsWidget->SetDataNode(selectedNode);
    this->RequestRenderWindowUpdate();
  }

  this->SetTfAdvancedMode(true);
}

void QmitkVolumeVisualizationV2View::OnImportUserTransferFunction()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  auto fileName = QFileDialog::getOpenFileName(nullptr, "Load transfer function", QString(), "Transfer function (*.json)");

  if (fileName.isEmpty())
    return;

  std::ifstream stream(fileName.toStdString());

  if (!stream.is_open())
  {
    QMessageBox::warning(nullptr, "Load transfer function", "Could not open the file.");
    return;
  }

  auto transferFunction = mitk::TransferFunctionPresets::LoadTransferFunction(stream);

  if (transferFunction.IsNull())
  {
    QMessageBox::warning(nullptr, "Load transfer function", "The file does not contain a valid transfer function.");
    return;
  }

  // Enable rendering so the loaded function is visible immediately.
  selectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));

  m_AppliedTransferFunction = transferFunction;

  auto &scalarPoints = transferFunction->GetScalarOpacityPoints();
  m_EffectiveRange = scalarPoints.empty()
    ? std::array<double, 2>{ 0.0, 0.0 }
    : std::array<double, 2>{ scalarPoints.front().first, scalarPoints.back().first };

  // A loaded custom function is not one of the named presets.
  m_Controls->presetComboBox->setCurrentIndex(-1);

  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeVisualizationV2View::OnCancelTfAdvancedMode()
{
  if (m_AppliedTransferFunction.IsNotNull() && m_PreEditTransferFunction.IsNotNull())
  {
    // Discard the advanced edits: copy the pre-create functions back into the
    // live ones in place, so the canvas / node pointers stay valid and
    // the opacity baseline (unchanged) still matches the restored curve
    m_AppliedTransferFunction->GetColorTransferFunction()->DeepCopy(m_PreEditTransferFunction->GetColorTransferFunction());
    m_AppliedTransferFunction->GetScalarOpacityFunction()->DeepCopy(m_PreEditTransferFunction->GetScalarOpacityFunction());
    m_AppliedTransferFunction->GetGradientOpacityFunction()->DeepCopy(m_PreEditTransferFunction->GetGradientOpacityFunction());
    m_AppliedTransferFunction->Modified();
    m_PreEditTransferFunction = nullptr;

    m_Controls->combinedTfCanvas->update();
    this->RequestRenderWindowUpdate();
  }

  this->SetTfAdvancedMode(false);
}

void QmitkVolumeVisualizationV2View::OnSaveUserTransferFunction()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  auto fileName = QFileDialog::getSaveFileName(nullptr, "Save transfer function", QString(), "Transfer function (*.json)");

  if (fileName.isEmpty())
    return;

  if (!fileName.endsWith(".json", Qt::CaseInsensitive))
    fileName += ".json";

  std::ofstream stream(fileName.toStdString());
  const auto name = QFileInfo(fileName).completeBaseName().toStdString();

  if (!stream.is_open() ||
      !mitk::TransferFunctionPresets::SaveTransferFunction(stream, name, m_AppliedTransferFunction.GetPointer()))
  {
    QMessageBox::warning(nullptr, "Save transfer function", "Could not save the transfer function.");
  }
}

void QmitkVolumeVisualizationV2View::OnResetTransferFunction()
{
  // Reload the current preset: restores both the color map and the opacity curve
  // to the authored defaults and re-seeds the canvas + sliders.
  this->OnTransferFunctionPresetSelected(m_Controls->presetComboBox->currentText());
}

void QmitkVolumeVisualizationV2View::OnCanvasOpacityChanged()
{
  if (m_AppliedTransferFunction.IsNotNull())
    m_AppliedTransferFunction->Modified();

  // Keep the Advanced per-point editor in sync with the canvas edit.
  //m_Controls->tfControlPanelsWidget->OnUpdateCanvas();
}

void QmitkVolumeVisualizationV2View::UpdateInterface()
{
  auto selectedNode = m_SelectedNode.Lock();

  this->UpdateLightingControls();

  if(selectedNode.IsNull())
  {
    m_Controls->enableRenderingCB->setChecked(false);
    m_Controls->enableRenderingCB->setEnabled(false);
    m_Controls->presetComboBox->setEnabled(false);
    m_Controls->opacityShiftSlider->setEnabled(false);
    m_Controls->opacityHeightSlider->setEnabled(false);
    m_Controls->colorShiftSlider->setEnabled(false);
    m_Controls->colorWidthSlider->setEnabled(false);
    m_Controls->resetTfButton->setEnabled(false);
    m_Controls->combinedTfCanvas->setEnabled(false);
    return;
  }

  bool volumeRenderingOn = false;
  selectedNode->GetBoolProperty("volumerendering", volumeRenderingOn);

  m_Controls->enableRenderingCB->setEnabled(true);
  m_Controls->presetComboBox->setEnabled(volumeRenderingOn);
  m_Controls->createTfButton->setEnabled(volumeRenderingOn);
  m_Controls->loadTfButton->setEnabled(volumeRenderingOn);

  const bool tfAdjustable = volumeRenderingOn && m_AppliedTransferFunction.IsNotNull();
  m_Controls->opacityShiftSlider->setEnabled(tfAdjustable);
  m_Controls->opacityHeightSlider->setEnabled(tfAdjustable);
  m_Controls->colorShiftSlider->setEnabled(tfAdjustable);
  m_Controls->colorWidthSlider->setEnabled(tfAdjustable);
  m_Controls->resetTfButton->setEnabled(tfAdjustable);
  m_Controls->combinedTfCanvas->setEnabled(tfAdjustable);

  const QSignalBlocker blocker(m_Controls->enableRenderingCB);
  m_Controls->enableRenderingCB->setChecked(volumeRenderingOn);
}
