/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(berry::IStickyViewManager);

  ~IStickyViewManager() override;

  virtual void Remove(const QString& perspectiveId) = 0;

  virtual void Add(const QString& perspectiveId, const std::set<QString>& stickyViewSet) = 0;

  virtual void Clear() = 0;

  virtual void Update(SmartPointer<Perspective> oldPersp, SmartPointer<Perspective> newPersp) = 0;

  virtual void Save(IMemento::Pointer memento) = 0;

  virtual void Restore(IMemento::Pointer memento) = 0;

};

}

#endif /* BERRYISTICKYVIEWMANAGER_H_ */
