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
#ifndef QMITKTHRESHOLDACTION_H
#define QMITKTHRESHOLDACTION_H

#include <org_mitk_gui_qt_segmentation_Export.h>

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
  ~QmitkThresholdAction();

  // IContextMenuAction
  void Run(const QList<mitk::DataNode::Pointer> &selectedNodes);
  void SetDataStorage(mitk::DataStorage *dataStorage);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart *functionality);

  void OnThresholdingToolManagerToolModified();

private:
  QmitkThresholdAction(const QmitkThresholdAction &);
  QmitkThresholdAction & operator=(const QmitkThresholdAction &);

  mitk::DataNode::Pointer m_SelectedNode;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ThresholdingToolManager;
};

#endif
