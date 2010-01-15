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


#ifndef CHERRYISTICKYVIEWMANAGER_H_
#define CHERRYISTICKYVIEWMANAGER_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include <cherryIMemento.h>

#include <set>

namespace cherry {

class Perspective;

struct IStickyViewManager : public Object {

  cherryInterfaceMacro(IStickyViewManager, cherry)

  virtual void Remove(const std::string& perspectiveId) = 0;

  virtual void Add(const std::string& perspectiveId, const std::set<std::string>& stickyViewSet) = 0;

  virtual void Clear() = 0;

  virtual void Update(SmartPointer<Perspective> oldPersp, SmartPointer<Perspective> newPersp) = 0;

  virtual void Save(IMemento::Pointer memento) = 0;

  virtual void Restore(IMemento::Pointer memento) = 0;

};

}

#endif /* CHERRYISTICKYVIEWMANAGER_H_ */
