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

  virtual void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnActionChanged();

protected:

  virtual void InitializeAction() override;

private:

  QmitkNumberPropertySlider* m_ComponentSlider;

};

#endif // QMITKDATANODECOMPONENTACTION_H
