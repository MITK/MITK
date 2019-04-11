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

  virtual void InitializeWithDataNode(const mitk::DataNode* dataNode) override;

private Q_SLOTS:

  void OnColorChanged();
  void OnActionChanged();

protected:

  virtual void InitializeAction() override;

private:

  QPushButton* m_ColorButton;

};

#endif // QMITKDATANODECOLORACTION_H
