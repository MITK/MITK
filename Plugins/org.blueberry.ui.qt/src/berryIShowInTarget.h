/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISHOWINTARGET_H
#define BERRYISHOWINTARGET_H

#include <berryObject.h>

namespace berry {

class ShowInContext;

/**
 * This interface must be provided by Show In targets (views listed in the 'Show In'
 * menu).
 * <p>
 * The view can either directly implement this interface, or provide it via
 * <code>IAdaptable.getAdapter(IShowInTarget.class)</code>.
 * </p>
 *
 * @see IPageLayout#AddShowInPart
 */
struct IShowInTarget : public virtual Object
{

  berryObjectMacro(berry::IShowInTarget);

  /**
   * Shows the given context in this target.
   * The target should check the context's selection for elements
   * to show.  If there are no relevant elements in the selection,
   * then it should check the context's input.
   *
   * @param context the context to show
   * @return <code>true</code> if the context could be shown,
   *   <code>false</code> otherwise
   */
  virtual bool Show(const SmartPointer<ShowInContext>& context) = 0;
};

}

#endif // BERRYISHOWINTARGET_H
