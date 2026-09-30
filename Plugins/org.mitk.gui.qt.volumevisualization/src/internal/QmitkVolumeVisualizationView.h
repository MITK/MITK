/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeVisualizationView_h
#define QmitkVolumeVisualizationView_h

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

#include <mitkVolumeRenderingLightingModel.h>
#include <mitkVtkPropRenderer.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <memory>
#include <vector>

class QmitkRenderWindow;

namespace Ui
{
  class QmitkVolumeVisualizationView;
}

class QmitkVolumeVisualizationView : public QmitkAbstractView,
                                       public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkVolumeVisualizationView();
  ~QmitkVolumeVisualizationView() override;

  void SetFocus() override;

private Q_SLOTS:
  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void OnToggleRendering();
  void OnTransferFunctionChanged();
  void OnLightingChanged();

  /** \brief React to a lighting rig picked from the 3D window's own menu.
   *
   * The window carries one rig for everything drawn in it, so every lit volume
   * is moved onto the model tuned for that rig - hidden ones included, as
   * showing one again does not reach this view. Without this the lights would
   * change while the material and scattering values chosen for the old ones stay
   * on the nodes.
   */
  void OnRenderWindowLightingModeChanged(mitk::VtkPropRenderer::LightingMode mode);

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

  /** \brief Take over the 3D window of a render window part that has just become
   * available.
   *
   * A part that replaces another one arrives with the default rig and with a
   * menu this view is not listening to, so both have to be picked up again.
   *
   * \param[in] renderWindowPart Unused; the current part is used, as everywhere
   *            else in this view.
   */
  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;

  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;

  /** \brief Re-derive the light rig once a removed volume is actually gone.
   *
   * The data storage announces a removal before carrying it out, so the node
   * would still count as lit if the rig were derived right away.
   */
  void NodeRemoved(const mitk::DataNode *node) override;

  /** \brief Follow the selected node's rendering flag when it is switched
   *         elsewhere, such as in the Properties view.
   *
   * Only a flip of that flag refreshes the panel. This view's own controls
   * write the node continuously while dragged, and a full refresh can write
   * lighting values back to it, so reacting to every change would be both
   * wasteful and self-triggering.
   */
  void NodeChanged(const mitk::DataNode *node) override;

  /** \brief The 3D render window of the current part, or nullptr when there is none. */
  QmitkRenderWindow *Get3DRenderWindow() const;

  /** \brief The volume-rendered nodes currently drawn in the 3D window.
   *
   * These are the volumes that may pick a rig for a window still on Default
   * lighting; a hidden one lights nothing there and so has no say. Moving
   * volumes onto a model is wider, see MoveVolumesOntoLightingModel.
   */
  std::vector<mitk::DataNode *> GetRenderedVolumes() const;

  /** \brief The lighting model of the first volume lit in the 3D window.
   *
   * The window carries one rig, so only one volume can pick it; the window takes
   * this model's rig only while it is still on Default lighting.
   *
   * \return The model, or nullptr when no volume is lit there.
   */
  const mitk::VolumeRenderingLightingModel *GetRenderedLightingModel() const;

  /** \brief Move every lit volume onto the given model, hidden ones included.
   *
   * A volume already on it is left alone, so values tuned there by hand survive.
   *
   * \return Whether any volume was moved, which leaves the lighting controls and
   *         the picture out of date.
   */
  bool MoveVolumesOntoLightingModel(const mitk::VolumeRenderingLightingModel &model);

  /** \brief Give a node that records no lighting model the one for the rig the
   *         3D window is on.
   *
   * A no-op for a node already on a model, so values tuned there by hand
   * survive.
   */
  void EnsureLightingModel(mitk::DataNode *node);

  /** \brief Listen to the lighting menu of whichever part is current. */
  void ConnectLightingMode();

  /** Lights belong to the renderer, so this is 3D-render-window state rather
   * than node state.
   */
  void ApplyLightingMode(mitk::VtkPropRenderer::LightingMode mode);

  /** \brief Bring the 3D window's light rig and the volumes lit by it into line.
   *
   * The rig belongs to the window, so a window on any rig but Default lighting
   * keeps it and every lit volume is moved onto the model for it. Default
   * lighting is where every window starts, so a window on it has not been given
   * a rig yet: there the first lit volume picks one and the others follow. With
   * nothing lit the window returns to the rig last chosen in its own menu.
   *
   * Closing this view restores nothing: the rig stays whatever the window and
   * its volumes last made it.
   */
  void UpdateLightingRig();

  std::unique_ptr<Ui::QmitkVolumeVisualizationView> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;

  /** The rendering state the panel was last built for, which is what
   * NodeChanged compares the node against. The button carries no checked state
   * to ask instead.
   */
  bool m_RenderingShownOn = false;

  /** Kept so that a part change can drop the old window's menu before taking up
   * the new one, rather than leaving this view listening to both.
   */
  QMetaObject::Connection m_LightingModeConnection;
};

#endif
