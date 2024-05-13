/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROI.h>

void mitk::to_json(nlohmann::json& j, const ROI::Element& roi)
{
  j["ID"] = roi.GetID();

  if (roi.HasTimeSteps())
  {
    auto timeSteps = roi.GetTimeSteps();

    for (const auto t : timeSteps)
    {
      nlohmann::json jTimeStep = {
        { "t", t },
        { "Min", roi.GetMin(t) },
        { "Max", roi.GetMax(t) }
      };

      if (auto* properties = roi.GetProperties(t); properties != nullptr && !properties->IsEmpty())
        properties->ToJSON(jTimeStep["Properties"]);

      j["TimeSteps"].push_back(jTimeStep);
    }
  }
  else
  {
    j["Min"] = roi.GetMin();
    j["Max"] = roi.GetMax();
  }

  if (auto* properties = roi.GetDefaultProperties(); properties != nullptr && !properties->IsEmpty())
    properties->ToJSON(j["Properties"]);
}

void mitk::from_json(const nlohmann::json& j, ROI::Element& roi)
{
  auto id = j["ID"].get<unsigned int>();
  roi.SetID(id);

  if (j.contains("TimeSteps"))
  {
    for (const auto& jTimeStep : j["TimeSteps"])
    {
      auto t = jTimeStep["t"].get<TimeStepType>();

      roi.SetMin(jTimeStep["Min"].get<Point3D>(), t);
      roi.SetMax(jTimeStep["Max"].get<Point3D>(), t);

      if (jTimeStep.contains("Properties"))
      {
        auto properties = mitk::PropertyList::New();
        properties->FromJSON(jTimeStep["Properties"]);
        roi.SetProperties(properties, t);
      }
    }
  }
  else
  {
    roi.SetMin(j["Min"].get<Point3D>());
    roi.SetMax(j["Max"].get<Point3D>());
  }

  if (j.contains("Properties"))
  {
    auto properties = mitk::PropertyList::New();
    properties->FromJSON(j["Properties"]);
    roi.SetDefaultProperties(properties);
  }
}

mitk::ROI::Element::Element()
  : Element(0)
{
}

mitk::ROI::Element::Element(unsigned int id)
  : m_ID(id),
    m_DefaultProperties(PropertyList::New())
{
}

unsigned int mitk::ROI::Element::GetID() const
{
  return m_ID;
}

void mitk::ROI::Element::SetID(unsigned int id)
{
  m_ID = id;
}

bool mitk::ROI::Element::HasTimeStep(TimeStepType t) const
{
  return m_Min.count(t) != 0 && m_Max.count(t) != 0;
}

bool mitk::ROI::Element::HasTimeSteps() const
{
  // Check for multiple time steps.
  if (m_Min.size() > 1 && m_Max.size() > 1)
    return true;

  // Check for single time step that is not 0.
  if (m_Min.size() >= 1 && m_Max.size() >= 1)
    return m_Min.count(0) == 0 && m_Max.count(0) == 0;

  // Single time step 0.
  return false;
}

std::vector<mitk::TimeStepType> mitk::ROI::Element::GetTimeSteps() const
{
  std::vector<TimeStepType> result;
  result.reserve(m_Min.size());

  for (const auto& [t, min] : m_Min)
  {
    if (m_Max.count(t) != 0)
      result.push_back(t);
  }

  return result;
}

mitk::Point3D mitk::ROI::Element::GetMin(TimeStepType t) const
{
  return m_Min.at(t);
}

void mitk::ROI::Element::SetMin(const Point3D& min, TimeStepType t)
{
  m_Min[t] = min;
}

mitk::Point3D mitk::ROI::Element::GetMax(TimeStepType t) const
{
  return m_Max.at(t);
}

void mitk::ROI::Element::SetMax(const Point3D& max, TimeStepType t)
{
  m_Max[t] = max;
}

mitk::PropertyList* mitk::ROI::Element::GetDefaultProperties() const
{
  return m_DefaultProperties;
}

void mitk::ROI::Element::SetDefaultProperties(PropertyList* properties)
{
  m_DefaultProperties = properties;
}

mitk::PropertyList* mitk::ROI::Element::GetProperties(TimeStepType t) const
{
  if (m_Properties.count(t) != 0)
    return m_Properties.at(t);

  return nullptr;
}

void mitk::ROI::Element::SetProperties(PropertyList* properties, TimeStepType t)
{
  m_Properties[t] = properties;
}

mitk::BaseProperty::ConstPointer mitk::ROI::Element::GetConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext) const
{
  return !contextName.empty()
    ? this->GetConstProperty(propertyKey, std::stoul(contextName), fallBackOnDefaultContext)
    : m_DefaultProperties->GetConstProperty(propertyKey);
}

mitk::BaseProperty::ConstPointer mitk::ROI::Element::GetConstProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext) const
{
  auto it = m_Properties.find(t);

  if (it != m_Properties.end() && it->second.IsNotNull())
  {
    auto property = it->second->GetConstProperty(propertyKey);

    if (property.IsNotNull())
      return property;
  }

  if (!fallBackOnDefaultContext)
    return nullptr;

  return m_DefaultProperties->GetConstProperty(propertyKey);
}

std::vector<std::string> mitk::ROI::Element::GetPropertyKeys(const std::string& contextName, bool includeDefaultContext) const
{
  return !contextName.empty()
    ? this->GetPropertyKeys(std::stoul(contextName), includeDefaultContext)
    : m_DefaultProperties->GetPropertyKeys();
}

std::vector<std::string> mitk::ROI::Element::GetPropertyKeys(TimeStepType t, bool includeDefaultContext) const
{
  auto it = m_Properties.find(t);

  std::vector<std::string> result;

  if (it != m_Properties.end() && it->second.IsNotNull())
    result = it->second->GetPropertyKeys();

  if (includeDefaultContext)
  {
    auto keys = m_DefaultProperties->GetPropertyKeys();
    auto end = result.cend();

    std::remove_copy_if(keys.cbegin(), keys.cend(), std::back_inserter(result), [&, result, end](const std::string& key) {
      return end != std::find(result.cbegin(), end, key);
    });
  }

  return result;
}

std::vector<std::string> mitk::ROI::Element::GetPropertyContextNames() const
{
  std::vector<std::string> result;
  result.reserve(m_Properties.size());

  std::transform(m_Properties.cbegin(), m_Properties.cend(), std::back_inserter(result), [](const PropertyListsType::value_type& property) {
    return std::to_string(property.first);
  });

  return result;
}

mitk::BaseProperty* mitk::ROI::Element::GetNonConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext)
{
  return !contextName.empty()
    ? this->GetNonConstProperty(propertyKey, std::stoul(contextName), fallBackOnDefaultContext)
    : m_DefaultProperties->GetNonConstProperty(propertyKey);
}

mitk::BaseProperty* mitk::ROI::Element::GetNonConstProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext)
{
  auto it = m_Properties.find(t);

  if (it != m_Properties.end() && it->second.IsNotNull())
  {
    auto* property = it->second->GetNonConstProperty(propertyKey);

    if (property != nullptr)
      return property;
  }

  if (!fallBackOnDefaultContext)
    return nullptr;

  return m_DefaultProperties->GetNonConstProperty(propertyKey);
}

void mitk::ROI::Element::SetProperty(const std::string& propertyKey, BaseProperty* property, const std::string& contextName, bool fallBackOnDefaultContext)
{
  if (!contextName.empty())
  {
    this->SetProperty(propertyKey, property, std::stoul(contextName), fallBackOnDefaultContext);
    return;
  }

  m_DefaultProperties->SetProperty(propertyKey, property);
}

void mitk::ROI::Element::SetProperty(const std::string& propertyKey, BaseProperty* property, TimeStepType t, bool fallBackOnDefaultContext)
{
  auto it = m_Properties.find(t);

  if (it != m_Properties.end() && it->second.IsNotNull())
  {
    it->second->SetProperty(propertyKey, property);
    return;
  }

  if (!fallBackOnDefaultContext)
    mitkThrow() << "Time step " << t << " does not exist!";

  m_DefaultProperties->SetProperty(propertyKey, property);
}

void mitk::ROI::Element::RemoveProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext)
{
  if (!contextName.empty())
  {
    this->RemoveProperty(propertyKey, std::stoul(contextName), fallBackOnDefaultContext);
    return;
  }

  m_DefaultProperties->RemoveProperty(propertyKey);
}

void mitk::ROI::Element::RemoveProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext)
{
  auto it = m_Properties.find(t);

  if (it != m_Properties.end() && it->second.IsNotNull())
  {
    it->second->RemoveProperty(propertyKey);
    return;
  }

  if (!fallBackOnDefaultContext)
    mitkThrow() << "Time step " << t << " does not exist!";

  m_DefaultProperties->RemoveProperty(propertyKey);
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

void mitk::ROI::AddElement(const Element& element)
{
  const auto id = element.GetID();

  if (m_Elements.count(id) != 0)
    mitkThrow() << "ROI already contains an element with ID " << std::to_string(id) << '.';

  m_Elements[id] = element;
}

const mitk::ROI::Element& mitk::ROI::GetElement(unsigned int id) const
{
  return m_Elements.at(id);
}

mitk::ROI::Element& mitk::ROI::GetElement(unsigned int id)
{
  return m_Elements.at(id);
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

void mitk::ROI::SetRequestedRegion(const itk::DataObject*)
{
}
