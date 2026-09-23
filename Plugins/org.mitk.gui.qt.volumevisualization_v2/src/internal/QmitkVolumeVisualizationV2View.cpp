/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeVisualizationV2View.h"

#include <mitkImage.h>

#include <mitkVolumeBlendMode.h>
#include <mitkVolumeRenderingLightingModel.h>
#include <mitkVtkPropRenderer.h>
#include <QmitkVolumeLightingWidget.h>
#include <QmitkVolumeTransferFunctionEditor.h>
#include <QmitkRenderWindow.h>
#include <QmitkStyleManager.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

#include <mitkProperties.h>

#include <ui_QmitkVolumeVisualizationV2View.h>

#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>

#include <optional>

const std::string QmitkVolumeVisualizationV2View::VIEW_ID = "org.mitk.views.volumevisualization_v2";

namespace
{
  /** \brief Whether the node is rendered as a volume at all.
   *
   * Nothing outside this view writes the property, so it doubles as the marker
   * that someone deliberately configured this node here. "TransferFunction"
   * cannot serve that purpose: mitk::VolumeMapperVtkSmart3D registers a default
   * one on every image node, so its presence says nothing.
   */
  bool IsVolumeRenderingOn(const mitk::DataNode *node)
  {
    if (node == nullptr)
      return false;

    bool volumeRenderingOn = false;
    node->GetBoolProperty("volumerendering", volumeRenderingOn);

    return volumeRenderingOn;
  }

  /** \brief Whether the node's lighting and shading properties reach the ray
   *         caster at all.
   *
   * Every one of them is applied inside the front-to-back compositing loop, and
   * only the composite mode runs one. The projection modes reduce each ray to a
   * single value and light nothing, so lighting is inert there rather than
   * merely subtle.
   */
  bool LightingApplies(const mitk::DataNode *node)
  {
    return IsVolumeRenderingOn(node) && mitk::GetVolumeBlendMode(node) == mitk::VolumeBlendMode::Composite;
  }

  /** The arrow is the only cue that a section folds away, so it is drawn by
   * the style from arrowType rather than taken from a pixmap: the toolbar
   * extension chevron the collapsible widgets reach for is deliberately tight
   * and reads as decoration rather than as a control.
   */
  void SetSectionExpanded(QToolButton *header, QWidget *panel, bool expanded)
  {
    header->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    panel->setVisible(expanded);
  }

  /** \brief Labels the rendering toggle with what pressing it does.
   *
   * The leading spaces widen the gap to the icon: Qt draws a button's label
   * four pixels from it and offers no way to ask for more.
   */
  void SetRenderingButtonText(QPushButton *button, bool on)
  {
    button->setText(on
      ? QStringLiteral("  Disable volume rendering")
      : QStringLiteral("  Enable volume rendering"));
  }

  /** mitk::VolumeMapperVtkSmart3D branches on this property before it reads
   * "TransferFunction", and for a mask it builds a flat colour from the node
   * colour instead. Every transfer function control is inert on such a node,
   * so the view has to ask the same question the mapper asks.
   */
  bool IsBinaryImage(const mitk::DataNode *node)
  {
    bool isBinary = false;
    node->GetBoolProperty("binary", isBinary);

    return isBinary;
  }

}

QmitkVolumeVisualizationV2View::QmitkVolumeVisualizationV2View()
{
  m_Controls = std::make_unique<Ui::QmitkVolumeVisualizationV2View>();
}

QmitkVolumeVisualizationV2View::~QmitkVolumeVisualizationV2View() = default;

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

  m_Controls->enableRenderingButton->setIcon(
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/volumevisualization_v2/volume_visualization.svg")));

  m_Controls->binaryHintLabel->setText(
    "Binary image: its appearance is set by the node colour, not by a transfer function.");
  m_Controls->binaryHintLabel->setVisible(false);

  connect(m_Controls->volumeSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
      this, &QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged);

  connect(m_Controls->enableRenderingButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnToggleRendering);

  // The editor writes the node itself, including switching rendering on when a
  // function is loaded, so the view has to re-read rather than only re-render.
  connect(m_Controls->transferFunctionEditor, &QmitkVolumeTransferFunctionEditor::TransferFunctionChanged,
    this, &QmitkVolumeVisualizationV2View::OnTransferFunctionChanged);

  // Lighting Option Controls

  // The application stylesheet gives a checked button an accent border without
  // guarding it on enabled, so a section left folded out stays fully marked
  // while it is greyed out. Translucent grey rather than a fixed colour: it has
  // to hold over a dark and a light background alike, and the theme exposes
  // nothing but its icon colours to ask for.
  m_Controls->lightingExpandButton->setStyleSheet(
    "QToolButton:checked:disabled { border: 1px solid rgba(127, 127, 127, 90); }");

  connect(m_Controls->lightingExpandButton, &QToolButton::toggled, this,
    [this](bool expanded)
    {
      SetSectionExpanded(m_Controls->lightingExpandButton, m_Controls->lightingWidget, expanded);
    });

  // The widget writes the node itself; what it cannot do is reach a renderer, so
  // re-deriving the light rig is what the view contributes here.
  connect(m_Controls->lightingWidget, &QmitkVolumeLightingWidget::LightingChanged,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);

  // The part open at this point gets no RenderWindowPartActivated of its own, so
  // its menu has to be picked up here.
  this->ConnectLightingMode();

  // Before UpdateInterface, which lets the volumes set the rig: run after it,
  // they would already have undone a rig chosen while this view was closed.
  this->AdoptWindowLightingMode();

  // Auto-selection reports only a selection it actually made, so on an empty
  // data storage nothing would ever bring the panel out of the state the .ui
  // file left it in, which is every section enabled and acting on no node.
  this->UpdateInterface();

  m_Controls->volumeSelectionWidget->SetAutoSelectNewNodes(true);
}

void QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_SelectedNode = nullptr;

  if (!nodes.empty() && nodes.front().IsNotNull() && nodes.front()->GetDataAs<mitk::Image>() != nullptr)
    m_SelectedNode = nodes.front();

  // Only here, and deliberately not from UpdateInterface: binding is where the
  // editor decides whether the node's existing function is one to adopt, and
  // re-deciding that right after rendering is switched on would take over the
  // mapper's registered default instead of leaving room for a preset.
  m_Controls->transferFunctionEditor->SetDataNode(m_SelectedNode.Lock().GetPointer());

  this->UpdateInterface();
}

void QmitkVolumeVisualizationV2View::OnToggleRendering()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  // The node is what says which way the toggle currently sits: the button
  // carries no checked state of its own, and other paths - the Properties view,
  // or the v1 view - switch rendering on without going through here.
  const bool state = !IsVolumeRenderingOn(selectedNode.GetPointer());

  selectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(state));

  if (state)
  {
    // The mapper's registered defaults predate this view: its transfer function
    // is one no preset names, and its material values describe no lighting
    // model. Both are taken over the moment rendering is switched on, rather
    // than by changing what the mapper registers for every plugin. Each is a
    // no-op if the node already carries a choice made here.
    m_Controls->transferFunctionEditor->EnsureTransferFunction();

    if (mitk::VolumeRenderingLightingModel::FromNode(selectedNode) == nullptr)
    {
      auto *renderWindow = this->Get3DRenderWindow();
      auto *renderer = renderWindow != nullptr ? renderWindow->GetRenderer() : nullptr;

      // The lighting the window is already on, which is how a second volume
      // joins the first rather than relighting it, and how a rig chosen with
      // nothing rendered yet is honoured rather than overridden.
      auto mode = renderer != nullptr
        ? renderer->GetLightingMode()
        : mitk::VtkPropRenderer::LightingMode::Studio;

      // Except the renderer's default five-light kit: it is the flattest rig and
      // the most expensive to shade a volume with, so a volume never starts
      // there however long the window has been sitting on it.
      if (mode == mitk::VtkPropRenderer::LightingMode::Studio)
        mode = mitk::VtkPropRenderer::LightingMode::Headlight;

      if (const auto *model = mitk::VolumeRenderingLightingModel::FromLightingMode(mode); model != nullptr)
        model->ApplyTo(selectedNode);
    }
  }

  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

QmitkRenderWindow *QmitkVolumeVisualizationV2View::Get3DRenderWindow() const
{
  auto *renderWindowPart = this->GetRenderWindowPart();

  return renderWindowPart != nullptr
    ? renderWindowPart->GetQmitkRenderWindow("3d")
    : nullptr;
}

std::vector<mitk::DataNode *> QmitkVolumeVisualizationV2View::GetRenderedVolumes() const
{
  std::vector<mitk::DataNode *> volumes;

  auto *renderWindow = this->Get3DRenderWindow();

  if (renderWindow == nullptr)
    return volumes;

  const auto *renderer = renderWindow->GetRenderer();

  auto nodes = this->GetDataStorage()->GetSubset(
    mitk::NodePredicateProperty::New("volumerendering", mitk::BoolProperty::New(true)));

  for (auto node : *nodes)
  {
    // Visibility is per renderer, so a volume hidden in this window lights
    // nothing here however it is configured.
    if (node.IsNotNull() && node->IsVisible(renderer))
      volumes.push_back(node.GetPointer());
  }

  return volumes;
}

void QmitkVolumeVisualizationV2View::ConnectLightingMode()
{
  QObject::disconnect(m_LightingModeConnection);

  auto *renderWindow = this->Get3DRenderWindow();

  if (renderWindow == nullptr)
    return;

  m_LightingModeConnection = connect(renderWindow, &QmitkRenderWindow::LightingModeChanged,
    this, &QmitkVolumeVisualizationV2View::OnRenderWindowLightingModeChanged);
}

void QmitkVolumeVisualizationV2View::ApplyLightingMode(mitk::VtkPropRenderer::LightingMode mode)
{
  auto *renderWindow = this->Get3DRenderWindow();

  if (renderWindow == nullptr)
    return;

  auto *renderer = renderWindow->GetRenderer();

  if (renderer == nullptr)
    return;

  const auto previousMode = renderer->GetLightingMode();
  renderer->SetLightingMode(mode);

  // Swapping the lights marks the renderer modified but schedules nothing, and
  // the rig changes on paths that alter nothing else - a selection arriving,
  // a view reopening - so the repaint belongs here rather than at the call
  // sites, which would otherwise each have to know they touched the renderer.
  // Asked for only on a real change, so the refreshes that reapply the mode
  // the renderer already carries stay free.
  if (renderer->GetLightingMode() != previousMode)
    this->RequestRenderWindowUpdate();
}

const mitk::VolumeRenderingLightingModel *QmitkVolumeVisualizationV2View::GetRenderedLightingModel() const
{
  for (const auto *volume : this->GetRenderedVolumes())
  {
    // A volume the ray caster does not light has no say: the projection blend
    // modes reduce each ray to one value and run no compositing loop.
    if (!LightingApplies(volume))
      continue;

    if (const auto *model = mitk::VolumeRenderingLightingModel::FromNode(volume); model != nullptr)
      return model;
  }

  return nullptr;
}

void QmitkVolumeVisualizationV2View::UpdateLightingRig()
{
  auto *renderWindow = this->Get3DRenderWindow();

  if (renderWindow == nullptr)
    return;

  const auto *model = this->GetRenderedLightingModel();

  // With nothing lit in this window the rig is the user's to choose, so it falls
  // back to whatever that window's own menu was last set to rather than to a
  // fixed default.
  this->ApplyLightingMode(model != nullptr
    ? model->lightingMode
    : renderWindow->GetPreferredLightingMode());
}

void QmitkVolumeVisualizationV2View::AdoptWindowLightingMode()
{
  auto *renderWindow = this->Get3DRenderWindow();
  auto *renderer = renderWindow != nullptr ? renderWindow->GetRenderer() : nullptr;

  if (renderer == nullptr)
    return;

  const auto mode = renderer->GetLightingMode();

  // Default lighting is where every window starts, so a window on it says
  // nothing about a choice - and it is the rig volumes are steered away from
  // anyway. There the volumes keep deciding, which is also what brings a loaded
  // scene back lit as it was saved.
  if (mode == mitk::VtkPropRenderer::LightingMode::Studio)
    return;

  const auto *model = mitk::VolumeRenderingLightingModel::FromLightingMode(mode);

  if (model == nullptr)
    return;

  for (auto *volume : this->GetRenderedVolumes())
  {
    if (LightingApplies(volume) && mitk::VolumeRenderingLightingModel::FromNode(volume) != model)
      model->ApplyTo(volume);
  }

  // The sliders show the selected node's values, which this may have rewritten.
  this->UpdateLightingSection();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::RenderWindowPartActivated(mitk::IRenderWindowPart *)
{
  // The incoming part brings a renderer in its default rig and a menu this view
  // is not listening to yet. The rig matters beyond the lights: the mapper keeps
  // applying each volume's scattering every render pass, so one left on the
  // default five-light rig renders the muddy, five-times-more-expensive
  // combination the models exist to avoid. The widgets need nothing here - no
  // node changed.
  this->ConnectLightingMode();
  this->UpdateLightingRig();
}

void QmitkVolumeVisualizationV2View::RenderWindowPartDeactivated(mitk::IRenderWindowPart *)
{
  // Required by the interface, and deliberately empty: a part that is closing
  // takes its renderer and rig with it, and one that is merely superseded is
  // replaced by a part configured in RenderWindowPartActivated.
}

void QmitkVolumeVisualizationV2View::OnTransferFunctionChanged()
{
  // A full refresh rather than a repaint: a preset brings the blend mode it was
  // authored for, and the lighting section is gated on that mode.
  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::OnLightingChanged()
{
  // The widget has already written to the node, so the rig it now asks for can
  // simply be re-derived.
  this->UpdateLightingRig();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::OnRenderWindowLightingModeChanged(mitk::VtkPropRenderer::LightingMode mode)
{
  const auto *model = mitk::VolumeRenderingLightingModel::FromLightingMode(mode);

  // A rig no model is tuned for leaves the nodes alone: the menu has installed it
  // either way, and inventing material values to go with it would be a guess.
  if (model == nullptr)
    return;

  // The window carries one rig, so every volume it lights moves onto the model
  // tuned for that rig. One left behind would keep applying material and
  // scattering values chosen for lights that are no longer installed.
  for (auto *volume : this->GetRenderedVolumes())
  {
    if (LightingApplies(volume))
      model->ApplyTo(volume);
  }

  // The sliders show the selected node's values, which this may have rewritten.
  this->UpdateLightingSection();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::UpdateLightingSection()
{
  auto selectedNode = m_SelectedNode.Lock();

  const bool volumeRenderingOn = IsVolumeRenderingOn(selectedNode.GetPointer());

  // Narrower than !LightingApplies: specifically "rendering, but the blend mode
  // lights nothing", which is the only case the header can explain and offer a
  // way out of. With nothing selected both are false, and the header stays plain.
  const bool gatedByBlendMode =
    volumeRenderingOn && mitk::GetVolumeBlendMode(selectedNode.GetPointer()) != mitk::VolumeBlendMode::Composite;

  // Named on the header rather than left to a tooltip: a greyed-out section
  // whose precondition is written on it teaches the constraint, while a mute
  // one just looks broken.
  m_Controls->lightingExpandButton->setText(
    gatedByBlendMode ? "Lighting / shading - Composite only" : "Lighting / shading");
  m_Controls->lightingExpandButton->setToolTip(gatedByBlendMode
    ? QString("The projection modes flatten each ray to one value and light nothing. Apply a preset authored for"
              " composite to shade the volume.")
    : QString());

  const bool lightingApplies = LightingApplies(selectedNode.GetPointer());

  m_Controls->lightingExpandButton->setEnabled(lightingApplies);
  m_Controls->lightingWidget->setEnabled(lightingApplies);

  // Rebinding rather than a separate refresh call: SetDataNode re-reads, so one
  // entry point cannot fall out of step with a changed selection.
  m_Controls->lightingWidget->SetDataNode(selectedNode.GetPointer());
}

void QmitkVolumeVisualizationV2View::UpdateInterface()
{
  auto selectedNode = m_SelectedNode.Lock();
  const bool hasNode = selectedNode.IsNotNull();

  // Above the early return below, so that the sections come back on that path
  // too. The lighting panel follows its header rather than being remembered, so
  // a section left expanded comes back expanded.
  //
  // Everything below the image section acts on a node, so with none selected it
  // is put away rather than greyed out: the panel then asks for the one thing it
  // needs instead of showing a page of controls none of which can be used.
  m_Controls->transferFunctionEditor->setVisible(hasNode);
  m_Controls->lightingExpandButton->setVisible(hasNode);
  m_Controls->lightingWidget->setVisible(hasNode && m_Controls->lightingExpandButton->isChecked());

  // The rig is 3D-render-window state rather than widget state, and no longer
  // follows the selection. A refresh is still where a volume switched on or off
  // shows up, though, which is what it does follow.
  this->UpdateLightingSection();
  this->UpdateLightingRig();

  if (!hasNode)
  {
    m_Controls->binaryHintLabel->setVisible(false);
    m_Controls->blendModeHintLabel->setVisible(false);
    m_Controls->enableRenderingButton->setEnabled(false);
    SetRenderingButtonText(m_Controls->enableRenderingButton, false);

    // Disabled as well as hidden: the editor draws its preset previews when it
    // is enabled again, which is how a hidden one is kept from rendering a
    // catalogue nobody is looking at.
    m_Controls->transferFunctionEditor->setEnabled(false);

    return;
  }

  const bool isBinary = IsBinaryImage(selectedNode.GetPointer());

  m_Controls->binaryHintLabel->setVisible(isBinary);

  const bool volumeRenderingOn = IsVolumeRenderingOn(selectedNode.GetPointer());

  m_Controls->enableRenderingButton->setEnabled(true);

  // Disabling the whole editor rather than its individual controls is what greys
  // its headers and row labels too, so an inactive section reads as inactive.
  // Which of its own controls apply within that is the editor's own business.
  m_Controls->transferFunctionEditor->setEnabled(volumeRenderingOn && !isBinary);

  const auto blendMode = mitk::GetVolumeBlendMode(selectedNode.GetPointer());

  // Shown only away from the default, so the panel carries no weight for the
  // common case while a greyed-out lighting section always has a visible cause.
  // A readout rather than a control: the mode comes with the transfer function,
  // from the preset applied, and is changed only while editing that curve.
  const bool showBlendModeHint = volumeRenderingOn && blendMode != mitk::VolumeBlendMode::Composite;
  m_Controls->blendModeHintLabel->setVisible(showBlendModeHint);

  if (showBlendModeHint)
  {
    const auto *description =
      blendMode.has_value() ? mitk::VolumeBlendModeDescription::FromMode(*blendMode) : nullptr;

    m_Controls->blendModeHintLabel->setText(QString("Blend mode: %1")
      .arg(description != nullptr ? QString::fromStdString(description->label)
                                  : QString("one this view does not offer")));
  }

  SetRenderingButtonText(m_Controls->enableRenderingButton, volumeRenderingOn);
}
