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


#ifndef BERRYWINDOWSELECTIONSERVICE_H_
#define BERRYWINDOWSELECTIONSERVICE_H_

#include "berryAbstractSelectionService.h"

namespace berry {

struct IWorkbenchWindow;

/**
 * The selection service for a window.
 */
/* package */
class WindowSelectionService : public AbstractSelectionService {

private:

    /**
     * The window.
     */
    IWorkbenchWindow* window;

     /**
     * Sets the window.
     */
    void SetWindow(IWorkbenchWindow* window);

protected:

    /**
     * Returns the window.
     */
    IWorkbenchWindow* GetWindow() const;

    /*
     * @see AbstractSelectionService#createPartTracker(String)
     */
    AbstractPartSelectionTracker::Pointer CreatePartTracker(const std::string& partId) const;

public:

    /**
     * Creates a new selection service for the given window.
     */
    WindowSelectionService(IWorkbenchWindow* window);

};

}

#endif /* BERRYWINDOWSELECTIONSERVICE_H_ */
