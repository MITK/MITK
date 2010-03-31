/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <berryISelectionProvider.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>
#include "QmitkPropertyListView.h"
#include <QmitkPropertiesTableEditor.h>
#include <mitkDataNodeObject.h>
#include <mitkLogMacros.h>
#include <QVBoxLayout>

const std::string QmitkPropertyListView::VIEW_ID = "org.mitk.views.propertylistview";

QmitkPropertyListView::QmitkPropertyListView()
{
}

QmitkPropertyListView::~QmitkPropertyListView()
{
}

void QmitkPropertyListView::CreateQtPartControl( QWidget* parent )
{
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor;

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(m_NodePropertiesTableEditor);

  parent->setLayout(layout);
}

void QmitkPropertyListView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if (nodes.empty() || (nodes.front() == NULL)) return;
      
  m_NodePropertiesTableEditor->SetPropertyList(nodes.front()->GetPropertyList());
}

bool QmitkPropertyListView::IsExclusiveFunctionality() const
{
  return false;
}
