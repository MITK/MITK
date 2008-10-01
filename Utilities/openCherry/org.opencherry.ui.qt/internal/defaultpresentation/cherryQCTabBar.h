/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYQCTABBAR_H_
#define CHERRYQCTABBAR_H_

#include <QTabBar>
#include <deque>

#include "../util/cherryAbstractTabItem.h"

namespace cherry {

class QCTabBar : public QTabBar
{
  Q_OBJECT

private:

  // maps the index in the tabbar to the AbstractTabItem
  std::deque<AbstractTabItem*> tabItemList;

protected:

  void tabRemoved(int index);

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

  void setCurrentTab(AbstractTabItem* item);
  AbstractTabItem* getCurrentTab();
};

}

#endif /* CHERRYQCTABBAR_H_ */
