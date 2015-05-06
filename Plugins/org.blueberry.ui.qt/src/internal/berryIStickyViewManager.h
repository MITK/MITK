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

struct BERRY_UI_QT IStickyViewManager : public Object
{

  berryObjectMacro(berry::IStickyViewManager)

  ~IStickyViewManager();

  virtual void Remove(const QString& perspectiveId) = 0;

  virtual void Add(const QString& perspectiveId, const std::set<QString>& stickyViewSet) = 0;

  virtual void Clear() = 0;

  virtual void Update(SmartPointer<Perspective> oldPersp, SmartPointer<Perspective> newPersp) = 0;

  virtual void Save(IMemento::Pointer memento) = 0;

  virtual void Restore(IMemento::Pointer memento) = 0;

};

}

#endif /* BERRYISTICKYVIEWMANAGER_H_ */
