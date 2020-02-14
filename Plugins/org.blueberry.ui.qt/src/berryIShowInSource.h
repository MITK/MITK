/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISHOWINSOURCE_H
#define BERRYISHOWINSOURCE_H

#include <berryObject.h>

namespace berry {

class ShowInContext;

/**
 * Parts which need to provide a particular context to a 'Show In' target can
 * provide this interface.
 * <p>
 * The part can either directly implement this interface, or provide it via
 * <code>IAdaptable#GetAdapter<IShowInSource>()</code>.
 * </p
 *
 * @see IShowInTarget
 */
struct IShowInSource : public virtual Object
{
  berryObjectMacro(berry::IShowInSource);

  /**
   * Returns the context to show, or <code>null</code> if there is
   * currently no valid context to show.
   *
   * @return the context to show, or <code>null</code>
   */
  virtual SmartPointer<ShowInContext> GetShowInContext() const = 0;
};

}

Q_DECLARE_INTERFACE(berry::IShowInSource, "org.blueberry.ui.IShowInSource")

#endif // BERRYISHOWINSOURCE_H
