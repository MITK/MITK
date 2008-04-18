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

#ifndef CHERRYIEXTENSIONPOINT_H_
#define CHERRYIEXTENSIONPOINT_H_

#include "../cherryDll.h"

#include "cherryIExtension.h"

#include <vector>

namespace cherry {

class IBundle;

struct CHERRY_API IExtensionPoint
{

public:
  
  virtual ~IExtensionPoint() {};
  
  virtual std::string GetContributor() const = 0;
  virtual const IExtension* GetExtension(const std::string& extensionId) const = 0;
  virtual const std::vector<const IExtension*> GetExtensions() const = 0;
  virtual std::string GetLabel() const = 0;
  virtual std::string GetSimpleIdentifier() const = 0;
  virtual std::string GetUniqueIdentifier() const = 0;
  
};

}

#endif /*CHERRYIEXTENSIONPOINT_H_*/
