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

#include <QmitkAbstractView.h>

#include <vtkSmartPointer.h>

#include <memory>

class vtkColorTransferFunction;

namespace Ui
{
  class QmitkVolumeVisualizationV2View;
}

class QmitkVolumeVisualizationV2View : public QmitkAbstractView
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
  void OnColorWindowChanged();
  void OnResetTransferFunction();
  void OnCanvasOpacityChanged();
  void OnLightingChanged();
  void OnResetLighting();
  void OnCreateUserTransferFunction();
  void OnImportUserTransferFunction();
  void OnCancelTfAdvancedMode();
  void OnSaveUserTransferFunction();

private:
  void CreateQtPartControl(QWidget *parent) override;

  void UpdateInterface();
  void UpdateLightingControls();

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
