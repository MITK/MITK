/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLoadMultiLabelPresetAction_h
#define QmitkLoadMultiLabelPresetAction_h

#include <mitkIContextMenuAction.h>

class QmitkLoadMultiLabelPresetAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkLoadMultiLabelPresetAction() = default;
  ~QmitkLoadMultiLabelPresetAction() override = default;

  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes) override;
  void SetDataStorage(mitk::DataStorage*) override;
  void SetFunctionality(berry::QtViewPart*) override;
  void SetSmoothed(bool) override;
  void SetDecimated(bool) override;
};

#endif
