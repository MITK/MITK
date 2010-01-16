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
