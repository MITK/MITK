/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITKTHRESHOLDACTION_H
#define QMITKTHRESHOLDACTION_H

#include <org_mitk_gui_qt_multilabelsegmentation_Export.h>

// Parent classes
#include <QObject>
#include <mitkIContextMenuAction.h>

// Data members
#include <mitkDataStorage.h>
#include <mitkToolManager.h>


class MITK_QT_SEGMENTATION QmitkThresholdAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkThresholdAction();
  ~QmitkThresholdAction() override;

  // IContextMenuAction
  void Run(const QList<mitk::DataNode::Pointer> &selectedNodes) override;
  void SetDataStorage(mitk::DataStorage *dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart *functionality) override;

  void OnThresholdingToolManagerToolModified();

private:
  QmitkThresholdAction(const QmitkThresholdAction &);
  QmitkThresholdAction & operator=(const QmitkThresholdAction &);

  mitk::DataNode::Pointer m_SelectedNode;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ThresholdingToolManager;
};

#endif
