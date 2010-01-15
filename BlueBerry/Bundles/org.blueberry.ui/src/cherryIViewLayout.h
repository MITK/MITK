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

#ifndef CHERRYIVIEWLAYOUT_H_
#define CHERRYIVIEWLAYOUT_H_

#include <cherryMacros.h>
#include <cherryObject.h>

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * Represents the layout info for a view or placeholder in an {@link IPageLayout}.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IViewLayout : public Object {

  cherryInterfaceMacro(IViewLayout, cherry)

  virtual  ~IViewLayout() {}

    /**
     * Returns whether the view is closeable.
     * The default is <code>true</code>.
     *
     * @return <code>true</code> if the view is closeable, <code>false</code> if not
     */
    virtual bool IsCloseable() = 0;

    /**
     * Sets whether the view is closeable.
     *
     * @param closeable <code>true</code> if the view is closeable, <code>false</code> if not
     */
  virtual void SetCloseable(bool closeable) = 0;

    /**
     * Returns whether the view is moveable.
     * The default is <code>true</code>.
     *
     * @return <code>true</code> if the view is moveable, <code>false</code> if not
     */
  virtual bool IsMoveable() = 0;

    /**
     * Sets whether the view is moveable.
     *
     * @param moveable <code>true</code> if the view is moveable, <code>false</code> if not
     */
  virtual void SetMoveable(bool moveable) = 0;

    /**
     * Returns whether the view is a standalone view.
     *
     * @see IPageLayout#addStandaloneView
     */
  virtual bool IsStandalone() = 0;

    /**
     * Returns whether the view shows its title.
     * This is only applicable to standalone views.
     *
     * @see IPageLayout#addStandaloneView
     */
  virtual bool GetShowTitle() = 0;
};

}

#endif /*CHERRYIVIEWLAYOUT_H_*/
