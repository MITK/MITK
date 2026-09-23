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

#include <mitkVolumeRenderingLightingModel.h>
#include <mitkVtkPropRenderer.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <memory>
#include <vector>

class QmitkRenderWindow;

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
  void OnToggleRendering();
  void OnTransferFunctionChanged();
  void OnLightingChanged();

  /** \brief React to a lighting rig picked from the 3D window's own menu.
   *
   * The window carries one rig for everything drawn in it, so the volumes lit by
   * it are moved onto the model tuned for that rig. Without this the lights would
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

  /** \brief The 3D render window of the current part, or nullptr when there is none. */
  QmitkRenderWindow *Get3DRenderWindow() const;

  /** \brief The volume-rendered nodes currently drawn in the 3D window.
   *
   * Not only the selected one: the rig lights everything in that window, so
   * every volume in it has a say in which rig belongs there.
   */
  std::vector<mitk::DataNode *> GetRenderedVolumes() const;

  /** \brief The lighting model the 3D window is currently lit by.
   *
   * The window carries one rig, so the first volume naming a model decides it
   * for all of them.
   *
   * \return The model, or nullptr when no volume is lit there.
   */
  const mitk::VolumeRenderingLightingModel *GetRenderedLightingModel() const;

  /** \brief Listen to the lighting menu of whichever part is current. */
  void ConnectLightingMode();

  /** Lights belong to the renderer, so this is 3D-render-window state rather
   * than node state.
   */
  void ApplyLightingMode(mitk::VtkPropRenderer::LightingMode mode);

  /** \brief Bring the 3D window's light rig into line with what is drawn there.
   *
   * The rig belongs to the window, so it follows the volumes rendered in it and
   * falls back to the rig chosen in that window's own menu when none is.
   * Closing this view restores nothing: the rig stays whatever the window and
   * its volumes last made it. See AdoptWindowLightingMode for what opening the
   * view does when the two disagree.
   */
  void UpdateLightingRig();

  /** \brief Move the lit volumes onto the model for the 3D window's rig, unless
   *         the window is on Default lighting.
   *
   * The menu can switch the rig while this view is closed, and then nothing
   * moves the volumes along with it. Without this, reopening the view would let
   * the volumes put their old rig back and undo the user's choice.
   *
   * Only volumes on another model are moved; one already on it keeps the values
   * tuned for it by hand. Default lighting is left to the volumes to override:
   * every window starts on it, so it does not tell a choice from an untouched
   * window, and volumes are steered away from it anyway.
   */
  void AdoptWindowLightingMode();

  std::unique_ptr<Ui::QmitkVolumeVisualizationV2View> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;

  /** Kept so that a part change can drop the old window's menu before taking up
   * the new one, rather than leaving this view listening to both.
   */
  QMetaObject::Connection m_LightingModeConnection;
};

#endif
