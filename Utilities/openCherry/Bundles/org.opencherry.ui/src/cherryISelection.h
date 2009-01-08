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

#ifndef CHERRYISELECTION_H_
#define CHERRYISELECTION_H_

#include <cherryMacros.h>
#include <cherryObject.h>

#include "cherryUiDll.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 **/
struct CHERRY_UI ISelection : public Object
{
  cherryClassMacro(ISelection);

  virtual bool IsEmpty() const = 0;

  virtual ~ISelection() {}
};

}

#endif /*CHERRYISELECTION_H_*/
