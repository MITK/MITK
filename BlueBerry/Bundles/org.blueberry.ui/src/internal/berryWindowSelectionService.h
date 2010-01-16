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
    SmartPointer<IWorkbenchWindow> window;

     /**
     * Sets the window.
     */
    void SetWindow(SmartPointer<IWorkbenchWindow> window);

protected:

    /**
     * Returns the window.
     */
    SmartPointer<IWorkbenchWindow> GetWindow() const;

    /*
     * @see AbstractSelectionService#createPartTracker(String)
     */
    AbstractPartSelectionTracker::Pointer CreatePartTracker(const std::string& partId) const;

public:

    /**
     * Creates a new selection service for the given window.
     */
    WindowSelectionService(SmartPointer<IWorkbenchWindow> window);

};

}

#endif /* BERRYWINDOWSELECTIONSERVICE_H_ */
