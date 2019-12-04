/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYWINDOWPARTSELECTIONTRACKER_H_
#define BERRYWINDOWPARTSELECTIONTRACKER_H_

#include "berryAbstractPartSelectionTracker.h"

#include "berryINullSelectionListener.h"

namespace berry {

struct IWorkbenchWindow;
struct IWorkbenchPage;

/**
 * Provides part selection tracking for a part with a specific id
 * in all pages of a specific workbench window. This tracker shields
 * clients from a part opening and closing, and still provides selection
 * notification/information even when the part is not active.
 */
class WindowPartSelectionTracker : public AbstractPartSelectionTracker {
        // implements IPageListener {

private:

    /**
     * The window this selection tracker is working in
     */
    IWorkbenchWindow* fWindow;

    /**
     * Part selection listener.
     */
    QScopedPointer<INullSelectionListener> selListener;

    /**
     * Part post selection listener
     */
    QScopedPointer<INullSelectionListener> postSelListener;

    /**
     * Sets the window this tracker is working in.
     *
     * @param window workbench window
     */
    void SetWindow(IWorkbenchWindow* window);

public:

    /**
     * Constructs a new selection tracker for the given window and part id.
     *
     * @param window workbench window
     * @param partId part identifier
     */
    WindowPartSelectionTracker(IWorkbenchWindow* window, const QString& partId);

    /*
     * @see IPageListener#pageActivated(IWorkbenchPage)
     */
    void PageActivated(SmartPointer<IWorkbenchPage> page);

    /*
     * @see IPageListener#pageClosed(IWorkbenchPage)
     */
    void PageClosed(SmartPointer<IWorkbenchPage> page);

    /*
     * @see IPageListener#pageOpened(IWorkbenchPage)
     */
    void PageOpened(SmartPointer<IWorkbenchPage> page);

    /*
     * @see AbstractPartSelectionTracker#getSelection()
     */
    ISelection::ConstPointer GetSelection() override;


protected:

    /**
     * Returns the window this tracker is working in.
     *
     * @return workbench window
     */
    SmartPointer<IWorkbenchWindow> GetWindow();

};

}

#endif /* BERRYWINDOWPARTSELECTIONTRACKER_H_ */
