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

  virtual void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnOpacityChanged(int);
  void OnActionChanged();

protected:

  virtual void InitializeAction() override;

  QSlider* m_OpacitySlider;

};

#endif // QMITKDATANODEOPACITYACTION_H
