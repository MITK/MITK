/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAutocropLabelSetImageAction_h
#define QmitkAutocropLabelSetImageAction_h

#include <mitkIContextMenuAction.h>
#include <org_mitk_gui_qt_segmentation_Export.h>

class MITK_QT_SEGMENTATION QmitkAutocropLabelSetImageAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkAutocropLabelSetImageAction();
  ~QmitkAutocropLabelSetImageAction() override;

  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes) override;
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart* view) override;
};

#endif
