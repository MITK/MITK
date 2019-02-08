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

#ifndef QMITKDATANODESETCONTROLPOINTACTION_H
#define QMITKDATANODESETCONTROLPOINTACTION_H

// mitk gui qt semanticrelations plugin
#include "QmitkAbstractSemanticRelationsAction.h"

// qt
#include <QAction>

class QmitkDataNodeSetControlPointAction : public QAction, public QmitkAbstractSemanticRelationsAction
{
  Q_OBJECT

public:

  QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeSetControlPointAction() override;

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  QWidget* m_Parent;

};

#endif // QMITKDATANODESETCONTROLPOINTACTION_H
