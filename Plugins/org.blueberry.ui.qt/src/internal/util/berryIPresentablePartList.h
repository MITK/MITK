/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPRESENTABLEPARTLIST_H_
#define BERRYIPRESENTABLEPARTLIST_H_

#include <berryIPresentablePart.h>
#include <vector>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

struct BERRY_UI_QT IPresentablePartList
{

  virtual ~IPresentablePartList();

  virtual void Insert(IPresentablePart::Pointer part, int idx) = 0;

  virtual void Remove(IPresentablePart::Pointer part) = 0;

  virtual void Move(IPresentablePart::Pointer part, int newIndex) = 0;

  virtual int Size() = 0;

  virtual void Select(IPresentablePart::Pointer part) = 0;

  virtual QList<IPresentablePart::Pointer> GetPartList() = 0;
};

}

#endif /* BERRYIPRESENTABLEPARTLIST_H_ */
