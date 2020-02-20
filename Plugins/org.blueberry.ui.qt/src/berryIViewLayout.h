/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIVIEWLAYOUT_H_
#define BERRYIVIEWLAYOUT_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryMacros.h>
#include <berryObject.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Represents the layout info for a view or placeholder in an {@link IPageLayout}.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IViewLayout : public Object
{

  berryObjectMacro(berry::IViewLayout);

   ~IViewLayout() override;

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
