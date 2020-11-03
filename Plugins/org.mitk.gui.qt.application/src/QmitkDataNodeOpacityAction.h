/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODEOPACITYACTION_H
#define QMITKDATANODEOPACITYACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QSlider>
#include <QWidgetAction>

class MITK_QT_APP QmitkDataNodeOpacityAction : public QWidgetAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnOpacityChanged(int);
  void OnActionChanged();

protected:

  void InitializeAction() override;

  QSlider* m_OpacitySlider;

};

#endif // QMITKDATANODEOPACITYACTION_H
