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

// semantic relations module
#include <mitkSemanticRelations.h>

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// qt
#include <QAction.h>

namespace RemoveFromSemanticRelationsAction
{
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* dataNode);

  void RemoveImage(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* image);
  void RemoveSegmentation(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* segmentation);
}

class MITK_GUI_SEMANTICRELATIONS_EXPORT QmitkDataNodeRemoveFromSemanticRelationsAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeRemoveFromSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeRemoveFromSemanticRelationsAction() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;

};

#endif // QMITKDATANODEREMOVEFROMSEMANTICRELATIONSACTION_H
