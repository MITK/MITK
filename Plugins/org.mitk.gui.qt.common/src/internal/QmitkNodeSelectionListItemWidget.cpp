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

#include <berryQtStyleManager.h>

#include <QMouseEvent>

#include "QmitkNodeDetailsDialog.h"

QmitkNodeSelectionListItemWidget::QmitkNodeSelectionListItemWidget(QWidget *parent)
  : QWidget(parent)
{
  m_Controls.setupUi(this);

  m_Controls.btnSelect->installEventFilter(this);
  m_Controls.btnSelect->setVisible(true);
  m_Controls.btnSelect->SetNodeInfo("No valid selection");
  m_Controls.btnClear->setVisible(false);

  m_Controls.btnClear->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org.mitk.gui.qt.common/times.svg")));

  connect(m_Controls.btnClear, SIGNAL(clicked(bool)), this, SLOT(OnClearSelection()));
}

QmitkNodeSelectionListItemWidget::~QmitkNodeSelectionListItemWidget()
{
}

const mitk::DataNode* QmitkNodeSelectionListItemWidget::GetSelectedNode() const
{
  return m_Controls.btnSelect->GetSelectedNode();
};

void QmitkNodeSelectionListItemWidget::SetSelectedNode(const mitk::DataNode* node)
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

bool QmitkNodeSelectionListItemWidget::eventFilter(QObject *obj, QEvent *ev)
{
  if (obj == m_Controls.btnSelect)
  {
    if (ev->type() == QEvent::MouseButtonRelease)
    {
      auto mouseEv = dynamic_cast<QMouseEvent*>(ev);
      if (!mouseEv)
      {
        return false;
      }

      if (mouseEv->button() == Qt::RightButton)
      {
        auto selection = this->GetSelectedNode();

        if (selection != nullptr)
        {
          QList<mitk::DataNode::ConstPointer> selectionList({ this->GetSelectedNode() });
          QmitkNodeDetailsDialog infoDialog(selectionList, this);
          infoDialog.exec();
          return true;
        }
      }
    }
  }

  return false;
}
