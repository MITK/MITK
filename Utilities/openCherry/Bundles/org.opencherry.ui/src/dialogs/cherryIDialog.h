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

#ifndef CHERRYIDIALOG_H_
#define CHERRYIDIALOG_H_

#include <cherryMacros.h>
#include <cherryObject.h>

#include "../cherryUiDll.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 */
struct CHERRY_UI IDialog : public virtual Object
{
  cherryInterfaceMacro(IDialog, cherry);

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

#endif /*CHERRYIDIALOG_H_*/
