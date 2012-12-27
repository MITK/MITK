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

#ifndef BERRYIDIALOG_H_
#define BERRYIDIALOG_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_Export.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 */
struct BERRY_UI IDialog : public virtual Object
{
  berryInterfaceMacro(IDialog, berry);

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

  ~IDialog();

  virtual int Open() = 0;

};

}

#endif /*BERRYIDIALOG_H_*/
