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

#ifndef QMITKDATANODEINFORMATIONTYPEACTION_H
#define QMITKDATANODEINFORMATIONTYPEACTION_H

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

class QmitkDataNodeInformationTypeAction : public QAction
{
  Q_OBJECT

public:

  QmitkDataNodeInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeInformationTypeAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeInformationTypeAction() override;

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

#endif // QMITKDATANODEINFORMATIONTYPEACTION_H
