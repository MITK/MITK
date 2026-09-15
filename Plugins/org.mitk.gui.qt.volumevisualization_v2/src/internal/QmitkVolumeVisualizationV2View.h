/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeVisualizationV2View_h
#define QmitkVolumeVisualizationV2View_h

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

#include <mitkVtkPropRenderer.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <memory>

namespace Ui
{
  class QmitkVolumeVisualizationV2View;
}

class QmitkVolumeVisualizationV2View : public QmitkAbstractView,
                                       public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkVolumeVisualizationV2View();
  ~QmitkVolumeVisualizationV2View() override;

  void SetFocus() override;

private Q_SLOTS:
  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void OnEnabledRendering(bool state);
  void OnTransferFunctionChanged();
  void OnLightingChanged();
  void OnCustomModeChanged(bool active);

private:
  void CreateQtPartControl(QWidget *parent) override;

  void UpdateInterface();

  /** \brief Refresh the collapsible lighting section: its header, whether it
   *         applies at all, and the node the controls inside it act on.
   *
   * The controls themselves belong to QmitkVolumeLightingWidget. What stays here
   * is the part that depends on the blend mode - which arrives with the transfer
   * function - and so cannot be the widget's to decide.
   */
  void UpdateLightingSection();

  /** \brief Put the panel back at its top.
   *
   * The panel is taller than the room it gets and sits in a scroll area it
   * neither owns nor is told about, so entering or leaving a mode that changes
   * which sections exist would otherwise leave the view scrolled to an offset
   * measured against sections that are no longer there.
   */
  void ScrollToTop();

  /** \brief Install the light rig on a render window part that has just become
   * available.
   *
   * The rig belongs to the renderer while the model that decides it belongs to
   * the node, so a part that replaces another one arrives with the default rig
   * and no knowledge of the node this view has selected. Without this the view
   * would keep naming a model the new renderer is not in.
   *
   * \param[in] renderWindowPart Unused; the rig is installed on whichever part
   *            is current, as everywhere else in this view.
   */
  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;

  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;

  /** Lights belong to the renderer, so this is 3D-render-window state rather
   * than node state, and every path that stops asking for a directional rig
   * has to restore the default - the view's own destructor included.
   */
  void ApplyLightingMode(mitk::VtkPropRenderer::LightingMode mode);

  /** \brief Install the rig the currently selected node asks for.
   *
   * The rig lives on the renderer while the model that decides it lives on the
   * node, so the two only agree if something reconciles them. This is that
   * something, kept apart from UpdateLightingControls so that refreshing the
   * widgets does not silently reconfigure a renderer, and so that the paths
   * which genuinely need a rig change - a new node, a new model, a new render
   * window part - say so at the call site.
   */
  void ApplyLightingModeFromNode();

  std::unique_ptr<Ui::QmitkVolumeVisualizationV2View> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;

  /** \brief Whether the editor has taken the panel over to author a curve.
   *
   * Kept here because UpdateInterface decides what the panel shows and runs
   * again while authoring - the blend mode control changes the transfer
   * function, which the view answers with a full refresh. Were the sections
   * hidden where the mode changes instead, that refresh would bring them back.
   */
  bool m_CustomModeActive = false;
};

#endif
