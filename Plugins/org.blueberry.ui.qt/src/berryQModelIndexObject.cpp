/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQModelIndexObject.h"

namespace berry
{

QModelIndexObject::QModelIndexObject(const QModelIndex& index) :
  m_QModelIndex(index)
{

}

const QModelIndex& QModelIndexObject::GetQModelIndex() const
{
  return m_QModelIndex;
}

bool QModelIndexObject::operator==(const Object* obj) const
{
  if (const QModelIndexObject* other = dynamic_cast<const QModelIndexObject*>(obj))
  {
    return m_QModelIndex == other->m_QModelIndex;
  }

  return false;
}

}
