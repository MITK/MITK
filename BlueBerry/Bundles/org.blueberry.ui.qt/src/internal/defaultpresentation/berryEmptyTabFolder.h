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

#ifndef BERRYEMPTYTABFOLDER_H_
#define BERRYEMPTYTABFOLDER_H_

#include "internal/util/berryAbstractTabFolder.h"

namespace berry
{

/**
 * Implements the AbstractTabFolder interface, however this object only displays
 * the content of the currently selected part. There are no tabs, no title, no toolbar,
 * etc. There is no means to select a different part, unless it is done programmatically.
 *
 * @since 3.1
 */
class EmptyTabFolder: public AbstractTabFolder
{

private:
  QWidget* control;
  QWidget* childControl;
  //QColor borderColor;

public:

  EmptyTabFolder(QWidget* parent, bool showborder);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#computeSize(int, int)
   */
  QSize ComputeSize(int widthHint, int heightHint);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#add(int, int)
   */
  AbstractTabItem* Add(int index, int flags);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#move(int, int)
   */
  void Move(int from, int to);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getContentParent()
   */
  QWidget* GetContentParent();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setContent(org.blueberry.swt.widgets.Control)
   */
  void SetContent(QWidget* newContent);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getItems()
   */
  QList<AbstractTabItem*> GetItems();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getSelection()
   */
  AbstractTabItem* GetSelection();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setSelection(org.blueberry.ui.internal.presentations.util.AbstractTabItem)
   */
  void SetSelection(AbstractTabItem* toSelect);

  //    void SetToolbar(Control toolbar) {
  //        if (toolbar != null) {
  //            toolbar.setVisible(false);
  //        }
  //    }

  void Layout(bool flushCache);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setSelectedInfo(org.blueberry.ui.internal.presentations.util.PartInfo)
   */
  void SetSelectedInfo(const PartInfo& info);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#enablePaneMenu(boolean)
   */
  void EnablePaneMenu(bool enabled);

  //    /* (non-Javadoc)
  //     * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getToolbarParent()
  //     */
  //    Composite getToolbarParent() {
  //        return control;
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getControl()
   */
  QWidget* GetControl();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getTabArea()
   */
  QRect GetTabArea();
};

}

#endif /* BERRYEMPTYTABFOLDER_H_ */
