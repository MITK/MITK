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

#ifndef QMITKDATANODETOGGLEVISIBILITYACTION_H
#define QMITKDATANODETOGGLEVISIBILITYACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

namespace ToggleVisibilityAction
{
  MITK_QT_APP void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite,
                       mitk::DataStorage::Pointer dataStorage,
                       QList<mitk::DataNode::Pointer> selectedNodes);
}

class MITK_QT_APP QmitkDataNodeToggleVisibilityAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeToggleVisibilityAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeToggleVisibilityAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeToggleVisibilityAction() override;

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

};

#endif // QMITKDATANODETOGGLEVISIBILITYACTION_H
