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

#ifndef BERRYPAGEPARTSELECTIONTRACKER_H_
#define BERRYPAGEPARTSELECTIONTRACKER_H_

#include "../berryIPartListener.h"
#include "../berryISelectionChangedListener.h"
#include "../berryISelectionService.h"
#include "../berryISelectionProvider.h"
#include "../berrySelectionChangedEvent.h"
#include "../berryIWorkbenchPage.h"

#include "berryAbstractPartSelectionTracker.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Provides per-part selection tracking for the selection service.
 */
class PagePartSelectionTracker : public AbstractPartSelectionTracker
{

public:
  osgiObjectMacro(PagePartSelectionTracker);

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
  friend class SelTrackerPartListener;
  friend class SelTrackerPerspectiveListener;
  friend class SelTrackerSelectionChangedListener;

  ISelectionChangedListener::Pointer postSelectionListener;
  IPerspectiveListener::Pointer perspListener;
  ISelectionChangedListener::Pointer selChangedListener;
  IPartListener::Pointer partListener;

public:

  /**
   * Constructs a part selection tracker for the part with the given id.
   *
   * @param id part identifier
   */
  PagePartSelectionTracker(IWorkbenchPage::Pointer page,
      const std::string& partId);


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

#endif /*BERRYPAGEPARTSELECTIONTRACKER_H_*/
