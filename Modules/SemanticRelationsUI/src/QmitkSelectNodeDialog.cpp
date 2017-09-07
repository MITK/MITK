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

#include "QmitkSelectNodeDialog.h"

// qt
#include <QPushButton>

QmitkSelectNodeDialog::QmitkSelectNodeDialog(QWidget* parent/* = nullptr*/)
  : QDialog(parent)
  , m_SelectionWidget(nullptr)
  , m_SelectedDataNode(nullptr)
{
  m_VerticalLayout = new QVBoxLayout(this);

  QPushButton* acceptButton = new QPushButton(tr("Ok"), this);
  QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
  acceptButton->setDefault(true);

  m_VerticalLayout->addWidget(acceptButton);
  m_VerticalLayout->addWidget(cancelButton);
  connect(acceptButton, SIGNAL(clicked()), SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

QmitkSelectNodeDialog::~QmitkSelectNodeDialog()
{
  // nothing here
}

void QmitkSelectNodeDialog::SetSelectionWidget(QmitkSelectionWidget* selectionWidget)
{
  m_SelectionWidget = selectionWidget;
  m_VerticalLayout->insertWidget(0, m_SelectionWidget);
  connect(m_SelectionWidget, SIGNAL(SelectionChanged(mitk::DataNode*)), SLOT(OnSelectionWidgetSelectionChanged(mitk::DataNode*)));
}

void QmitkSelectNodeDialog::OnSelectionWidgetSelectionChanged(mitk::DataNode* dataNode)
{
  m_SelectedDataNode = dataNode;
}

mitk::DataNode* QmitkSelectNodeDialog::GetSelectedDataNode() const
{
  return m_SelectedDataNode;
}
