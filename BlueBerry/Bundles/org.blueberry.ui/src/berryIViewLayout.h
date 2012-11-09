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

#ifndef BERRYIVIEWLAYOUT_H_
#define BERRYIVIEWLAYOUT_H_

#include <org_blueberry_ui_Export.h>

#include <berryMacros.h>
#include <berryObject.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * Represents the layout info for a view or placeholder in an {@link IPageLayout}.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IViewLayout : public Object {

  berryInterfaceMacro(IViewLayout, berry)

  virtual  ~IViewLayout();

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

#endif /*BERRYIVIEWLAYOUT_H_*/
