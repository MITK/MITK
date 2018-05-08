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


#include "QmitkNodeSelectionButton.h"

#include "QPainter"
#include "QTextDocument"

#include "mitkDataNode.h"

QmitkNodeSelectionButton::QmitkNodeSelectionButton(QWidget *parent)
  : QPushButton(parent)
{ }

void QmitkNodeSelectionButton::SetSelectedNode(mitk::DataNode* node)
{
  this->m_SelectedNode = mitk::WeakPointer<mitk::DataNode>(node);
  this->update();
};

void QmitkNodeSelectionButton::SetNodeInfo(QString info)
{
  this->m_Info = info;
  this->update();
};

void QmitkNodeSelectionButton::paintEvent(QPaintEvent *p)
{
  QPushButton::paintEvent(p);

  QPainter painter(this);
  QTextDocument td;
  td.setHtml(m_Info);
  painter.translate(QPoint(5,5));
  td.drawContents(&painter);

}
