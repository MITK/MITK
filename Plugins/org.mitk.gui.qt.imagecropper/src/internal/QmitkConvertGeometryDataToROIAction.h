/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkConvertGeometryDataToROIAction_h
#define QmitkConvertGeometryDataToROIAction_h

#include <mitkIContextMenuAction.h>

class QmitkConvertGeometryDataToROIAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkConvertGeometryDataToROIAction();
  ~QmitkConvertGeometryDataToROIAction() override;

  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes) override;
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetFunctionality(berry::QtViewPart* functionality) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;

private:
  mitk::DataStorage::Pointer m_DataStorage;
};

#endif
