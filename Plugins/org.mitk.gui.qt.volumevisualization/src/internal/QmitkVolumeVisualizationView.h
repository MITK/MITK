/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKVOLUMEVISUALIZATIONVIEW_H_
#define QMITKVOLUMEVISUALIZATIONVIEW_H_

#include <QmitkAbstractView.h>

#include <mitkDataNodeSelection.h>

#include <mitkWeakPointer.h>

#include <mitkImage.h>

#include "mitkDataStorage.h"

#include <QmitkDataStorageListModel.h>
#include <QmitkDataStorageComboBox.h>
#include <QmitkTransferFunctionWidget.h>

#include "ui_QmitkVolumeVisualizationViewControls.h"

/**
 * \ingroup org_mitk_gui_qt_volumevisualization_internal
 */
class QmitkVolumeVisualizationView : public QmitkAbstractView
{
  Q_OBJECT

public:

  void SetFocus() override;

  QmitkVolumeVisualizationView();

  ~QmitkVolumeVisualizationView() override;

  void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Invoked when the DataManager selection changed
  ///
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes) override;

  static const std::string VIEW_ID;


protected slots:

  void OnMitkInternalPreset( int mode );

  void OnEnableRendering( bool state );
  void OnRenderMode( int mode );
  void OnBlendMode(int mode);

protected:

  Ui::QmitkVolumeVisualizationViewControls* m_Controls;

private:

  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;

  void UpdateInterface();
  void NodeRemoved(const mitk::DataNode* node) override;

};

#endif /*QMITKVOLUMEVISUALIZATIONVIEW_H_*/
