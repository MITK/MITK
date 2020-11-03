/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQCTABBAR_H_
#define BERRYQCTABBAR_H_

#include <QTabBar>

#include "internal/util/berryAbstractTabItem.h"

namespace berry {

class QCTabBar : public QTabBar
{
  Q_OBJECT

private:

  // maps the index in the tabbar to the AbstractTabItem
  QList<AbstractTabItem*> tabItemList;

  QPoint dragStartPosition;

protected:

  void tabRemoved(int index) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

public:

  QCTabBar(QWidget* parent = nullptr);
  ~QCTabBar() override;

  AbstractTabItem* getTab(int index) const;

  QList<AbstractTabItem*> getTabs() const;

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
