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

#ifndef CHERRYPROPERTYCACHE_H_
#define CHERRYPROPERTYCACHE_H_

#include "Poco/LRUCache.h"

#include "cherryProperty.h"

namespace cherry {

class PropertyCache {
  
private:
  Poco::LRUCache<Property::Pointer, Property> fCache;
  
public:
  
  PropertyCache(const int cacheSize) : fCache(cacheSize) {
    
  }
  
  Property::Pointer Get(Property::Pointer key) {
    return fCache.get(key);
  }
  
  void Put(Property::Pointer method) {
    fCache.add(method, method);
  }
  
  void Remove(Property::Pointer method) {
    fCache.remove(method);
  }
};

} // namespace cherry

#endif /*CHERRYPROPERTYCACHE_H_*/
