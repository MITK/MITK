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


#ifndef CHERRYISHELLPROVIDER_H_
#define CHERRYISHELLPROVIDER_H_

#include "cherryUiDll.h"

#include "cherryShell.h"
#include <cherryMacros.h>

namespace cherry {

/**
 * Interface for objects that can return a shell. This is normally used for
 * opening child windows. An object that wants to open child shells can take
 * an IShellProvider in its constructor, and the object that implements IShellProvider
 * can dynamically choose where child shells should be opened.
 *
 * @since 3.1
 */
struct CHERRY_UI IShellProvider : public virtual Object {

  cherryClassMacro(IShellProvider);

    /**
     * Returns the current shell (or null if none). This return value may
     * change over time, and should not be cached.
     *
     * @return the current shell or null if none
     */
   virtual Shell::Pointer GetShell() = 0;
};

}

#endif /* CHERRYISHELLPROVIDER_H_ */
