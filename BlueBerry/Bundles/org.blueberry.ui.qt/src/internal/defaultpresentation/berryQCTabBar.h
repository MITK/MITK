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


#ifndef BERRYQCTABBAR_H_
#define BERRYQCTABBAR_H_

#include <QTabBar>
#include <deque>

#include "internal/util/berryAbstractTabItem.h"

namespace berry {

class QCTabBar : public QTabBar
{
  Q_OBJECT

private:

  // maps the index in the tabbar to the AbstractTabItem
  std::deque<AbstractTabItem*> tabItemList;

  QPoint dragStartPosition;

protected:

  void tabRemoved(int index);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);

public:

  QCTabBar(QWidget* parent = 0);
  ~QCTabBar();

  AbstractTabItem* getTab(int index) const;

  std::vector<AbstractTabItem*> getTabs() const;

  /**
   * Inserts a new tab at the specified index. The TabBar takes
   * ownership of the AbstractTabItem.
   */
  void insertTab(int index, AbstractTabItem* item);

  void moveAbstractTab(int from, int to);

  void setCurrentTab(AbstractTabItem* item);
  AbstractTabItem* getCurrentTab();

signals:

  void dragStarted(const QPoint& location);
};

}

#endif /* BERRYQCTABBAR_H_ */
