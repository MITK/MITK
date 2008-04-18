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

#ifndef CHERRYIEDITORINPUT_H_
#define CHERRYIEDITORINPUT_H_

#include <org.opencherry.osgi/cherryMacros.h>

namespace cherry {

struct CHERRY_API IEditorInput : public Object
{
  cherryClassMacro(IEditorInput);
};

}

#endif /*CHERRYIEDITORINPUT_H_*/
