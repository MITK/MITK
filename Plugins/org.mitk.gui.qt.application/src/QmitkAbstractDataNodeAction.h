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

#ifndef QMITKABSTRACTDATANODEACTION_H
#define QMITKABSTRACTDATANODEACTION_H

#include <org_mitk_gui_qt_application_Export.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// berry
#include <berryIWorkbenchPartSite.h>

// qt
#include <QList>

class MITK_QT_APP QmitkAbstractDataNodeAction
{

public:

  QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkAbstractDataNodeAction();

  void SetDataStorage(mitk::DataStorage* dataStorage);

  virtual void InitializeWithDataNode(const mitk::DataNode* dataNode) { }

protected:

  virtual void InitializeAction() = 0;
  mitk::DataNode::Pointer GetSelectedNode();
  QList<mitk::DataNode::Pointer> GetSelectedNodes();

  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

};

#endif // QMITKABSTRACTDATANODEACTION_H
