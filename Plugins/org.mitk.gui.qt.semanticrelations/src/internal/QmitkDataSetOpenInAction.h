/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATASETOPENINACTION_H
#define QMITKDATASETOPENINACTION_H

#include "QmitkDataNodeOpenInAction.h"

namespace OpenInAction
{
  void Run(mitk::DataStorage::Pointer dataStorage,
           mitk::DataNode::Pointer imageNode,
           mitk::BaseRenderer* renderer = nullptr);
}

class QmitkDataSetOpenInAction : public QmitkDataNodeOpenInAction
{
  Q_OBJECT

public:

  QmitkDataSetOpenInAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataSetOpenInAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnMenuAboutToShow();
  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;
};

#endif // QMITKDATASETOPENINACTION_H
