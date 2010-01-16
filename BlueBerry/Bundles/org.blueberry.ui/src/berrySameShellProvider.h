/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYSAMESHELLPROVIDER_H_
#define BERRYSAMESHELLPROVIDER_H_

#include "berryUiDll.h"
#include "berryShell.h"

#include <berryMacros.h>

#include "berryIShellProvider.h"

namespace berry {

/**
 * Standard shell provider that always returns the shell containing the given
 * control. This will always return the correct shell for the control, even if
 * the control is reparented.
 *
 * @since 3.1
 */
class BERRY_UI SameShellProvider : public IShellProvider {

private:

  void* targetControl;
  Shell::Pointer shell;

public:

  berryObjectMacro(SameShellProvider);

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

#endif /* BERRYSAMESHELLPROVIDER_H_ */
