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

#ifndef BERRYISHOWVIEWDIALOG_H_
#define BERRYISHOWVIEWDIALOG_H_

#include "berryIDialog.h"

#include <vector>
#include "../berryIViewDescriptor.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 */
struct BERRY_UI IShowViewDialog : public IDialog
{
  osgiInterfaceMacro(berry::IShowViewDialog);


  virtual std::vector<IViewDescriptor::Pointer> GetSelection() = 0;

};

}

#endif /*BERRYISHOWVIEWDIALOG_H_*/
