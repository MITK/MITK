/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewNavigatorView.h"
#include "QmitkViewNavigatorWidget.h"

// BlueBerry
#include <berryIWorkbenchPartSite.h>

#include <QVBoxLayout>

const std::string QmitkViewNavigatorView::VIEW_ID = "org.mitk.views.viewnavigator";

void QmitkViewNavigatorView::SetFocus()
{
  m_ViewNavigatorWidget->SetFocus();
}

void QmitkViewNavigatorView::CreateQtPartControl(QWidget* parent)
{
  parent->setLayout(new QVBoxLayout);
  parent->layout()->setContentsMargins(0, 0, 0, 0);
  m_ViewNavigatorWidget = new QmitkViewNavigatorWidget(this->GetSite()->GetWorkbenchWindow(), parent);
  parent->layout()->addWidget(m_ViewNavigatorWidget);
}
