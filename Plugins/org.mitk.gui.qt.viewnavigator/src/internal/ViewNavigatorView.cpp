/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "ViewNavigatorView.h"

#include "QmitkViewNavigatorWidget.h"

// BlueBerry
#include <berryIWorkbenchPartSite.h>

#include <QVBoxLayout>

const std::string ViewNavigatorView::VIEW_ID = "org.mitk.views.viewnavigatorview";

void ViewNavigatorView::SetFocus()
{
  m_ViewNavigatorWidget->setFocus();
}

void ViewNavigatorView::CreateQtPartControl( QWidget *parent )
{
  parent->setLayout(new QVBoxLayout);
  parent->layout()->setContentsMargins(0, 0, 0, 0);
  m_ViewNavigatorWidget = new QmitkViewNavigatorWidget(this->GetSite()->GetWorkbenchWindow(), parent);
  parent->layout()->addWidget(m_ViewNavigatorWidget);
}
