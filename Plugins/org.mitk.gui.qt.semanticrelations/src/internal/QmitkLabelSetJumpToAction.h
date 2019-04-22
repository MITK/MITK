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

#ifndef QMITKLABELSETJUMPTOACTION_H
#define QMITKLABELSETJUMPTOACTION_H

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

namespace LabelSetJumpToAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite,
           const mitk::DataNode* dataNode,
           mitk::BaseRenderer* baseRenderer = nullptr);
}

class QmitkLabelSetJumpToAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkLabelSetJumpToAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkLabelSetJumpToAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;
};

#endif // QMITKLABELSETJUMPTOACTION_H
