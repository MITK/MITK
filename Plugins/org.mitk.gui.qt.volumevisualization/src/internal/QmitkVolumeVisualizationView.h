/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

  virtual ~QmitkVolumeVisualizationView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Invoked when the DataManager selection changed
  ///
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes) override;

  static const std::string VIEW_ID;


protected slots:

  void OnMitkInternalPreset( int mode );

  void OnEnableRendering( bool state );
  void OnEnableLOD( bool state );
  void OnRenderMode( int mode );


protected:

  Ui::QmitkVolumeVisualizationViewControls* m_Controls;

private:

  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;

  void UpdateInterface();
  void NodeRemoved(const mitk::DataNode* node) override;

};

#endif /*QMITKVOLUMEVISUALIZATIONVIEW_H_*/
