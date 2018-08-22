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

#ifndef QMITKDATANODECONTROLPOINTEACTION_H
#define QMITKDATANODECONTROLPOINTEACTION_H

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// semantic relations module
#include <mitkSemanticRelations.h>

// berry
#include <berryIWorkbenchPartSite.h>

// qt
#include <QAction.h>

class QmitkDataNodeControlPointAction : public QAction
{
  Q_OBJECT

public:

  QmitkDataNodeControlPointAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeControlPointAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeControlPointAction() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  void InitializeAction();

  QList<mitk::DataNode::Pointer> GetSelectedNodes();
  mitk::DataNode::Pointer GetSelectedNode();

  QWidget* m_Parent;
  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
};

#endif // QMITKDATANODECONTROLPOINTEACTION_H
