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

#ifndef CHERRYNATIVETABFOLDER_H_
#define CHERRYNATIVETABFOLDER_H_

#include "../util/cherryAbstractTabFolder.h"

#include <cherryIQtStyleManager.h>

#include <QObject>

class QFrame;
class QWidget;

namespace cherry
{

class QCTabBar;

class NativeTabFolder: public QObject, public AbstractTabFolder
{

Q_OBJECT

private:

  QCTabBar* tabControl;
  QFrame* contentFrame;
  QWidget* viewForm;
  QWidget* content;
  //  ViewForm viewForm;
  //  StandardSystemToolbar systemToolbar;
  //  CLabel title;
  //    ProxyControl topCenter;
  //    SizeCache topCenterCache;

  //    IPropertyListener systemToolbarListener = new IPropertyListener() {
  //
  //        public void propertyChanged(Object source, int propId) {
  //            Point location;
  //
  //            if (propId == TabFolderEvent.EVENT_PANE_MENU) {
  //                location = getPaneMenuLocation();
  //            } else {
  //                location = new Point(0,0);
  //            }
  //
  //            fireEvent(propId, getSelection(), location);
  //        }
  //
  //    };

  IQtStyleManager::Pointer skinManager;

  /**
   * @param item
   * @return
   */
  AbstractTabItem* GetTab(int index);

private slots:

  void TabSelectionChanged(int index);
  void DragStarted(const QPoint& location);

public:

  NativeTabFolder(QWidget* parent);

  void UpdateColors();

  void SetActive(int activeState);

  bool eventFilter(QObject* watched, QEvent* event);

  void CloseButtonClicked(AbstractTabItem* item);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#computeSize(int, int)
   */
  QSize ComputeSize(int widthHint, int heightHint);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#add(int)
   */
  AbstractTabItem* Add(int index, int flags);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#layout(boolean)
   */
  void Layout(bool flushCache);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getPaneMenuLocation()
   */
  QPoint GetPaneMenuLocation();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#setState(int)
   */
  void SetState(int state);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getClientArea()
   */
  QRect GetClientArea();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getItems()
   */
  std::vector<AbstractTabItem*> GetItems();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#setSelection(org.opencherry.ui.internal.presentations.util.Widget)
   */
  void SetSelection(AbstractTabItem* toSelect);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#setSelectedInfo(org.opencherry.ui.internal.presentations.util.PartInfo)
   */
  void SetSelectedInfo(const PartInfo& info);

  //    /* (non-Javadoc)
  //     * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getToolbarParent()
  //     */
  //    Composite getToolbarParent() {
  //        return viewForm;
  //    }

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getTabArea()
   */
  QRect GetTabArea();

  //    /* (non-Javadoc)
  //     * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#setToolbar(org.opencherry.swt.widgets.Control)
  //     */
  //    void setToolbar(Control toolbarControl) {
  //
  //        if (toolbarControl != null) {
  //            topCenterCache.setControl(toolbarControl);
  //            topCenter.setTarget(topCenterCache);
  //            viewForm.setTopCenter(topCenter.getControl());
  //        } else {
  //            topCenterCache.setControl(null);
  //            topCenter.setTarget(null);
  //            viewForm.setTopCenter(null);
  //        }
  //
  //        super.setToolbar(toolbarControl);
  //    }

  QWidget* GetControl();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#isOnBorder(org.opencherry.swt.graphics.Point)
   */
  bool IsOnBorder(const QPoint& globalPos);

  AbstractTabItem* GetSelection();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getContentParent()
   */
  QWidget* GetContentParent();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#setContent(org.opencherry.swt.widgets.Control)
   */
  void SetContent(QWidget* newContent);

  /**
   * @return
   * @since 3.1
   */
  QCTabBar* GetTabFolder();

  /**
   * @param item
   * @param newTitle
   * @since 3.1
   */
  /* protected */void SetSelectedTitle(const QString& newTitle);

  /**
   * @param image
   * @since 3.1
   */
  /* protected */void SetSelectedImage(const QPixmap* image);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.presentations.util.AbstractTabFolder#getItem(org.opencherry.swt.graphics.Point)
   */
  AbstractTabItem* GetItem(const QPoint& toFind);

  /**
   * @param enabled
   * @since 3.1
   */
  void EnablePaneMenu(bool enabled);

};

}

#endif /* CHERRYNATIVETABFOLDER_H_ */
