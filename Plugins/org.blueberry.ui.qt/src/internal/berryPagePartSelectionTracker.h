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

#ifndef BERRYPAGEPARTSELECTIONTRACKER_H_
#define BERRYPAGEPARTSELECTIONTRACKER_H_

#include "berryIPartListener.h"
#include "berryISelectionChangedListener.h"
#include "berryISelectionService.h"
#include "berryISelectionProvider.h"
#include "berrySelectionChangedEvent.h"
#include "berryIWorkbenchPage.h"

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
  berryObjectMacro(PagePartSelectionTracker)

private:

  /**
   * The workbench page for which this is tracking selection.
   */
  IWorkbenchPage* fPage;

  /**
   * The part in this tracker's page, or <code>null</code> if one is not open.
   */
  IWorkbenchPart::Pointer fPart;

  struct PostSelectionListener : public ISelectionChangedListener
  {
    PostSelectionListener(PagePartSelectionTracker* tracker);

    void SelectionChanged(const SelectionChangedEvent::Pointer& event) override;

    PagePartSelectionTracker* m_Tracker;
  };

  friend struct PostSelectionListener;
  friend class SelTrackerPartListener;
  friend class SelTrackerPerspectiveListener;
  friend class SelTrackerSelectionChangedListener;

  QScopedPointer<ISelectionChangedListener> postSelectionListener;
  QScopedPointer<IPerspectiveListener> perspListener;
  QScopedPointer<ISelectionChangedListener> selChangedListener;
  QScopedPointer<IPartListener> partListener;

public:

  /**
   * Constructs a part selection tracker for the part with the given id.
   *
   * @param id part identifier
   */
  PagePartSelectionTracker(IWorkbenchPage* page,
      const QString& partId);


  /**
   * Returns the selection from the part being tracked,
   * or <code>null</code> if the part is closed or has no selection.
   */
  ISelection::ConstPointer GetSelection() override;

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
  QString GetPartId(IWorkbenchPart::Pointer part);

  /**
   * Sets the page this selection provider works for
   *
   * @param page workbench page
   */
  void SetPage(IWorkbenchPage* page);

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
