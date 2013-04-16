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


#ifndef BERRYISHELLPROVIDER_H_
#define BERRYISHELLPROVIDER_H_

#include <org_blueberry_ui_Export.h>

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
struct BERRY_UI IShellProvider : public virtual Object {

  berryInterfaceMacro(IShellProvider, berry);

  ~IShellProvider();

    /**
     * Returns the current shell (or null if none). This return value may
     * change over time, and should not be cached.
     *
     * @return the current shell or null if none
     */
   virtual Shell::Pointer GetShell() = 0;
};

}

#endif /* BERRYISHELLPROVIDER_H_ */
