/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkAssignDistinctColorsAction_h
#define QmitkAssignDistinctColorsAction_h

#include <mitkIContextMenuAction.h>

#include <org_mitk_gui_qt_segmentation_Export.h>

class MITK_QT_SEGMENTATION QmitkAssignDistinctColorsAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkAssignDistinctColorsAction();
  ~QmitkAssignDistinctColorsAction() override;

  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes) override;
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetFunctionality(berry::QtViewPart* view) override;
};

#endif
