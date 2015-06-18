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

#include "berryQtItemSelection.h"

#include "berryQModelIndexObject.h"

namespace berry {

QtItemSelection::QtItemSelection()
{

}

QtItemSelection::QtItemSelection(const QItemSelection& sel)
: m_Selection(new ContainerType())
{
  QModelIndexList indexes = sel.indexes();
  for (QModelIndexList::const_iterator index = indexes.constBegin(); index != indexes.constEnd(); ++index)
  {
    Object::Pointer indexObj(new QModelIndexObject(*index));
    m_Selection->push_back(indexObj);
  }
}

QItemSelection QtItemSelection::GetQItemSelection() const
{
  return m_QItemSelection;
}

bool QtItemSelection::IsEmpty() const
{
  return m_Selection->empty();
}

Object::Pointer QtItemSelection::GetFirstElement() const
{
  if (m_Selection->empty()) return Object::Pointer();

  return *(m_Selection->begin());
}

  QtItemSelection::iterator QtItemSelection::Begin() const
  {
    return m_Selection->begin();
  }

  QtItemSelection::iterator QtItemSelection::End() const
  {
    return m_Selection->end();
  }

  int QtItemSelection::Size() const
  {
    return (int) m_Selection->size();
  }

  QtItemSelection::ContainerType::Pointer QtItemSelection::ToVector() const
  {
    return m_Selection;
  }

  bool QtItemSelection::operator==(const Object* obj) const
  {
    if (const IStructuredSelection* other = dynamic_cast<const IStructuredSelection*>(obj))
    {
      return m_Selection == other->ToVector();
    }

    return false;
  }

}
