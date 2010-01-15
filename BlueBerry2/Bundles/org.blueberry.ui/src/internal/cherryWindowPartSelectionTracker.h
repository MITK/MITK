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


#ifndef CHERRYWINDOWPARTSELECTIONTRACKER_H_
#define CHERRYWINDOWPARTSELECTIONTRACKER_H_

#include "cherryAbstractPartSelectionTracker.h"

namespace cherry {

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
    SmartPointer<IWorkbenchWindow> fWindow;

    /**
     * Part selection listener.
     */
    ISelectionListener::Pointer selListener;

    /**
     * Part post selection listener
     */
    ISelectionListener::Pointer postSelListener;

    /**
     * Sets the window this tracker is working in.
     *
     * @param window workbench window
     */
    void SetWindow(SmartPointer<IWorkbenchWindow> window);

public:

    /**
     * Constructs a new selection tracker for the given window and part id.
     *
     * @param window workbench window
     * @param partId part identifier
     */
    WindowPartSelectionTracker(SmartPointer<IWorkbenchWindow> window, const std::string& partId);

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
    ISelection::ConstPointer GetSelection();


protected:

    /**
     * Returns the window this tracker is working in.
     *
     * @return workbench window
     */
    SmartPointer<IWorkbenchWindow> GetWindow();

};

}

#endif /* CHERRYWINDOWPARTSELECTIONTRACKER_H_ */
