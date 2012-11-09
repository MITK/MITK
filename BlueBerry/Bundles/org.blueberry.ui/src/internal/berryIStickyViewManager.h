/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYISTICKYVIEWMANAGER_H_
#define BERRYISTICKYVIEWMANAGER_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <berryIMemento.h>

#include <set>

namespace berry {

class Perspective;

struct BERRY_UI IStickyViewManager : public Object {

  berryInterfaceMacro(IStickyViewManager, berry)

  ~IStickyViewManager();

  virtual void Remove(const std::string& perspectiveId) = 0;

  virtual void Add(const std::string& perspectiveId, const std::set<std::string>& stickyViewSet) = 0;

  virtual void Clear() = 0;

  virtual void Update(SmartPointer<Perspective> oldPersp, SmartPointer<Perspective> newPersp) = 0;

  virtual void Save(IMemento::Pointer memento) = 0;

  virtual void Restore(IMemento::Pointer memento) = 0;

};

}

#endif /* BERRYISTICKYVIEWMANAGER_H_ */
