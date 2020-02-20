/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISHELLPROVIDER_H_
#define BERRYISHELLPROVIDER_H_

#include <org_blueberry_ui_qt_Export.h>

#include "berryShell.h"
#include <berryMacros.h>

namespace berry {

/**
 * Interface for objects that can return a shell. This is normally used for
 * opening child windows. An object that wants to open child shells can take
 * an IShellProvider in its constructor, and the object that implements IShellProvider
 * can dynamically choose where child shells should be opened.
 *
 * @since 3.1
 */
struct BERRY_UI_QT IShellProvider : public virtual Object
{

  berryObjectMacro(berry::IShellProvider, Object);

  ~IShellProvider() override;

    /**
     * Returns the current shell (or null if none). This return value may
     * change over time, and should not be cached.
     *
     * @return the current shell or null if none
     */
   virtual Shell::Pointer GetShell() const = 0;
};

}

#endif /* BERRYISHELLPROVIDER_H_ */
