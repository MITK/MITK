/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYEMPTYTABFOLDER_H_
#define BERRYEMPTYTABFOLDER_H_

#include "../util/berryAbstractTabFolder.h"

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
  std::vector<AbstractTabItem*> GetItems();

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
