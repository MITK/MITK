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

#ifndef CHERRYPRESENTABLEPARTFOLDER_H_
#define CHERRYPRESENTABLEPARTFOLDER_H_

#include "cherryIPresentablePartList.h"
#include "cherryAbstractTabFolder.h"
#include "cherryPartInfo.h"

#include <cherryIShellListener.h>
#include <cherryIPropertyChangeListener.h>
#include <cherryGuiTkIControlListener.h>
#include <list>

namespace cherry
{

class PresentablePartFolder: public IPresentablePartList
{
private:

  AbstractTabFolder* folder;
  IPresentablePart::Pointer current;
  //private ProxyControl toolbarProxy;
  QWidget* contentProxy;
  static PartInfo tempPartInfo;

  std::list<IPresentablePart::Pointer> partList;
  bool isVisible;

  /**
   * Movement listener. Updates the bounds of the target to match the
   * bounds of the dummy control.
   */
  struct ContentProxyListener : public GuiTk::IControlListener
  {
    ContentProxyListener(PresentablePartFolder*);

    Events::Types GetEventTypes() const;

    void ControlMoved(GuiTk::ControlEvent::Pointer e);

    void ControlResized(GuiTk::ControlEvent::Pointer e);

  private:

    PresentablePartFolder* folder;

  };

  GuiTk::IControlListener::Pointer contentListener;

  struct ShellListener: public IShellListener
  {

    ShellListener(AbstractTabFolder* folder);

    void ShellActivated(ShellEvent::Pointer e);

    void ShellDeactivated(ShellEvent::Pointer e);

  private:

    AbstractTabFolder* folder;
  };

  IShellListener::Pointer shellListener;

  /**
   * Listener attached to all child parts. It responds to changes in part properties
   */
  struct ChildPropertyChangeListener: public IPropertyChangeListener
  {
    ChildPropertyChangeListener(PresentablePartFolder* folder);

    void PropertyChange(Object::Pointer source, int property);

  private:

    PresentablePartFolder* presentablePartFolder;
  };

  IPropertyChangeListener::Pointer childPropertyChangeListener;

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
   * @see org.opencherry.ui.internal.presentations.util.IPresentablePartList#getPartList()
   */
  std::vector<IPresentablePart::Pointer> GetPartList();

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
  std::size_t Size();

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

#endif /* CHERRYPRESENTABLEPARTFOLDER_H_ */
