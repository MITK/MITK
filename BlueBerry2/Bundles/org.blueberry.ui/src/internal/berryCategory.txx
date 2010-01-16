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

#ifndef __BERRY_CATEGORY_TXX__
#define __BERRY_CATEGORY_TXX__

#include "berryWorkbenchRegistryConstants.h"
#include "../berryUIException.h"

#include <Poco/StringTokenizer.h>

namespace berry
{

template<class T>
const std::string Category<T>::MISC_NAME = "Other";

template<class T>
const std::string Category<T>::MISC_ID =
    "org.blueberry.ui.internal.otherCategory"; //$NON-NLS-1$

template<class T> Category<T>::Category()
{
  this->id = MISC_ID;
  this->name = MISC_NAME;
}

template<class T> Category<T>::Category(const std::string& ID,
    const std::string& label)
 : id(ID), name(label) {

}

template<class T>
Category<T>::Category(IConfigurationElement::Pointer configElement)
 : configurationElement(configElement) {

  std::string id;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id);

  if (id == "" || GetLabel() == "")
  {
    throw WorkbenchException("Invalid category", id);
  }
}

template<class T>
void Category<T>::AddElement(ElementType element)
{
  elements.push_back(element);
}

template<class T>
Poco::Any Category<T>::GetAdapter(const std::string& adapter)
{
  if (adapter == IConfigurationElement::GetStaticClassName())
  {
    return Poco::Any(configurationElement);
  }
  else
  {
    return Poco::Any();
  }
}

//template<class T>
//ImageDescriptor Category<T>::GetImageDescriptor()
//{
//  return WorkbenchImages.getImageDescriptor(ISharedImages.IMG_OBJ_FOLDER);
//}

template<class T>
const std::string& Category<T>::GetId() const
{
  return id;
}

template<class T>
std::string Category<T>::GetLabel() const
{
  if (configurationElement.IsNull())
    return name;

  std::string val;
  configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, val);
  return val;
}

template<class T>
const std::vector<std::string>& Category<T>::GetParentPath()
{
  if (parentPath.size() > 0)
  {
    return parentPath;
  }

  std::string unparsedPath(this->GetRawParentPath());
  Poco::StringTokenizer stok(unparsedPath, "/", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM); //$NON-NLS-1$
  for (Poco::StringTokenizer::Iterator iter = stok.begin(); iter != stok.end(); ++iter)
  {
    parentPath.push_back(*iter);
  }

  return parentPath;
}

template<class T>
std::string Category<T>::GetRawParentPath() const
{
  if (configurationElement.IsNull())
    return "";

  std::string raw;
  configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_PARENT_CATEGORY, raw);
  return raw;
}

template<class T>
std::string Category<T>::GetRootPath()
{
  if (this->GetParentPath().size() > 0)
  {
    return GetParentPath()[0];
  }

  return id;
}

template<class T>
const std::vector<T>& Category<T>::GetElements() const
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

  for (typename std::vector<ElementType>::const_iterator iter = elements.begin(); iter != elements.end(); ++iter)
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
