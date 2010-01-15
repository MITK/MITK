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


#ifndef OBJECTMAP_H_
#define OBJECTMAP_H_

#include "osgi/framework/Macros.h"

#include <map>

namespace osgi {

namespace framework {

template<typename Key, typename T, class Cmp = std::less<Key>,
    class A = std::allocator<std::pair<const Key, T> > >
class ObjectMap : public Object, public std::map<Key, T, Cmp, A>
{
public:

  typedef osgi::framework::ObjectMap<Key,T,Cmp,A> Self;
  osgiObjectMacroT(osgi::framework::ObjectMap<Key T Cmp A>);

  ObjectMap(const Self& o) : Object(), std::map<Key,T,Cmp,A>(o) {}

};

}
}

#endif /* OBJECTMAP_H_ */
