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

#ifndef QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H
#define QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H

#include <org_mitk_gui_qt_semanticrelations_Export.h>

// mitk gui qt application plugin
#include "QmitkAbstractSemanticRelationsAction.h"

// qt
#include <QAction>

namespace RemoveFromSemanticRelationsAction
{
  /**
  * @brief The function checks whether the given node is an image or a segmentation and calls the corresponding remove function.
  *        The corresponding remove functions will remove the data node from the semantic relations storage.
  *        If an image is removed, the child segmentation nodes will also be removed from the semantic relations storage.
  *
  * @pre    The given semantic relations integration has to be valid (!nullptr).
  * @pre    The given dataStorage has to be valid (!nullptr).
  * @pre    The given dataNode has to be valid (!nullptr).
  *         The function simply returns if the preconditions are not met.
  *
  * @throw  SemanticRelationException re-thrown.
  *
  * @param semanticRelationsIntegration   An instance of the semantic relations integration that should be used to perform the node removal.
  * @param dataStorage                    The data storage to use to check for the child segmentation nodes of an image.
  * @param dataNode                       The data node to remove.
  */
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, mitk::DataStorage* dataStorage, const mitk::DataNode* dataNode);

  void RemoveImage(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, mitk::DataStorage* dataStorage, const mitk::DataNode* image);
  void RemoveSegmentation(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* segmentation);
}

class MITK_GUI_SEMANTICRELATIONS_EXPORT QmitkDataNodeRemoveFromSemanticRelationsAction : public QAction, public QmitkAbstractSemanticRelationsAction
{
  Q_OBJECT

public:

  QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeRemoveFromSemanticRelationsAction() override;

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

};

#endif // QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H
