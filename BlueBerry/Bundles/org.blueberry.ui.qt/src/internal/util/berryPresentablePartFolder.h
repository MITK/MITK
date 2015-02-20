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

#ifndef BERRYPRESENTABLEPARTFOLDER_H_
#define BERRYPRESENTABLEPARTFOLDER_H_

#include "berryIPresentablePartList.h"
#include "berryAbstractTabFolder.h"
#include "berryPartInfo.h"

#include <berryIShellListener.h>
#include <berryIPropertyChangeListener.h>
#include <berryGuiTkIControlListener.h>
#include <list>

namespace berry
{

class PresentablePartFolder: public IPresentablePartList
{
private:

  AbstractTabFolder* folder;
  IPresentablePart::Pointer current;
  //private ProxyControl toolbarProxy;
  QWidget* contentProxy;
  static PartInfo tempPartInfo;

  QList<IPresentablePart::Pointer> partList;
  bool isVisible;

  struct ShellListener: public IShellListener
  {

    ShellListener(AbstractTabFolder* folder);

    void ShellActivated(const ShellEvent::Pointer& e);

    void ShellDeactivated(const ShellEvent::Pointer& e);

  private:

    AbstractTabFolder* folder;
  };

  QScopedPointer<IShellListener> shellListener;

  /**
   * Listener attached to all child parts. It responds to changes in part properties
   */
  struct ChildPropertyChangeListener: public IPropertyChangeListener
  {
    ChildPropertyChangeListener(PresentablePartFolder* folder);

    using IPropertyChangeListener::PropertyChange;
    void PropertyChange(const Object::Pointer& source, int property);

  private:

    PresentablePartFolder* presentablePartFolder;
  };

  QScopedPointer<IPropertyChangeListener> childPropertyChangeListener;

  //    /**
  //     * Dispose listener that is attached to the main control. It triggers cleanup of
  //     * any listeners. This is required to prevent memory leaks.
  //     */
  //    private DisposeListener tabDisposeListener = new DisposeListener() {
  //        public void widgetDisposed(DisposeEvent e) {
  //            if (e.widget == folder.getControl()) {
  //                // If we're disposing the main control...
  //                disposed();
  //            }
  //        }
  //    };


  void LayoutContent();

  void InternalRemove(IPresentablePart::Pointer toRemove);

  //    void SetToolbar(Control newToolbar) {
  //        if (folder.getToolbar() != newToolbar) {
  //            folder.setToolbar(newToolbar);
  //        }
  //    }

  void ChildPropertyChanged(IPresentablePart::Pointer part, int property);

protected:

  void InitTab(AbstractTabItem* item, IPresentablePart::Pointer part);

public:

  /**
   * The PresentablePartFolder takes ownership of the AbstractTabFolder pointer.
   */
  PresentablePartFolder(AbstractTabFolder* folder);

  ~PresentablePartFolder();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.IPresentablePartList#getPartList()
   */
  QList<IPresentablePart::Pointer> GetPartList();

  /**
   * Adds the given presentable part directly into this presentation at the
   * given index. Does nothing if a tab already exists for the given part.
   * This is intended to be called by TabOrder and its subclasses.
   *
   * @param part part to add
   * @param idx index to insert at
   */
  void Insert(IPresentablePart::Pointer part, int idx);

  void Remove(IPresentablePart::Pointer toRemove);

  /**
   * Moves the given part to the given index. When this method returns,
   * indexOf(part) will return newIndex.
   *
   * @param part
   * @param newIndex
   */
  void Move(IPresentablePart::Pointer part, int newIndex);

  /**
   * Returns the number of parts in this folder
   */
  int Size();

  void SetBounds(const QRect& bounds);

  void Select(IPresentablePart::Pointer toSelect);

  IPresentablePart::Pointer GetPartForTab(AbstractTabItem* tab);

  /**
   * Returns the tab for the given part, or null if there is no such tab
   *
   * @param part the part being searched for
   * @return the tab for the given part, or null if there is no such tab
   */
  AbstractTabItem* GetTab(IPresentablePart::Pointer part);

  int IndexOf(IPresentablePart::Pointer part);

  AbstractTabFolder* GetTabFolder();

  void SetVisible(bool isVisible);

  void Layout(bool changed);

  IPresentablePart::Pointer GetCurrent();
};

}

#endif /* BERRYPRESENTABLEPARTFOLDER_H_ */
