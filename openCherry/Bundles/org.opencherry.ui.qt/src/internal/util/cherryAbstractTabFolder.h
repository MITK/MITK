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

#ifndef CHERRYABSTRACTTABFOLDER_H_
#define CHERRYABSTRACTTABFOLDER_H_

#include <cherryMessage.h>

#include <QRect>

#include "cherryPartInfo.h"
#include "cherryTabFolderEvent.h"

class QWidget;

namespace cherry
{

class AbstractTabFolder
{

public:

  Message1<TabFolderEvent::Pointer> tabFolderEvent;

private:

  //  Control toolbar;
  int state;
  int activeState;

  //  Listener menuListener = new Listener() {
  //    /* (non-Javadoc)
  //     * @see org.opencherry.swt.widgets.Listener#handleEvent(org.opencherry.swt.widgets.Event)
  //     */
  //    public void handleEvent(Event event) {
  //      Point globalPos = new Point(event.x, event.y);
  //        handleContextMenu(globalPos, event);
  //    }
  //  };

  //    Listener dragListener = new Listener() {
  //        public void handleEvent(Event e) {
  //            Point globalPos = ((Control)e.widget).toDisplay(e.x, e.y);
  //            handleDragStarted(globalPos, e);
  //        }
  //    };

  //  MouseListener mouseListener = new MouseAdapter() {
  //
  //    // If we single-click on an empty space on the toolbar, move focus to the
  //    // active control
  //    public void mouseDown(MouseEvent e) {
  //      Point p = ((Control)e.widget).toDisplay(e.x, e.y);
  //
  //      handleMouseDown(p, e);
  //    }
  //
  //
  //    // If we double-click on the toolbar, maximize the presentation
  //    public void mouseDoubleClick(MouseEvent e) {
  //      Point p = ((Control)e.widget).toDisplay(e.x, e.y);
  //
  //      handleDoubleClick(p, e);
  //    }
  //  };


public:

  AbstractTabFolder();
  virtual ~AbstractTabFolder();

  virtual QSize ComputeSize(int widthHint, int heightHint) = 0;

  virtual AbstractTabItem* Add(int index, int flags) = 0;

  virtual QWidget* GetContentParent() = 0;
  virtual void SetContent(QWidget* newContent) = 0;

  virtual std::vector<AbstractTabItem*> GetItems() = 0;

  virtual AbstractTabItem* GetSelection() = 0;
  virtual void SetSelection(AbstractTabItem* toSelect) = 0;
  virtual void SetSelectedInfo(const PartInfo& info) = 0;
  virtual void EnablePaneMenu(bool enabled) = 0;

  virtual void SetActive(int activeState);

  virtual int GetActive();

  /**
   * Returns the location where the pane menu should be opened when activated
   * by a keyboard shortcut (display coordinates)
   *
   * @return the location for the pane menu (display coordinates)
   * @since 3.1
   */
  virtual QPoint GetPaneMenuLocation();

  /**
   * Returns the location where the part list should be opened when activated
   * by a keyboard shortcut (display coordinates)
   *
   * @return the location for the part list (display coordinates)
   * @since 3.1
   */
  virtual QPoint GetPartListLocation();

  /**
   * Returns the location where the pane menu should be opened when activated
   * by a keyboard shortcut (display coordinates)
   *
   * @return the location for the pane menu (display coordinates)
   * @since 3.1
   */
  virtual QPoint GetSystemMenuLocation();

  //    /**
  //     * Returns the parent composite that should be used for creating the toolbar.
  //     * Any control passed into setToolbar must have this composite as its parent.
  //     *
  //     * @return the parent composite that should be used for creating the toolbar
  //     *
  //     * @since 3.1
  //     */
  //    virtual QWidget* GetToolbarParent() = 0;

  /**
   * Returns the main control for this folder.
   *
   * @return the main control for the folder
   * @since 3.1
   */
  virtual QWidget* GetControl() = 0;

  virtual AbstractTabItem* GetItem(int idx);

  virtual AbstractTabItem* GetItem(const QPoint& toFind);

  virtual AbstractTabItem* FindItem(Object::Pointer dataToFind);

  /**
   * Returns the index of the given item, or -1 if the given item is
   * not found in this tab folder. Subclasses should override this if
   * the underlying SWT widget has an equivalent method
   *
   * @param item item to find
   * @return the index of the given item or -1
   */
  virtual int IndexOf(AbstractTabItem* item);

  virtual int GetItemCount();

  /**
   * Sets the current state for the folder
   *
   * @param state one of the IStackPresentationSite.STATE_* constants
   */
  virtual void SetState(int state);

  /**
   * Returns the title area for this control (in the control's coordinate system)
   *
   * @return
   */
  virtual QRect GetTabArea() = 0;

  /**
   * Called when the tab folder's shell becomes active or inactive. Subclasses
   * can override this to change the appearance of the tabs based on activation.
   *
   * @param isActive
   */
  virtual void ShellActive(bool isActive);

  virtual void Layout(bool flushCache);

  virtual void SetTabPosition(int tabPosition);

  virtual int GetTabPosition();

  virtual int GetState();

  /**
   * Returns true iff the given point is on the border of the folder.
   * By default, double-clicking, context menus, and drag/drop are disabled
   * on the folder's border.
   *
   * @param toTest a point (display coordinates)
   * @return true iff the point is on the presentation border
   * @since 3.1
   */
  virtual bool IsOnBorder(const QPoint& toTest);

  /**
   * Set the folder to visible. This can be extended to propogate the
   * visibility request to other components in the subclass.
   *
   * @param visible
   *            <code>true</code> - the folder is visible.
   * @since 3.2
   */
  virtual void SetVisible(bool visible);

  /**
   * Cause the folder to hide or show its
   * Minimize and Maximize affordances.
   *
   * @param show
   *            <code>true</code> - the min/max buttons are visible.
   * @since 3.3
   */
  virtual void ShowMinMax(bool show);

protected:

  friend class PresentablePartFolder;

  void FireEvent(TabFolderEvent::Pointer e);

  void FireEvent(int id);

  void FireEvent(int id, AbstractTabItem* w);

  void FireEvent(int id, AbstractTabItem* w, const QPoint& pos);

  //    void attachSlots(QWidget* theControl) {
  //        theControl.addListener(SWT.MenuDetect, menuListener);
  //        theControl.addMouseListener(mouseListener);
  //
  //        //TODO DnD
  //        //PresentationUtil.addDragListener(theControl, dragListener);
  //
  //    }


  void HandleContextMenu(const QPoint& displayPos);

  void HandleMousePress(const QPoint& displayPos);

  void HandleDoubleClick(const QPoint& displayPos);

  void HandleDragStarted(const QPoint& displayPos);

};

}

#endif /* CHERRYABSTRACTTABFOLDER_H_ */
