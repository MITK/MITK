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

#ifndef QMITKDATANODESHOWDETAILSACTION_H
#define QMITKDATANODESHOWDETAILSACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

namespace ShowDetailsAction
{
  MITK_QT_APP void Run(const QList<mitk::DataNode::Pointer>& selectedNodes,
                       QWidget* parent = nullptr);
}

class MITK_QT_APP QmitkDataNodeShowDetailsAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeShowDetailsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeShowDetailsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

private:

  QWidget* m_Parent;

};

#endif // QMITKDATANODESHOWDETAILSACTION_H
