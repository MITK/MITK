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

#include <mitkTransferFunctionPresets.h>

#include <QmitkAbstractView.h>

#include <memory>

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

private:
  void CreateQtPartControl(QWidget *parent) override;

  void UpdateInterface();

  std::unique_ptr<Ui::QmitkVolumeVisualizationV2View> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
  mitk::TransferFunctionPresets m_Presets;
};

#endif
