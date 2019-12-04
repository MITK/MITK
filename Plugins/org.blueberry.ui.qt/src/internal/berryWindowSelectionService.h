/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
    AbstractPartSelectionTracker::Pointer CreatePartTracker(const QString& partId) const override;

public:

    /**
     * Creates a new selection service for the given window.
     */
    WindowSelectionService(IWorkbenchWindow* window);

};

}

#endif /* BERRYWINDOWSELECTIONSERVICE_H_ */
