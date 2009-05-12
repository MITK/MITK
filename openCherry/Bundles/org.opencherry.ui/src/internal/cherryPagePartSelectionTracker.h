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

#ifndef CHERRYPAGEPARTSELECTIONTRACKER_H_
#define CHERRYPAGEPARTSELECTIONTRACKER_H_

#include "../cherryIPartListener.h"
#include "../cherryISelectionChangedListener.h"
#include "../cherryISelectionService.h"
#include "../cherryISelectionProvider.h"
#include "../cherrySelectionChangedEvent.h"
#include "../cherryIWorkbenchPage.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 *
 * Provides per-part selection tracking for the selection service.
 */
class PagePartSelectionTracker : public IPartListener,
  public ISelectionChangedListener
{

public:
  cherryObjectMacro(PagePartSelectionTracker);

private:

  ISelectionService::SelectionEvents selectionEvents;

  /**
   * List of selection listeners for this tracker
   */
  //private ListenerList fListeners = new ListenerList();

  /**
   * List of post selection listeners for this tracker
   */
  //private ListenerList postListeners = new ListenerList();

  /**
   * The id of the part this tracls
   */
  std::string fPartId;

  /**
   * The workbench page for which this is tracking selection.
   */
  IWorkbenchPage::Pointer fPage;

  /**
   * The part in this tracker's page, or <code>null</code> if one is not open.
   */
  IWorkbenchPart::Pointer fPart;

  struct PostSelectionListener : public ISelectionChangedListener
  {
    PostSelectionListener(PagePartSelectionTracker* tracker);

    void SelectionChanged(SelectionChangedEvent::Pointer event);

    PagePartSelectionTracker* m_Tracker;
  };

  friend struct PostSelectionListener;

  ISelectionChangedListener::Pointer postSelectionListener;

public:

  void SelectionChanged(SelectionChangedEvent::Pointer event);

  ISelectionService::SelectionEvents& GetSelectionEvents();

  /**
   * Constructs a part selection tracker for the part with the given id.
   *
   * @param id part identifier
   */
  PagePartSelectionTracker(IWorkbenchPage::Pointer page,
      const std::string& partId);

  /*
   * @see IPartListener#partActivated(IWorkbenchPart)
   */
  void PartActivated(IWorkbenchPartReference::Pointer partRef);

  /*
   * @see IPartListener#partBroughtToTop(IWorkbenchPart)
   */
  void PartBroughtToTop(IWorkbenchPartReference::Pointer partRef);

  /**
   * @see IPartListener#partClosed(IWorkbenchPart)
   */
  void PartClosed(IWorkbenchPartReference::Pointer partRef);

  /*
   * @see IPartListener#partDeactivated(IWorkbenchPart)
   */
  void PartDeactivated(IWorkbenchPartReference::Pointer partRef);

  /**
   * @see IPartListener#partOpened(IWorkbenchPart)
   */
  void PartOpened(IWorkbenchPartReference::Pointer partRef);

  /**
   * @see IPartListener#PartHidden(IWorkbenchPart)
   */
  void PartHidden(IWorkbenchPartReference::Pointer partRef);

  /**
   * @see IPartListener#PartVisible(IWorkbenchPart)
   */
  void PartVisible(IWorkbenchPartReference::Pointer partRef);

  /**
   * @see IPartListener#PartInputChanged(IWorkbenchPart)
   */
  void PartInputChanged(IWorkbenchPartReference::Pointer partRef);

  /**
   * Adds a selection listener to this tracker
   *
   * @param listener the listener to add
   */
  void AddSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Adds a post selection listener to this tracker
   *
   * @param listener the listener to add
   */
  void AddPostSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Returns the selection from the part being tracked,
   * or <code>null</code> if the part is closed or has no selection.
   */
  ISelection::Pointer GetSelection();

  /**
   * Removes a selection listener from this tracker.
   *
   * @param listener the listener to remove
   */
  void RemoveSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Removes a post selection listener from this tracker.
   *
   * @param listener the listener to remove
   */
  void RemovePostSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Disposes this selection tracker.  This removes all listeners currently registered.
   */
  ~PagePartSelectionTracker();

protected:

  /**
   * Fires a selection event to the listeners.
   *
   * @param part the part or <code>null</code> if no active part
   * @param sel the selection or <code>null</code> if no active selection
   * @param listeners the list of listeners to notify
   */
  void FireSelection(IWorkbenchPart::Pointer part, ISelection::Pointer sel);

  /**
   * Fires a post selection event to the listeners.
   *
   * @param part the part or <code>null</code> if no active part
   * @param sel the selection or <code>null</code> if no active selection
   * @param listeners the list of listeners to notify
   */
  void FirePostSelection(IWorkbenchPart::Pointer part, ISelection::Pointer sel);

  /**
   * Returns the id of the part that this tracks.
   *
   * @return part identifier
   */
  std::string GetPartId();

  /**
   * Returns the part this is tracking,
   * or <code>null</code> if it is not open
   *
   * @return part., or <code>null</code>
   */
  IWorkbenchPart::Pointer GetPart();

  /**
   * Returns the page this selection provider works for
   *
   * @return workbench page
   */
  IWorkbenchPage::Pointer GetPage();

  ISelectionProvider::Pointer GetSelectionProvider();

private:

  /**
   * Sets the id of the part that this tracks.
   *
   * @param id view identifier
   */
  void SetPartId(const std::string& partId);

  /**
   * Returns the id for the given part, taking into account
   * multi-view instances which may have a secondary id.
   *
   * @since 3.0
   */
  std::string GetPartId(IWorkbenchPart::Pointer part);

  /**
   * Sets the page this selection provider works for
   *
   * @param page workbench page
   */
  void SetPage(IWorkbenchPage::Pointer page);

  /**
   * Sets the part for this selection tracker.
   *
   * @param part the part
   * @param notify whether to send notification that the selection has changed.
   */
  void SetPart(IWorkbenchPart::Pointer part, bool notify);
};

}

#endif /*CHERRYPAGEPARTSELECTIONTRACKER_H_*/
