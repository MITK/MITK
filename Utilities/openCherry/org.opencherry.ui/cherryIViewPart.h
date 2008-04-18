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

#ifndef CHERRYIVIEWPART_H_
#define CHERRYIVIEWPART_H_

#include <org.opencherry.osgi/cherryDll.h>
#include "org.opencherry.osgi/cherryMacros.h"

#include "cherryIMemento.h"
#include "cherryIWorkbenchPart.h"
#include "cherryIWorkbenchPartSite.h"

namespace cherry {

struct CHERRY_API IViewPart : public virtual IWorkbenchPart
{
  
  cherryClassMacro(IViewPart);
      
  virtual void Init(IWorkbenchPartSite::Pointer site, IMemento* memento = 0) = 0;
  virtual void SaveState(IMemento* memento) = 0;
};

}

#endif /*CHERRYIVIEWPART_H_*/
