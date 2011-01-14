/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-09-07 13:46:44 +0200 (Di, 07 Sep 2010) $
Version:   $Revision: 25948 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPythonCommandHistoryTreeWidget.h"

#include <QDrag>
#include <QMimeData>
#include <QList>
#include <QMouseEvent>
#include <QUrl>


QmitkPythonCommandHistoryTreeWidget::QmitkPythonCommandHistoryTreeWidget(QWidget *parent)
:QTreeWidget(parent)
{
}

QmitkPythonCommandHistoryTreeWidget::~QmitkPythonCommandHistoryTreeWidget()
{
}

void QmitkPythonCommandHistoryTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
  // if not left button - return
  if (!(event->buttons() & Qt::LeftButton)) return;

  // if no item selected, return (else it would crash)
  if (selectedItems().isEmpty()) return;

  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  //QList<QUrl> list;
  QList<QTreeWidgetItem *> listItems = selectedItems();

  for(int i = 0; i < listItems.size(); i++)
  {
    //list.append(QUrl(listItems[i]->text(0)));
    mimeData->setText(listItems[i]->text(0));
  }
  //mimeData->setUrls(list);
  drag->setMimeData(mimeData);

  // start drag
  drag->start(Qt::CopyAction | Qt::MoveAction);
}
