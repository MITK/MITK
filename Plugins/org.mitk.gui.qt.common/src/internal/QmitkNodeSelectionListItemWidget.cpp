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


#include "QmitkNodeSelectionListItemWidget.h"

// berry includes
#include <berryQtStyleManager.h>

QmitkNodeSelectionListItemWidget::QmitkNodeSelectionListItemWidget(QWidget *parent)
  : QWidget(parent)
{
  m_Controls.setupUi(this);

  m_Controls.btnSelect->setVisible(true);
  m_Controls.btnSelect->SetNodeInfo("No valid selection");
  m_Controls.btnClear->setVisible(false);

  m_Controls.btnClear->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org.mitk.gui.qt.common/times.svg")));

  connect(m_Controls.btnClear, SIGNAL(clicked(bool)), this, SLOT(OnClearSelection()));
}

QmitkNodeSelectionListItemWidget::~QmitkNodeSelectionListItemWidget()
{
}

mitk::DataNode::Pointer QmitkNodeSelectionListItemWidget::GetSelectedNode() const
{
  return m_Controls.btnSelect->GetSelectedNode();
};

void QmitkNodeSelectionListItemWidget::SetSelectedNode(mitk::DataNode* node)
{
  m_Controls.btnSelect->SetSelectedNode(node);
  this->update();
};

void QmitkNodeSelectionListItemWidget::SetClearAllowed(bool allowed)
{
  m_Controls.btnClear->setVisible(allowed);
};

void QmitkNodeSelectionListItemWidget::OnClearSelection()
{
  emit ClearSelection(this->GetSelectedNode());
};
