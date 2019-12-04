/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  QSize ComputeSize(int widthHint, int heightHint) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#add(int, int)
   */
  AbstractTabItem* Add(int index, int flags) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#move(int, int)
   */
  void Move(int from, int to) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getContentParent()
   */
  QWidget* GetContentParent() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setContent(org.blueberry.swt.widgets.Control)
   */
  void SetContent(QWidget* newContent) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getItems()
   */
  QList<AbstractTabItem*> GetItems() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getSelection()
   */
  AbstractTabItem* GetSelection() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setSelection(org.blueberry.ui.internal.presentations.util.AbstractTabItem)
   */
  void SetSelection(AbstractTabItem* toSelect) override;

  //    void SetToolbar(Control toolbar) {
  //        if (toolbar != null) {
  //            toolbar.setVisible(false);
  //        }
  //    }

  void Layout(bool flushCache) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setSelectedInfo(org.blueberry.ui.internal.presentations.util.PartInfo)
   */
  void SetSelectedInfo(const PartInfo& info) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#enablePaneMenu(boolean)
   */
  void EnablePaneMenu(bool enabled) override;

  //    /* (non-Javadoc)
  //     * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getToolbarParent()
  //     */
  //    Composite getToolbarParent() {
  //        return control;
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getControl()
   */
  QWidget* GetControl() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getTabArea()
   */
  QRect GetTabArea() override;
};

}

#endif /* BERRYEMPTYTABFOLDER_H_ */
