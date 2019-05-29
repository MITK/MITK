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
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* dataNode);

  void RemoveImage(mitk::SemanticRelationsIntegration* semanticRelationsIntegration, const mitk::DataNode* image);
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
