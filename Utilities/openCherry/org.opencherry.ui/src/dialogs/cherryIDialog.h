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

#include <org.opencherry.osgi/cherryMacros.h>

#include "../cherryUiDll.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 */
struct CHERRY_UI IDialog : public Object
{
  cherryClassMacro(IDialog);
  
  enum ReturnCode { OK, CANCEL };
};

}
#endif /*CHERRYIDIALOG_H_*/
