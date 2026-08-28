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

#include <mitkSimpleHistogram.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionPresets.h>
#include <mitkVtkPropRenderer.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <vtkSmartPointer.h>

#include <memory>

class vtkColorTransferFunction;

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
  void OnTransferFunctionPresetSelected(const QString &presetName);
  void OnTechniqueChanged(int index);
  void OnColorWindowChanged();
  void OnResetTransferFunction();
  void OnCanvasOpacityChanged();
  void OnLightingChanged();
  void OnCinematicModeChanged(int index);
  void OnResetLighting();
  void OnCreateUserTransferFunction();
  void OnImportUserTransferFunction();
  void OnCancelTfAdvancedMode();
  void OnSaveUserTransferFunction();

private:
  void CreateQtPartControl(QWidget *parent) override;

  void UpdateInterface();
  void UpdateLightingControls();

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
   * than node state, and every path that leaves cinematic mode has to restore
   * it.
   */
  void ApplyLightingMode(mitk::VtkPropRenderer::LightingMode mode);

  void ApplyCurrentTransferFunction();

  void SnapshotAppliedTransferFunction();
  void ResetAdjustSliders();
  void SetTfAdvancedMode(bool active);

  std::unique_ptr<Ui::QmitkVolumeVisualizationV2View> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
  mitk::TransferFunctionPresets m_Presets;
  mitk::SimpleHistogramCache m_HistogramCache;

  mitk::TransferFunction::Pointer m_AppliedTransferFunction;
  vtkSmartPointer<vtkColorTransferFunction> m_BaseColorFn;
  mitk::TransferFunction::Pointer m_PreEditTransferFunction;
  std::array<double, 2> m_EffectiveRange { 0.0, 0.0 };
  std::array<double, 2> m_DataRange { 0.0, 0.0 };
};

#endif
