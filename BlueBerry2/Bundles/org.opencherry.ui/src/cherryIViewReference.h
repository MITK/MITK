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

#ifndef CHERRYIVIEWREFERENCE_H_
#define CHERRYIVIEWREFERENCE_H_

#include "cherryIWorkbenchPartReference.h"
#include "cherryIViewPart.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * Defines a reference to an IViewPart.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct CHERRY_UI IViewReference : virtual public IWorkbenchPartReference {

  osgiInterfaceMacro(cherry::IViewReference);

    /**
     * Returns the secondary ID for the view.
     *
     * @return the secondary ID, or <code>null</code> if there is no secondary id
     * @see IWorkbenchPage#showView(String, String, int)
     * @since 3.0
     */
    virtual std::string GetSecondaryId() = 0;

    /**
     * Returns the <code>IViewPart</code> referenced by this object.
     * Returns <code>null</code> if the view was not instantiated or
     * it failed to be restored.  Tries to restore the view
     * if <code>restore</code> is true.
     */
    virtual IViewPart::Pointer GetView(bool restore) = 0;

};

}  // namespace cherry

#endif /*CHERRYIVIEWREFERENCE_H_*/
