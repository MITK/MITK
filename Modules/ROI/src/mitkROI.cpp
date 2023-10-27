/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROI.h>

mitk::ROI::Element::Element()
  : Properties(PropertyList::New())
{
}

mitk::BaseProperty::ConstPointer mitk::ROI::Element::GetConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext) const
{
  return Properties->GetConstProperty(propertyKey, contextName, fallBackOnDefaultContext);
}

std::vector<std::string> mitk::ROI::Element::GetPropertyKeys(const std::string& contextName, bool includeDefaultContext) const
{
  return Properties->GetPropertyKeys(contextName, includeDefaultContext);
}

std::vector<std::string> mitk::ROI::Element::GetPropertyContextNames() const
{
  return Properties->GetPropertyContextNames();
}

mitk::BaseProperty* mitk::ROI::Element::GetNonConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext)
{
  return Properties->GetNonConstProperty(propertyKey, contextName, fallBackOnDefaultContext);
}

void mitk::ROI::Element::SetProperty(const std::string& propertyKey, BaseProperty* property, const std::string& contextName, bool fallBackOnDefaultContext)
{
  Properties->SetProperty(propertyKey, property, contextName, fallBackOnDefaultContext);
}

void mitk::ROI::Element::RemoveProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext)
{
  Properties->RemoveProperty(propertyKey, contextName, fallBackOnDefaultContext);
}

mitk::ROI::ROI()
{
}

mitk::ROI::ROI(const Self& other)
  : BaseData(other)
{
}

mitk::ROI::~ROI()
{
}

size_t mitk::ROI::GetNumberOfElements() const
{
  return m_Elements.size();
}

size_t mitk::ROI::AddElement(const Element& element)
{
  m_Elements.push_back(element);
  return m_Elements.size() - 1;
}

const mitk::ROI::Element* mitk::ROI::GetElement(size_t index) const
{
  return &m_Elements.at(index);
}

mitk::ROI::Element* mitk::ROI::GetElement(size_t index)
{
  return &m_Elements.at(index);
}

mitk::ROI::ConstIterator mitk::ROI::begin() const
{
  return m_Elements.begin();
}

mitk::ROI::ConstIterator mitk::ROI::end() const
{
  return m_Elements.end();
}

mitk::ROI::Iterator mitk::ROI::begin()
{
  return m_Elements.begin();
}

mitk::ROI::Iterator mitk::ROI::end()
{
  return m_Elements.end();
}

void mitk::ROI::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::ROI::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::ROI::VerifyRequestedRegion()
{
  return true;
}

void mitk::ROI::SetRequestedRegion(const itk::DataObject* data)
{
}
