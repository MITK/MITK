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

#ifndef CHERRYPERSPECTIVEHELPER_H_
#define CHERRYPERSPECTIVEHELPER_H_

#include "cherryWorkbenchPage.h"
#include "cherryPartPlaceholder.h"
#include "cherryPerspective.h"
#include "cherryViewSashContainer.h"
#include "cherryPartPlaceholder.h"
#include "cherryDetachedWindow.h"
#include "cherryDetachedPlaceHolder.h"
#include "cherryIDragOverListener.h"
#include "cherryAbstractDropTarget.h"
#include "../cherryPartPane.h"

namespace cherry
{

class WorkbenchPage;

/**
 * A perspective presentation is a collection of parts with a layout. Each part
 * is parented to a main window, so you can create more than one presentation
 * on a set of parts and change the layout just by activating / deactivating a
 * presentation.
 *
 * In addition, the user can change the position of any part by mouse
 * manipulation (drag & drop). If a part is removed, we leave a placeholder
 * behind to indicate where it goes should the part be added back.
 */
class PerspectiveHelper
{

  friend class PartStack;
  friend class ViewSashContainer;

private:
  WorkbenchPage::Pointer page;

protected:
  Perspective::Pointer perspective;

protected:
  void* parentWidget;

private:
  ViewSashContainer::Pointer mainLayout;

  //private: PartStack maximizedStack;

  /**
   * If there is a ViewStack maximized on shutdown the id is
   * cached and restored into this field on 'restoreState'.
   * This is then used to bash the ViewStack's presentation state
   * into the correct value on activation (the startup life-cycle
   * is such that we have to use this 'latch' because the window
   * state isn't valid until the activate happens.
   */
  //private: String maximizedStackId;

private:
  typedef std::list<DetachedWindow::Pointer> DetachedWindowsType;
  DetachedWindowsType detachedWindowList;

private:
  typedef std::list<DetachedPlaceHolder::Pointer> DetachedPlaceHoldersType;
  DetachedPlaceHoldersType detachedPlaceHolderList;

  /**
   * Maps a stack's id to its current bounds
   * this is used to capture the current bounds of all
   * stacks -before- starting a maximize (since the
   * iterative 'minimize' calls cause the intial stack's
   * bounds to change.
   */
private:
  std::map<std::string, Rectangle> boundsMap;

private:
  bool detachable;

protected:
  bool active;

  // key is the LayoutPart object, value is the PartDragDrop object
  //private: IPartDropListener partDropListener;

private:
  static const int MIN_DETACH_WIDTH;

private:
  static const int MIN_DETACH_HEIGHT;

  struct DragOverListener: public IDragOverListener
  {

    DragOverListener(PerspectiveHelper* perspHelper);

    IDropTarget::Pointer Drag(void* currentControl,
        Object::Pointer draggedObject, const Point& position,
        const Rectangle& dragRectangle);

  private:
    PerspectiveHelper* perspHelper;

  };

  IDragOverListener::Pointer dragTarget;

  struct ActualDropTarget: public AbstractDropTarget
  {

    cherryObjectMacro(ActualDropTarget)

    /**
     * @param part
     * @param dragRectangle
     * @since 3.1
     */
    void SetTarget(PartPane::Pointer part, const Rectangle& dragRectangle);

    /**
     * @param part
     * @param dragRectangle
     * @since 3.1
     */
    void SetTarget(PartStack::Pointer part, const Rectangle& dragRectangle);

    ActualDropTarget(PerspectiveHelper* perspHelper, PartPane::Pointer part, const Rectangle& dragRectangle);

    ActualDropTarget(PerspectiveHelper* perspHelper, PartStack::Pointer part, const Rectangle& dragRectangle);

    void Drop();

    DnDTweaklet::CursorType GetCursor();

  private:

    PartPane::Pointer part;
    PartStack::Pointer stack;

    Rectangle dragRectangle;

    PerspectiveHelper* perspHelper;
  };

  ActualDropTarget::Pointer dropTarget;

private:
  struct MatchingPart
  {

    std::string pid;
    std::string sid;
    StackablePart::Pointer part;
    bool hasWildcard;
    int len;

    MatchingPart(const std::string& pid, const std::string& sid,
        StackablePart::Pointer part);

  };

  struct CompareMatchingParts: public std::binary_function<MatchingPart, MatchingPart, bool>
  {
    bool operator()(const MatchingPart& m1, const MatchingPart& m2) const;
  };

  /**
   * Constructs a new object.
   */
public:
  PerspectiveHelper(WorkbenchPage::Pointer workbenchPage,
      ViewSashContainer::Pointer mainLayout, Perspective::Pointer perspective);

  /**
   * Show the presentation.
   */
public:
  void Activate(void* parent);

  /**
   * Adds a part to the presentation. If a placeholder exists for the part
   * then swap the part in. Otherwise, add the part in the bottom right
   * corner of the presentation.
   */
public:
  void AddPart(StackablePart::Pointer part);

  /**
   * Attaches a part that was previously detached to the mainLayout.
   *
   * @param ref
   */
public:
  void AttachPart(IViewReference::Pointer ref);

  /**
   * Return whether detachable parts can be supported.
   */
public:
  bool CanDetach();

  /**
   * Bring a part forward so it is visible.
   *
   * @return true if the part was brought to top, false if not.
   */
public:
  bool BringPartToTop(StackablePart::Pointer part);

  /**
   * Returns true if the given part is visible.
   * A part is visible if it's top-level (not in a tab folder) or if it is the top one
   * in a tab folder.
   */
public:
  bool IsPartVisible(IWorkbenchPartReference::Pointer partRef);

  /**
   * Returns true is not in a tab folder or if it is the top one in a tab
   * folder.
   */
public:
  bool WillPartBeVisible(const std::string& partId);

public:
  bool WillPartBeVisible(const std::string& partId,
      const std::string& secondaryId);

  /**
   * Answer a list of the PartPlaceholder objects.
   */
private:
  std::vector<PartPlaceholder::Pointer> CollectPlaceholders();

  /**
   * Answer a list of the PartPlaceholder objects.
   */
private:
  std::vector<PartPlaceholder::Pointer> CollectPlaceholders(
      const std::list<LayoutPart::Pointer>& parts);

  /**
   * Answer a list of the view panes.
   */
public:
  void CollectViewPanes(std::vector<PartPane::Pointer>& result);

  /**
   * Answer a list of the view panes.
   */
private:
  void CollectViewPanes(std::vector<PartPane::Pointer>& result,
      const std::list<LayoutPart::Pointer>& parts);

  /**
   * Hide the presentation.
   */
public:
  void Deactivate();

public:
  ~PerspectiveHelper();

  /**
   * Writes a description of the layout to the given string buffer.
   * This is used for drag-drop test suites to determine if two layouts are the
   * same. Like a hash code, the description should compare as equal iff the
   * layouts are the same. However, it should be user-readable in order to
   * help debug failed tests. Although these are english readable strings,
   * they should not be translated or equality tests will fail.
   * <p>
   * This is only intended for use by test suites.
   * </p>
   *
   * @param buf
   */
public:
  void DescribeLayout(std::string& buf) const;

  /**
   * Deref a given part. Deconstruct its container as required. Do not remove
   * drag listeners.
   */
protected:
  /* package */void DerefPart(StackablePart::Pointer part);

  /**
   * Create a detached window containing a part.
   */
private:
  void DetachPart(StackablePart::Pointer source, int x, int y);

private:
  void Detach(LayoutPart::Pointer source, int x, int y);

  /**
   * Detached a part from the mainLayout. Presently this does not use placeholders
   * since the current implementation is not robust enough to remember a view's position
   * in more than one root container. For now the view is simply derefed and will dock
   * in the default position when attachPart is called.
   *
   * By default parts detached this way are set to float on top of the workbench
   * without docking. It is assumed that people that want to drag a part back onto
   * the WorkbenchWindow will detach it via drag and drop.
   *
   * @param ref
   */
public:
  void DetachPart(IViewReference::Pointer ref);

  /**
   * Create a detached window containing a part.
   */
public:
  void AddDetachedPart(StackablePart::Pointer part);

public:
  void AddDetachedPart(StackablePart::Pointer part, const Rectangle& bounds);

  /**
   * disableDragging.
   */
private:
  void DisableAllDrag();

  /**
   * enableDragging.
   */
private:
  void EnableAllDrag();

  /**
   * Find the first part with a given ID in the presentation.
   * Wild cards now supported.
   */
private:
  StackablePart::Pointer FindPart(const std::string& id);

  /**
   * Find the first part that matches the specified
   * primary and secondary id pair.  Wild cards
   * are supported.
   */
public:
  StackablePart::Pointer FindPart(const std::string& primaryId,
      const std::string& secondaryId);

  /**
   * Find the first part with a given ID in the presentation.
   */
private:
  StackablePart::Pointer FindPart(const std::string& id,
      const std::list<LayoutPart::Pointer>& parts,
      std::vector<MatchingPart>& matchingParts);

  LayoutPart::Pointer FindLayoutPart(const std::string& id,
      const std::list<LayoutPart::Pointer>& parts,
      std::vector<MatchingPart>& matchingParts);

  StackablePart::Pointer FindPart(const std::string& id,
      const std::list<StackablePart::Pointer>& parts,
      std::vector<MatchingPart>& matchingParts);

  /**
   * Find the first part that matches the specified
   * primary and secondary id pair.  Wild cards
   * are supported.
   */
private:
  StackablePart::Pointer FindPart(const std::string& primaryId,
      const std::string& secondaryId,
      const std::list<LayoutPart::Pointer>& parts,
      std::vector<MatchingPart>& matchingParts);

  StackablePart::Pointer FindPart(const std::string& primaryId,
      const std::string& secondaryId,
      const std::list<StackablePart::Pointer>& parts,
      std::vector<MatchingPart>& matchingParts);

  /**
   * Returns true if a placeholder exists for a given ID.
   */
public:
  bool HasPlaceholder(const std::string& id);

  /**
   * Returns true if a placeholder exists for a given ID.
   * @since 3.0
   */
public:
  bool HasPlaceholder(const std::string& primaryId,
      const std::string& secondaryId);

  /**
   * Returns the layout container.
   */
public:
  PartSashContainer::Pointer GetLayout() const;

  /**
   * Gets the active state.
   */
public:
  bool IsActive();

  /**
   * Returns whether the presentation is zoomed.
   *
   * <strong>NOTE:</strong> As of 3.3 this method should always return 'false'
   * when using the new min/max behavior. It is only used for
   * legacy 'zoom' handling.
   */
  //    public: bool IsZoomed() {
  //        return mainLayout.getZoomedPart() != null;
  //    }

  /**
   * @return The currently maxmized stack (if any)
   */
  //  public: PartStack::Pointer GetMaximizedStack() {
  //    return maximizedStack;
  //  }

  /**
   * Sets the currently maximized stack. Used for query
   * and 'unZoom' purposes in the 3.3 presentation.
   *
   * @param stack The newly maximized stack
   */
  //  public: void SetMaximizedStack(PartStack::Pointer stack) {
  //    if (stack == maximizedStack)
  //      return;
  //
  //    maximizedStack = stack;
  //  }

  /**
   * Returns the ratio that should be used when docking the given source
   * part onto the given target
   *
   * @param source newly added part
   * @param target existing part being dragged over
   * @return the final size of the source part (wrt the current size of target)
   * after it is docked
   */
public:
  static float GetDockingRatio(StackablePart::Pointer source,
      LayoutPart::Pointer target);

  /**
   * Returns whether changes to a part will affect zoom. There are a few
   * conditions for this .. - we are zoomed. - the part is contained in the
   * main window. - the part is not the zoom part - the part is not a fast
   * view - the part and the zoom part are not in the same editor workbook
   * - the part and the zoom part are not in the same view stack.
   */
  //    public: bool PartChangeAffectsZoom(LayoutPart::Pointer pane) {
  //        return pane.isObscuredByZoom();
  //    }

  /**
   * Remove all references to a part.
   */
public:
  void RemovePart(StackablePart::Pointer part);

  /**
   * Add a part to the presentation.
   *
   * Note: unlike all other LayoutParts, PartPlaceholders will still point to
   * their parent container even when it is inactive. This method relies on this
   * fact to locate the parent.
   */
public:
  void ReplacePlaceholderWithPart(StackablePart::Pointer part);

  /**
   * @see org.opencherry.ui.IPersistable
   */
public:
  bool RestoreState(IMemento::Pointer memento);

  /**
   * @see org.opencherry.ui.IPersistable
   */
public:
  bool SaveState(IMemento::Pointer memento);

  /**
   * Zoom in on a particular layout part.
   */
  //    public: void zoomIn(IWorkbenchPartReference ref) {
  //        PartPane pane = ((WorkbenchPartReference) ref).getPane();
  //
  //
  //        parentWidget.setRedraw(false);
  //        try {
  //            pane.requestZoomIn();
  //        } finally {
  //            parentWidget.setRedraw(true);
  //        }
  //    }

  /**
   * Zoom out.
   */
  //    public: void zoomOut() {
  //      // New 3.3 behavior
  //    if (Perspective.useNewMinMax(perspective)) {
  //       if (maximizedStack != null)
  //         maximizedStack.setState(IStackPresentationSite.STATE_RESTORED);
  //       return;
  //    }
  //
  //        LayoutPart zoomPart = mainLayout.getZoomedPart();
  //        if (zoomPart != null) {
  //            zoomPart.requestZoomOut();
  //        }
  //    }

  /**
   * Forces the perspective to have no zoomed or minimized parts.
   * This is used when switching to the 3.3 presentation...
   */
  //    public: void forceNoZoom() {
  //      // Ensure that nobody's zoomed
  //      zoomOut();
  //
  //      // Now, walk the layout ensuring that nothing is minimized
  //      LayoutPart[] kids = mainLayout.getChildren();
  //      for (int i = 0; i < kids.length; i++) {
  //      if (kids[i] instanceof ViewStack) {
  //        ((ViewStack)kids[i]).setMinimized(false);
  //      }
  //      else if (kids[i] instanceof EditorSashContainer) {
  //        LayoutPart[] editorStacks = ((EditorSashContainer)kids[i]).getChildren();
  //        for (int j = 0; j < editorStacks.length; j++) {
  //          if (editorStacks[j] instanceof EditorStack) {
  //            ((EditorStack)editorStacks[j]).setMinimized(false);
  //          }
  //        }
  //      }
  //    }
  //    }

  /**
   * Captures the current bounds of all ViewStacks and the editor
   * area and puts them into an ID -> Rectangle map. This info is
   * used to cache the bounds so that we can correctly place minimized
   * stacks during a 'maximized' operation (where the iterative min's
   * affect the current layout while being performed.
   */
public:
  void UpdateBoundsMap();

  /**
   * Resets the bounds map so that it won't interfere with normal minimize
   * operations
   */
public:
  void ResetBoundsMap();

public:
  Rectangle GetCachedBoundsFor(const std::string& id);
};

}

#endif /* CHERRYPERSPECTIVEHELPER_H_ */
