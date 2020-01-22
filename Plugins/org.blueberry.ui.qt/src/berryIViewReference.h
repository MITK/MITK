/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIVIEWREFERENCE_H_
#define BERRYIVIEWREFERENCE_H_

#include "berryIWorkbenchPartReference.h"
#include "berryIViewPart.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Defines a reference to an IViewPart.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct BERRY_UI_QT IViewReference : virtual public IWorkbenchPartReference {

  berryObjectMacro(berry::IViewReference);

  ~IViewReference() override;

    /**
     * Returns the secondary ID for the view.
     *
     * @return the secondary ID, or <code>null</code> if there is no secondary id
     * @see IWorkbenchPage#showView(String, String, int)
     * @since 3.0
     */
    virtual QString GetSecondaryId() = 0;

    /**
     * Returns the <code>IViewPart</code> referenced by this object.
     * Returns <code>null</code> if the view was not instantiated or
     * it failed to be restored.  Tries to restore the view
     * if <code>restore</code> is true.
     */
    virtual IViewPart::Pointer GetView(bool restore) = 0;

};

}  // namespace berry

#endif /*BERRYIVIEWREFERENCE_H_*/
