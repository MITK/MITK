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

  virtual std::size_t Size() = 0;

  virtual void Select(IPresentablePart::Pointer part) = 0;

  virtual std::vector<IPresentablePart::Pointer> GetPartList() = 0;
};

}

#endif /* BERRYIPRESENTABLEPARTLIST_H_ */
