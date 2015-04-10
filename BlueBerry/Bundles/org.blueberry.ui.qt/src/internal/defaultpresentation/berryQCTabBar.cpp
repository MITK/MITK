/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQCTabBar.h"

#include <QApplication>
#include <QMouseEvent>

namespace berry
{

QCTabBar::QCTabBar(QWidget* parent) :
  QTabBar(parent)
{

}

QCTabBar::~QCTabBar()
{
  qDeleteAll(tabItemList);
}

void QCTabBar::tabRemoved(int index)
{
  if (index >= 0 && index < tabItemList.size())
  {
    delete tabItemList.takeAt(index);
  }
}

void QCTabBar::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
     dragStartPosition = event->globalPos();

  QTabBar::mousePressEvent(event);
}

void QCTabBar::mouseMoveEvent(QMouseEvent* event)
{
  if (!(event->buttons() & Qt::LeftButton))
  {
    QTabBar::mouseMoveEvent(event);
    return;
  }

   if ((event->globalPos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance())
   {
     QTabBar::mouseMoveEvent(event);
     return;
   }

   emit dragStarted(dragStartPosition);
}

AbstractTabItem* QCTabBar::getTab(int index) const
{
  if (index < 0 || index >= tabItemList.size()) return 0;
  return tabItemList[index];
}

QList<AbstractTabItem*> QCTabBar::getTabs() const
{
  return tabItemList;
}

void QCTabBar::insertTab(int index, AbstractTabItem* item)
{
  tabItemList.insert(index, item);
  QTabBar::insertTab(index, QString());
}

void QCTabBar::moveAbstractTab(int from, int to)
{
  AbstractTabItem* item = tabItemList[from];

  if (to >= tabItemList.size()) --to;

  tabItemList.removeAt(from);
  tabItemList.insert(to, item);

  this->moveTab(from, to);
}

void QCTabBar::setCurrentTab(AbstractTabItem* item)
{
  this->setCurrentIndex(tabItemList.indexOf(item));
}

AbstractTabItem* QCTabBar::getCurrentTab()
{
  int index = this->currentIndex();
  return tabItemList[index];
}

}
