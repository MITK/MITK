/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODECOMPONENTACTION_H
#define QMITKDATANODECOMPONENTACTION_H

#include <org_mitk_gui_qt_application_Export.h>

// qt widgets ext module
#include <QmitkNumberPropertySlider.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QWidgetAction>

class MITK_QT_APP QmitkDataNodeComponentAction : public QWidgetAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeComponentAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeComponentAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnActionChanged();

protected:

  void InitializeAction() override;

private:

  QmitkNumberPropertySlider* m_ComponentSlider;

};

#endif // QMITKDATANODECOMPONENTACTION_H
