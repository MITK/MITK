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

#ifndef BERRYIDIALOG_H_
#define BERRYIDIALOG_H_

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>

#include "../berryUiDll.h"

namespace berry {

using namespace osgi::framework;

/**
 * \ingroup org_blueberry_ui
 *
 */
struct BERRY_UI IDialog : public virtual Object
{
  osgiInterfaceMacro(berry::IDialog);

  /**
   * Constant for a dialog with no image (value 0).
   */
  const static int NONE; // = 0;

  /**
   * Constant for a dialog with an error image (value 1).
   */
  const static int ERR; // = 1;

  /**
   * Constant for a dialog with an info image (value 2).
   */
  const static int INFORMATION; // = 2;

  /**
   * Constant for a dialog with a question image (value 3).
   */
  const static int QUESTION; // = 3;

  /**
   * Constant for a dialog with a warning image (value 4).
   */
  const static int WARNING; // = 4;

  enum ReturnCode { OK, CANCEL };

  virtual int Open() = 0;
};

}

#endif /*BERRYIDIALOG_H_*/
