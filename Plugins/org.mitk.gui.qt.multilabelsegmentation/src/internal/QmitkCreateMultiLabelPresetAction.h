/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCreateMultiLabelPresetAction_h
#define QmitkCreateMultiLabelPresetAction_h

#include <mitkIContextMenuAction.h>

class QmitkCreateMultiLabelPresetAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkCreateMultiLabelPresetAction() = default;
  ~QmitkCreateMultiLabelPresetAction() override = default;

  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes) override;
  void SetDataStorage(mitk::DataStorage*) override;
  void SetFunctionality(berry::QtViewPart*) override;
  void SetSmoothed(bool) override;
  void SetDecimated(bool) override;
};

#endif
