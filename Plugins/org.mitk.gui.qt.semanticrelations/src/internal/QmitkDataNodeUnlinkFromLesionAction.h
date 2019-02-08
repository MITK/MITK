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

#ifndef QMITKDATANODEUNLINKFROMLESIONACTION_H
#define QMITKDATANODEUNLINKFROMLESIONACTION_H

#include <org_mitk_gui_qt_semanticrelations_Export.h>

// mitk gui qt semanticrelations plugin
#include "QmitkAbstractSemanticRelationsAction.h"

// qt
#include <QAction>

namespace UnlinkFromLesionAction
{
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(const mitk::DataNode* dataNode, mitk::SemanticRelationsIntegration* semanticRelationsIntegration);
}

class MITK_GUI_SEMANTICRELATIONS_EXPORT QmitkDataNodeUnlinkFromLesionAction : public QAction, public QmitkAbstractSemanticRelationsAction
{
  Q_OBJECT

public:

  QmitkDataNodeUnlinkFromLesionAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeUnlinkFromLesionAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeUnlinkFromLesionAction() override;

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

};

#endif // QMITKDATANODEUNLINKFROMLESIONACTION_H
