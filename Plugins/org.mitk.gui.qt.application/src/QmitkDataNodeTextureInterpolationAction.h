/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataNodeTextureInterpolationAction_h
#define QmitkDataNodeTextureInterpolationAction_h

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

class MITK_QT_APP QmitkDataNodeTextureInterpolationAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeTextureInterpolationAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeTextureInterpolationAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnActionChanged();
  void OnActionToggled(bool);

protected:

  void InitializeAction() override;

};

#endif
