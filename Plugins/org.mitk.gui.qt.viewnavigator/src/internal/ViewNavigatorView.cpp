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
