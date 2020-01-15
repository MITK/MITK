/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITKCREATEPOLYGONMODELACTION_H
#define QMITKCREATEPOLYGONMODELACTION_H

#include <org_mitk_gui_qt_segmentation_Export.h>

// Parent classes
#include <QObject>
#include <mitkIContextMenuAction.h>

// Data members
#include <mitkDataNode.h>

class MITK_QT_SEGMENTATION QmitkCreatePolygonModelAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkCreatePolygonModelAction();
  ~QmitkCreatePolygonModelAction() override;

  // IContextMenuAction
  void Run(const QList<mitk::DataNode::Pointer> &selectedNodes) override;
  void SetDataStorage(mitk::DataStorage *dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart* view) override;

  void OnSurfaceCalculationDone();

private:
  QmitkCreatePolygonModelAction(const QmitkCreatePolygonModelAction &);
  QmitkCreatePolygonModelAction & operator=(const QmitkCreatePolygonModelAction &);

  mitk::DataStorage::Pointer m_DataStorage;
  bool m_IsSmoothed;
  bool m_IsDecimated;
};

#endif
