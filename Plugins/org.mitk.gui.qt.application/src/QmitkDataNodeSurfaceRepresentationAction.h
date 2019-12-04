/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODESURFACEREPRESENTATIONACTION_H
#define QMITKDATANODESURFACEREPRESENTATIONACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

class MITK_QT_APP QmitkDataNodeSurfaceRepresentationAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeSurfaceRepresentationAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeSurfaceRepresentationAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnMenuAboutShow();
  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;

};

#endif // QMITKDATANODESURFACEREPRESENTATIONACTION_H
