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

#ifndef CHERRYSYSTEMBUNDLE_H_
#define CHERRYSYSTEMBUNDLE_H_

#include "cherryBundle.h"

namespace cherry {

class BundleLoader;

class SystemBundle : public Bundle
{
  
public:
 
  cherryObjectMacro(SystemBundle)
  
  SystemBundle(BundleLoader& loader, IBundleStorage::Pointer storage);
  
  void Start();
  
  void Resume();
  
  BundleLoader& GetBundleLoader();
};

}

#endif /*CHERRYSYSTEMBUNDLE_H_*/
