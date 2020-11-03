/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODEUNLINKFROMLESIONACTION_H
#define QMITKDATANODEUNLINKFROMLESIONACTION_H

#include <org_mitk_gui_qt_semanticrelations_Export.h>

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// qt
#include <QAction>

namespace UnlinkFromLesionAction
{
  MITK_GUI_SEMANTICRELATIONS_EXPORT void Run(const mitk::DataNode* dataNode);
}

class MITK_GUI_SEMANTICRELATIONS_EXPORT QmitkDataNodeUnlinkFromLesionAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeUnlinkFromLesionAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeUnlinkFromLesionAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;

};

#endif // QMITKDATANODEUNLINKFROMLESIONACTION_H
