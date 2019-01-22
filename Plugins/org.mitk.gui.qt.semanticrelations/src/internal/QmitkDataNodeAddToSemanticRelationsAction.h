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

#ifndef QMITKDATANODEADDTOSEMANTICRELATIONSACTION_H
#define QMITKDATANODEADDTOSEMANTICRELATIONSACTION_H

#include <org_mitk_gui_qt_semanticrelations_Export.h>

// semantic relations module
#include <mitkSemanticRelations.h>

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// qt
#include <QAction.h>

namespace AddToSemanticRelationsAction
{
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(mitk::SemanticRelations* semanticRelations, const mitk::DataStorage* dataStorage, const mitk::DataNode* image);
  
  void AddImage(mitk::SemanticRelations* semanticRelations, const mitk::DataNode* image);
  void AddSegmentation(mitk::SemanticRelations* semanticRelations, const mitk::DataStorage* dataStorage, const mitk::DataNode* segmentation);
}

class MITK_GUI_SEMANTICRELATIONS_EXPORT QmitkDataNodeAddToSemanticRelationsAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeAddToSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeAddToSemanticRelationsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeAddToSemanticRelationsAction() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;

};

#endif // QMITKDATANODEADDTOSEMANTICRELATIONSACTION_H
