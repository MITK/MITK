/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYIPRESENTABLEPARTLIST_H_
#define BERRYIPRESENTABLEPARTLIST_H_

#include <berryIPresentablePart.h>
#include <vector>

namespace berry
{

struct IPresentablePartList
{

  virtual ~IPresentablePartList()
  {
  }

  virtual void Insert(IPresentablePart::Pointer part, int idx) = 0;

  virtual void Remove(IPresentablePart::Pointer part) = 0;

  virtual void Move(IPresentablePart::Pointer part, int newIndex) = 0;

  virtual std::size_t Size() = 0;

  virtual void Select(IPresentablePart::Pointer part) = 0;

  virtual std::vector<IPresentablePart::Pointer> GetPartList() = 0;
};

}

#endif /* BERRYIPRESENTABLEPARTLIST_H_ */
