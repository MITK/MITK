/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODECOLORACTION_H
#define QMITKDATANODECOLORACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QPushButton>
#include <QWidgetAction>

class MITK_QT_APP QmitkDataNodeColorAction : public QWidgetAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeColorAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeColorAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnColorChanged();
  void OnActionChanged();

protected:

  void InitializeAction() override;

private:

  QPushButton* m_ColorButton;

};

#endif // QMITKDATANODECOLORACTION_H
