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

#ifndef __BERRY_CATEGORY_TXX__
#define __BERRY_CATEGORY_TXX__

#include "berryWorkbenchRegistryConstants.h"
#include "../berryUIException.h"

#include <QStringList>

namespace berry
{

template<class T>
const QString Category<T>::MISC_NAME = "Other";

template<class T>
const QString Category<T>::MISC_ID =
    "org.blueberry.ui.internal.otherCategory";

template<class T> Category<T>::Category()
{
  this->id = MISC_ID;
  this->name = MISC_NAME;
}

template<class T> Category<T>::Category(const QString& ID,
    const QString& label)
 : id(ID), name(label)
{
}

template<class T>
Category<T>::Category(IConfigurationElement::Pointer configElement)
 : configurationElement(configElement) {

  QString id = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID);

  if (id == "" || GetLabel() == "")
  {
    throw WorkbenchException(QString("Invalid category: ") + id);
  }
}

template<class T>
void Category<T>::AddElement(ElementType element)
{
  elements.push_back(element);
}

template<class T>
Object* Category<T>::GetAdapter(const QString& adapter)
{
  if (adapter == qobject_interface_iid<IConfigurationElement*>())
  {
    return configurationElement.GetPointer();
  }
  else
  {
    return NULL;
  }
}

//template<class T>
//ImageDescriptor Category<T>::GetImageDescriptor()
//{
//  return WorkbenchImages.getImageDescriptor(ISharedImages.IMG_OBJ_FOLDER);
//}

template<class T>
const QString& Category<T>::GetId() const
{
  return id;
}

template<class T>
QString Category<T>::GetLabel() const
{
  if (configurationElement.IsNull())
    return name;

  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);
}

template<class T>
QList<QString> Category<T>::GetParentPath()
{
  if (parentPath.size() > 0)
  {
    return parentPath;
  }

  QString unparsedPath(this->GetRawParentPath());
  foreach(QString token, unparsedPath.split('/', QString::SkipEmptyParts))
  {
    parentPath.push_back(token.trimmed());
  }

  return parentPath;
}

template<class T>
QString Category<T>::GetRawParentPath() const
{
  if (configurationElement.IsNull())
    return QString();

  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_PARENT_CATEGORY);
}

template<class T>
QString Category<T>::GetRootPath()
{
  if (this->GetParentPath().size() > 0)
  {
    return GetParentPath()[0];
  }

  return id;
}

template<class T>
const QList<T>& Category<T>::GetElements() const
{
  return elements;
}

template<class T>
bool Category<T>::HasElement(const ElementType& o) const
{
  if (elements.empty())
  {
    return false;
  }

  for (typename QList<ElementType>::const_iterator iter = elements.begin(); iter != elements.end(); ++iter)
  {
    if (*iter == o) return true;
  }

  return false;
}

template<class T>
bool Category<T>::HasElements() const
{
  return !elements.empty();
}

template<class T>
T* Category<T>::GetParent(const ElementType& o)
{
  return 0;
}

template<class T>
void Category<T>::Clear()
{
  elements.clear();
}

} // namespace berry

#endif // __BERRY_CATEGORY_TXX__
