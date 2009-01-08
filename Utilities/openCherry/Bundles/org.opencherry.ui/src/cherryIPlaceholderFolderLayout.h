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

#ifndef CHERRYIPLACEHOLDERFOLDERLAYOUT_H_
#define CHERRYIPLACEHOLDERFOLDERLAYOUT_H_

#include <cherryMacros.h>
#include <cherryObject.h>

#include "cherryUiDll.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * An <code>IPlaceholderFolderLayout</code> is used to define the initial
 * view placeholders within a folder.
 * The folder itself is contained within an <code>IPageLayout</code>.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IPageLayout#createPlaceholderFolder
 * @since 2.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IPlaceholderFolderLayout : public Object {

  cherryClassMacro(IPlaceholderFolderLayout)

  virtual ~IPlaceholderFolderLayout() {}

    /**
     * Adds a view placeholder to this folder.
     * A view placeholder is used to define the position of a view before the view
     * appears.  Initially, it is invisible; however, if the user ever opens a view
     * whose compound id matches the placeholder, the view will appear at the same
     * location as the placeholder.
     * See the {@link IPageLayout} type documentation for more details about compound ids.
     * If the placeholder contains wildcards, it remains in the layout, otherwise
     * it is replaced by the view.
     * If the primary id of the placeholder has no wildcards, it must refer to a view
     * contributed to the workbench's view extension point
     * (named <code>"org.opencherry.ui.views"</code>).
     *
     * @param viewId the compound view id (wildcards allowed)
     */
    virtual void AddPlaceholder(const std::string& viewId) = 0;

    /**
   * Returns the property with the given id or <code>null</code>. Folder
   * properties are an extensible mechanism for perspective authors to
   * customize the appearance of view stacks. The list of customizable
   * properties is determined by the presentation factory.
   *
   * @param id
   *            Must not be <code>null</code>.
   * @return property value, or <code>null</code> if the property is not
   *         set.
   * @since 3.3
   */
  virtual std::string GetProperty(const std::string& id) = 0;

    /**
   * Sets the given property to the given value. Folder properties are an
   * extensible mechanism for perspective authors to customize the appearance
   * of view stacks. The list of customizable properties is determined by the
   * presentation factory.
   * <p>
   * These folder properties are intended to be set during
   * <code>IPerspectiveFactory#createInitialLayout</code>. Any subsequent
   * changes to property values after this method completes will not fire
   * change notifications and will not be reflected in the presentation.
   * </p>
   *
   * @param id
   *            property id. Must not be <code>null</code>.
   * @param value
   *            property value. <code>null</code> will clear the property.
   * @since 3.3
   */
  virtual void SetProperty(const std::string& id, const std::string& value) = 0;
};

}

#endif /*CHERRYIPLACEHOLDERFOLDERLAYOUT_H_*/
