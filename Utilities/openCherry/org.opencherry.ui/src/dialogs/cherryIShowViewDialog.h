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

#ifndef CHERRYISHOWVIEWDIALOG_H_
#define CHERRYISHOWVIEWDIALOG_H_

#include "cherryIDialog.h"

#include <vector>
#include "../cherryIViewDescriptor.h"

namespace cherry {

struct CHERRY_UI IShowViewDialog : public IDialog
{
  cherryClassMacro(IShowViewDialog);
  
  virtual int Exec() = 0;
  
  virtual std::vector<IViewDescriptor::Pointer> GetSelection() = 0;
  
  virtual ~IShowViewDialog() {}
};

}

#endif /*CHERRYISHOWVIEWDIALOG_H_*/
