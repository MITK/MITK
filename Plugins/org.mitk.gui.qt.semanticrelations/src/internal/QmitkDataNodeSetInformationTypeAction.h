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

#ifndef QMITKDATANODESETINFORMATIONTYPEACTION_H
#define QMITKDATANODESETINFORMATIONTYPEACTION_H

// mitk gui qt application plugin
#include "QmitkAbstractSemanticRelationsAction.h"

// qt
#include <QAction>

class QmitkDataNodeSetInformationTypeAction : public QAction, public QmitkAbstractSemanticRelationsAction
{
  Q_OBJECT

public:

  QmitkDataNodeSetInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeSetInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeSetInformationTypeAction() override;

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  QWidget* m_Parent;

};

#endif // QMITKDATANODESETINFORMATIONTYPEACTION_H
