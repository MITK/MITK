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

  virtual void InitializeAction() override;
};

#endif // QMITKDATASETOPENINACTION_H
