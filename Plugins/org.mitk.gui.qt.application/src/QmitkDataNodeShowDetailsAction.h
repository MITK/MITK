/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  void InitializeAction() override;

private:

  QWidget* m_Parent;

};

#endif // QMITKDATANODESHOWDETAILSACTION_H
