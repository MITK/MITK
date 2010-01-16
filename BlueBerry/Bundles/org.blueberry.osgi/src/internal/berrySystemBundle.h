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

#ifndef BERRYSYSTEMBUNDLE_H_
#define BERRYSYSTEMBUNDLE_H_

#include "berryBundle.h"

namespace berry {

class BundleLoader;

class SystemBundle : public Bundle
{
  
public:
 
  berryObjectMacro(SystemBundle)
  
  SystemBundle(BundleLoader& loader, IBundleStorage::Pointer storage);
  
  void Start();
  
  void Resume();
  
  BundleLoader& GetBundleLoader();
};

}

#endif /*BERRYSYSTEMBUNDLE_H_*/
