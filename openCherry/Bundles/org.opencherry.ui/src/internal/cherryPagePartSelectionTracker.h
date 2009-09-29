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

#include "cherryAbstractPartSelectionTracker.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 *
 * Provides per-part selection tracking for the selection service.
 */
class PagePartSelectionTracker : public AbstractPartSelectionTracker, public IPartListener,
 public IPerspectiveListener, public ISelectionChangedListener
{

public:
  cherryObjectMacro(PagePartSelectionTracker);

private:

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

  void PerspectiveActivated(IWorkbenchPage::Pointer page, IPerspectiveDescriptor::Pointer perspective);

  void PerspectiveChanged(IWorkbenchPage::Pointer page, IPerspectiveDescriptor::Pointer perspective,
      const std::string& changeId);

  void PerspectiveChanged(IWorkbenchPage::Pointer page, IPerspectiveDescriptor::Pointer perspective,
      IWorkbenchPartReference::Pointer partRef, const std::string& changeId);

  /**
   * Returns the selection from the part being tracked,
   * or <code>null</code> if the part is closed or has no selection.
   */
  ISelection::ConstPointer GetSelection();

  /**
   * Disposes this selection tracker.  This removes all listeners currently registered.
   */
  ~PagePartSelectionTracker();

protected:

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
   * Returns the id for the given part, taking into account
   * multi-view instances which may have a secondary id.
   *
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
