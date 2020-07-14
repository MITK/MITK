/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKVOLUMEVISUALIZATIONVIEW_H
#define QMITKVOLUMEVISUALIZATIONVIEW_H

#include "ui_QmitkVolumeVisualizationViewControls.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

#include <QmitkAbstractView.h>

/**
 * @brief
 */
class QmitkVolumeVisualizationView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkVolumeVisualizationView();

  ~QmitkVolumeVisualizationView() override = default;

  void SetFocus() override;

private Q_SLOTS:

  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  void OnMitkInternalPreset(int mode);
  void OnEnableRendering(bool state);
  void OnRenderMode(int mode);
  void OnBlendMode(int mode);

private:

  void CreateQtPartControl(QWidget* parent) override;

  void UpdateInterface();

  Ui::QmitkVolumeVisualizationViewControls* m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;

};

#endif // QMITKVOLUMEVISUALIZATIONVIEW_H
