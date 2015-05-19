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
