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

#ifndef CHERRYIVIEWREGISTRY_H_
#define CHERRYIVIEWREGISTRY_H_

#include "cherryUiDll.h"

#include "cherryIViewDescriptor.h"
#include "cherryIViewCategory.h"

#include <vector>

namespace cherry {

struct CHERRY_UI IViewRegistry
{
  
  virtual IViewDescriptor::Pointer Find(const std::string& id) const = 0;
  virtual std::vector<IViewCategory::Pointer> GetCategories() = 0;
  virtual const std::vector<IViewDescriptor::Pointer>& GetViews() const = 0;

  virtual ~IViewRegistry() {}
};

}

#endif /*CHERRYIVIEWREGISTRY_H_*/
