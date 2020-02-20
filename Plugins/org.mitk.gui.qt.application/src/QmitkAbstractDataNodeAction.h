/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKABSTRACTDATANODEACTION_H
#define QMITKABSTRACTDATANODEACTION_H

#include <org_mitk_gui_qt_application_Export.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// berry
#include <berryIWorkbenchPartSite.h>

// qt
#include <QList>

namespace AbstractDataNodeAction
{
  MITK_QT_APP QList<mitk::DataNode::Pointer> GetSelectedNodes(berry::IWorkbenchPartSite::Pointer workbenchPartSite);
}

class MITK_QT_APP QmitkAbstractDataNodeAction
{

public:

  QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite* workbenchPartSite);

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetSelectedNodes(const QList<mitk::DataNode::Pointer>& selectedNodes);
  void SetBaseRenderer(mitk::BaseRenderer* baseRenderer);

  virtual void InitializeWithDataNode(const mitk::DataNode*) { }

protected:

  virtual void InitializeAction() = 0;

  /**
  * @brief Grants access to the base renderer stored for the action.
  *        Will return nullptr if renderer was never set or has become invalid
  */
  mitk::BaseRenderer::Pointer GetBaseRenderer();

  mitk::DataNode::Pointer GetSelectedNode() const;
  QList<mitk::DataNode::Pointer> GetSelectedNodes() const;

  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  QList<mitk::DataNode::Pointer> m_SelectedNodes;

private:

  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;

};

#endif // QMITKABSTRACTDATANODEACTION_H
