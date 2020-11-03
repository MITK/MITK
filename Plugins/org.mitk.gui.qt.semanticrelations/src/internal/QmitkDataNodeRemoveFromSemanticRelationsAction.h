/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H
#define QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H

#include <org_mitk_gui_qt_semanticrelations_Export.h>

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// qt
#include <QAction>

namespace RemoveFromSemanticRelationsAction
{
  /**
  * @brief The function checks whether the given node is an image or a segmentation and calls the corresponding remove function.
  *        The corresponding remove functions will remove the data node from the semantic relations storage.
  *        If an image is removed, the child segmentation nodes will also be removed from the semantic relations storage.
  *
  * @pre    The given dataStorage has to be valid (!nullptr).
  * @pre    The given dataNode has to be valid (!nullptr).
  *         The function simply returns if the preconditions are not met.
  *
  * @throw  SemanticRelationException re-thrown.
  *
  * @param dataStorage                    The data storage to use to check for the child segmentation nodes of an image.
  * @param dataNode                       The data node to remove.
  */
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(mitk::DataStorage* dataStorage, const mitk::DataNode* dataNode);

  void RemoveImage(mitk::DataStorage* dataStorage, const mitk::DataNode* image);
  void RemoveSegmentation(const mitk::DataNode* segmentation);
}

class MITK_GUI_SEMANTICRELATIONS_EXPORT QmitkDataNodeRemoveFromSemanticRelationsAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;

};

#endif // QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H
