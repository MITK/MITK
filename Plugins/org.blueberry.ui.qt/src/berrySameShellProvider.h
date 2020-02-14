/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSAMESHELLPROVIDER_H_
#define BERRYSAMESHELLPROVIDER_H_

#include <org_blueberry_ui_qt_Export.h>
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
class BERRY_UI_QT SameShellProvider : public IShellProvider {

private:

  QWidget* targetControl;
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
  SameShellProvider(QWidget* targetControl);

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
  Shell::Pointer GetShell() const override;

};

}

#endif /* BERRYSAMESHELLPROVIDER_H_ */
