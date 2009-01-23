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

#ifndef CHERRYIVIEWSITE_H_
#define CHERRYIVIEWSITE_H_

#include "cherryIWorkbenchPartSite.h"

#include <string>

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * The primary interface between a view part and the workbench.
 * <p>
 * The workbench exposes its implemention of view part sites via this interface,
 * which is not intended to be implemented or extended by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IViewSite : public virtual IWorkbenchPartSite {

  cherryInterfaceMacro(IViewSite, cherry)

  virtual ~IViewSite() {}

    /**
     * Returns the action bars for this part site.
     * Views have exclusive use of their site's action bars.
     *
     * @return the action bars
     */
    //IActionBars getActionBars();

    /**
     * Returns the secondary id for this part site's part,
     * or <code>null</code> if it has none.
     *
     * @see IWorkbenchPage#showView(String, String, int)
     * @since 3.0
     */
    virtual std::string GetSecondaryId() = 0;
};

}

#endif /*CHERRYIVIEWSITE_H_*/
