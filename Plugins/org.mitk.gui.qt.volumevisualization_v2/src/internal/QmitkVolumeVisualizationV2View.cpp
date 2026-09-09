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

  /** \brief Labels the rendering toggle with the state it is in.
   *
   * A checked button is a subtle cue next to a checkbox's tick, and the two
   * words are what say which way the toggle currently sits.
   *
   * The leading spaces widen the gap to the icon: Qt draws a button's label
   * four pixels from it and offers no way to ask for more.
   */
  void SetRenderingButtonText(QPushButton *button, bool on)
  {
    button->setText(on
      ? QStringLiteral("  Volume rendering: on")
      : QStringLiteral("  Volume rendering: off"));
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

  m_Controls->enableRenderingButton->setIcon(
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/volumevisualization_v2/volume_visualization.svg")));

  m_Controls->lightingWidget->setVisible(false);

  m_Controls->binaryHintLabel->setText(
    "Binary image: its appearance is set by the node colour, not by a transfer function.");
  m_Controls->binaryHintLabel->setVisible(false);

  connect(m_Controls->volumeSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
      this, &QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged);

  connect(m_Controls->enableRenderingButton, &QPushButton::toggled,
    this, &QmitkVolumeVisualizationV2View::OnEnabledRendering);

  // The editor writes the node itself, including switching rendering on when a
  // function is loaded, so the view has to re-read rather than only re-render.
  connect(m_Controls->transferFunctionEditor, &QmitkVolumeTransferFunctionEditor::TransferFunctionChanged,
    this, &QmitkVolumeVisualizationV2View::OnTransferFunctionChanged);

  // Authoring a curve by hand wants the room, and lighting is a rendering
  // control rather than part of authoring one.
  connect(m_Controls->transferFunctionEditor, &QmitkVolumeTransferFunctionEditor::CustomModeChanged, this,
    [this](bool active)
    {
      if (active)
        m_Controls->lightingExpandButton->setChecked(false);
    });

  // Lighting Option Controls
  connect(m_Controls->lightingExpandButton, &QToolButton::toggled, this,
    [this](bool expanded)
    {
      SetSectionExpanded(m_Controls->lightingExpandButton, m_Controls->lightingWidget, expanded);
    });

  // The widget writes the node itself; what it cannot do is reach a renderer, so
  // re-deriving the light rig is what the view contributes here.
  connect(m_Controls->lightingWidget, &QmitkVolumeLightingWidget::LightingChanged,
    this, &QmitkVolumeVisualizationV2View::OnLightingChanged);

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

void QmitkVolumeVisualizationV2View::OnEnabledRendering(bool state)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  selectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(state));

  if (state)
  {
    // The mapper's registered defaults predate this view: its transfer function
    // is one no preset names, and its material values describe no lighting
    // model. Both are taken over the moment rendering is switched on, rather
    // than by changing what the mapper registers for every plugin. Each is a
    // no-op if the node already carries a choice made here.
    m_Controls->transferFunctionEditor->EnsureTransferFunction();

    const auto &models = mitk::VolumeRenderingLightingModel::GetAllModels();

    if (!models.empty() && mitk::VolumeRenderingLightingModel::FromNode(selectedNode) == nullptr)
      models.front().ApplyTo(selectedNode);
  }

  this->UpdateInterface();
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

void QmitkVolumeVisualizationV2View::ApplyLightingModeFromNode()
{
  auto selectedNode = m_SelectedNode.Lock();

  // Null for a node that names no model - an older scene, or a volume configured
  // outside this view.
  const auto *model = LightingApplies(selectedNode.GetPointer())
                        ? mitk::VolumeRenderingLightingModel::FromNode(selectedNode.GetPointer())
                        : nullptr;

  // Nothing lit needs no directional rig, and neither does a configuration no
  // model describes - guessing which directional rig was meant would be worse
  // than the even one. Leaving a directional rig installed would also still
  // relight every surface sharing the window, for no volume benefit.
  this->ApplyLightingMode(model != nullptr
    ? model->lightingMode
    : mitk::VtkPropRenderer::LightingMode::Studio);
}

void QmitkVolumeVisualizationV2View::RenderWindowPartActivated(mitk::IRenderWindowPart *)
{
  // The incoming part brings a renderer in its default rig, while the node still
  // asks for whichever model was last chosen. It matters beyond the lights: the
  // mapper keeps applying the node's scattering every render pass, so a
  // cinematic node left on the default five-light rig renders the muddy,
  // five-times-more-expensive combination the models exist to avoid. The widgets
  // need nothing here - the node did not change.
  this->ApplyLightingModeFromNode();
}

void QmitkVolumeVisualizationV2View::RenderWindowPartDeactivated(mitk::IRenderWindowPart *)
{
  // Required by the interface, and deliberately empty: a part that is closing
  // takes its renderer and rig with it, and one that is merely superseded is
  // replaced by a part configured in RenderWindowPartActivated.
}

void QmitkVolumeVisualizationV2View::OnTransferFunctionChanged()
{
  // A full refresh rather than a repaint: loading a function switches volume
  // rendering on, so the checkbox and everything gated on it have to catch up.
  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::OnLightingChanged()
{
  // The widget has already written to the node, so the rig it now asks for can
  // simply be re-derived from there.
  this->ApplyLightingModeFromNode();
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
              " composite, or set Blend mode to Composite while creating a custom transfer function, to shade"
              " the volume.")
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

  // The rig is 3D-render-window state rather than widget state, but it follows
  // the node exactly as the widgets do, so both belong to a full refresh.
  this->UpdateLightingSection();
  this->ApplyLightingModeFromNode();

  if(selectedNode.IsNull())
  {
    m_Controls->binaryHintLabel->setVisible(false);
    m_Controls->blendModeHintLabel->setVisible(false);
    m_Controls->enableRenderingButton->setChecked(false);
    m_Controls->enableRenderingButton->setEnabled(false);
    SetRenderingButtonText(m_Controls->enableRenderingButton, false);
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
  // A readout rather than a control: the mode comes with the transfer function
  // now, from the preset applied or from the authoring panel.
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

  const QSignalBlocker blocker(m_Controls->enableRenderingButton);
  m_Controls->enableRenderingButton->setChecked(volumeRenderingOn);
  SetRenderingButtonText(m_Controls->enableRenderingButton, volumeRenderingOn);
}
