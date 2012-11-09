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

void QmitkPropertyListView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/,
                                               const QList<mitk::DataNode::Pointer> &nodes)
{
  if (nodes.empty() || (nodes.front().IsNull())) return;

  m_NodePropertiesTableEditor->SetPropertyList(nodes.front()->GetPropertyList());
}

void QmitkPropertyListView::SetFocus()
{
  m_NodePropertiesTableEditor->setFocus();
}
