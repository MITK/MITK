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


#ifndef BERRYIFOLDERLAYOUT_H_
#define BERRYIFOLDERLAYOUT_H_

#include "berryIPlaceholderFolderLayout.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * An <code>IFolderLayout</code> is used to define the initial views within a folder.
 * The folder itself is contained within an <code>IPageLayout</code>.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IPageLayout#createFolder
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IFolderLayout : public IPlaceholderFolderLayout {

  berryInterfaceMacro(IFolderLayout, berry)

    /**
     * Adds a view with the given compound id to this folder.
     * See the {@link IPageLayout} type documentation for more details about compound ids.
     * The primary id must name a view contributed to the workbench's view extension point
     * (named <code>"org.blueberry.ui.views"</code>).
     *
     * @param viewId the view id
     */
    virtual void AddView(const std::string& viewId) = 0;
};

}

#endif /*BERRYIFOLDERLAYOUT_H_*/
