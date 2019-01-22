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

#ifndef QMITKDATANODESETCONTROLPOINTEACTION_H
#define QMITKDATANODESETCONTROLPOINTEACTION_H

// semantic relations module
#include <mitkSemanticRelations.h>

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// qt
#include <QAction.h>

class QmitkDataNodeSetControlPointAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeSetControlPointAction() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  QWidget *m_Parent;
  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;

};

#endif // QMITKDATANODESETCONTROLPOINTEACTION_H
