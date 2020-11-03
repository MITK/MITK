/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYABSTRACTTABFOLDER_H_
#define BERRYABSTRACTTABFOLDER_H_

#include <berryMessage.h>

#include <QRect>

#include "berryPartInfo.h"
#include "berryTabFolderEvent.h"

class QWidget;

namespace berry
{

class AbstractTabFolder
{

public:

  Message1<TabFolderEvent::Pointer> tabFolderEvent;

private:

  int state;
  int activeState;

public:

  AbstractTabFolder();
  virtual ~AbstractTabFolder();

  virtual QSize ComputeSize(int widthHint, int heightHint) = 0;

  virtual AbstractTabItem* Add(int index, int flags) = 0;

  virtual void Move(int from, int to) = 0;

  virtual QWidget* GetContentParent() = 0;
  virtual void SetContent(QWidget* newContent) = 0;

  virtual QList<AbstractTabItem*> GetItems() = 0;

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

  void HandleContextMenu(const QPoint& displayPos);

  void HandleMousePress(const QPoint& displayPos);

  void HandleDoubleClick(const QPoint& displayPos);

  void HandleDragStarted(const QPoint& displayPos);

};

}

#endif /* BERRYABSTRACTTABFOLDER_H_ */
