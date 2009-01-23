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


#ifndef CHERRYSAMESHELLPROVIDER_H_
#define CHERRYSAMESHELLPROVIDER_H_

#include "cherryUiDll.h"
#include "cherryShell.h"

#include <cherryMacros.h>

#include "cherryIShellProvider.h"

namespace cherry {

/**
 * Standard shell provider that always returns the shell containing the given
 * control. This will always return the correct shell for the control, even if
 * the control is reparented.
 *
 * @since 3.1
 */
class CHERRY_UI SameShellProvider : public IShellProvider {

private:

  void* targetControl;
  Shell::Pointer shell;

public:

  cherryObjectMacro(SameShellProvider);

  /**
   * Returns a shell provider that always returns the current
   * shell for the given control.
   *
   * @param targetControl control whose shell will be tracked, or null if getShell() should always
   * return null
   */
  SameShellProvider(void* targetControl);

  /**
   * Returns a shell provider that always returns the given shell.
   *
   * @param shell the shell which should always be returned,
   *        or null if GetShell() should alway return null
   */
  SameShellProvider(Shell::Pointer shell);

  /* (non-javadoc)
   * @see IShellProvider#getShell()
   */
  Shell::Pointer GetShell();

};

}

#endif /* CHERRYSAMESHELLPROVIDER_H_ */
