/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <iterator>

#include "mitkTemporoSpatialStringProperty.h"

#ifdef _MSC_VER
// has to be deactivated because of a bug in boost v1.59. see Boost bug ticket #11599
// as soon as MITK uses a boost version with a bug fix we can remove the disableling.
#pragma warning(push)
#pragma warning(disable : 4715)
#endif

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/type_traits/make_unsigned.hpp>

mitk::TemporoSpatialStringProperty::TemporoSpatialStringProperty(const char *s)
{
  if (s)
  {
    SliceMapType slices{{0, s}};

    m_Values.insert(std::make_pair(0, slices));
  }
}

mitk::TemporoSpatialStringProperty::TemporoSpatialStringProperty(const std::string &s)
{
  SliceMapType slices{{0, s}};

  m_Values.insert(std::make_pair(0, slices));
}

mitk::TemporoSpatialStringProperty::TemporoSpatialStringProperty(const TemporoSpatialStringProperty &other)
  : BaseProperty(other), m_Values(other.m_Values)
{
}

bool mitk::TemporoSpatialStringProperty::IsEqual(const BaseProperty &property) const
{
  return this->m_Values == static_cast<const Self &>(property).m_Values;
}

bool mitk::TemporoSpatialStringProperty::Assign(const BaseProperty &property)
{
  this->m_Values = static_cast<const Self &>(property).m_Values;
  return true;
}

std::string mitk::TemporoSpatialStringProperty::GetValueAsString() const
{
  return GetValue();
}

itk::LightObject::Pointer mitk::TemporoSpatialStringProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

mitk::TemporoSpatialStringProperty::ValueType mitk::TemporoSpatialStringProperty::GetValue() const
{
  std::string result = "";

  if (!m_Values.empty())
  {
    if (!m_Values.begin()->second.empty())
    {
      result = m_Values.begin()->second.begin()->second;
    }
  }
  return result;
};

std::pair<bool, mitk::TemporoSpatialStringProperty::ValueType> mitk::TemporoSpatialStringProperty::CheckValue(
  const TimeStepType &timeStep, const IndexValueType &zSlice, bool allowCloseTime, bool allowCloseSlice) const
{
  std::string value = "";
  bool found = false;

  auto timeIter = m_Values.find(timeStep);
  auto timeEnd = m_Values.end();
  if (timeIter == timeEnd && allowCloseTime)
  { // search for closest time step (earlier preverd)
    timeIter = m_Values.upper_bound(timeStep);
    if (timeIter != m_Values.begin())
    { // there is a key lower than time step
      timeIter = std::prev(timeIter);
    }
  }

  if (timeIter != timeEnd)
  {
    const SliceMapType &slices = timeIter->second;

    auto sliceIter = slices.find(zSlice);
    auto sliceEnd = slices.end();
    if (sliceIter == sliceEnd && allowCloseSlice)
    { // search for closest slice (earlier preverd)
      sliceIter = slices.upper_bound(zSlice);
      if (sliceIter != slices.begin())
      { // there is a key lower than slice
        sliceIter = std::prev(sliceIter);
      }
    }

    if (sliceIter != sliceEnd)
    {
      value = sliceIter->second;
      found = true;
    }
  }

  return std::make_pair(found, value);
};

mitk::TemporoSpatialStringProperty::ValueType mitk::TemporoSpatialStringProperty::GetValue(const TimeStepType &timeStep,
                                                                                           const IndexValueType &zSlice,
                                                                                           bool allowCloseTime,
                                                                                           bool allowCloseSlice) const
{
  return CheckValue(timeStep, zSlice, allowCloseTime, allowCloseSlice).second;
};

mitk::TemporoSpatialStringProperty::ValueType mitk::TemporoSpatialStringProperty::GetValueBySlice(
  const IndexValueType &zSlice, bool allowClose) const
{
  return GetValue(0, zSlice, true, allowClose);
};

mitk::TemporoSpatialStringProperty::ValueType mitk::TemporoSpatialStringProperty::GetValueByTimeStep(
  const TimeStepType &timeStep, bool allowClose) const
{
  return GetValue(timeStep, 0, allowClose, true);
};

bool mitk::TemporoSpatialStringProperty::HasValue() const
{
  return !m_Values.empty();
};

bool mitk::TemporoSpatialStringProperty::HasValue(const TimeStepType &timeStep,
                                                  const IndexValueType &zSlice,
                                                  bool allowCloseTime,
                                                  bool allowCloseSlice) const
{
  return CheckValue(timeStep, zSlice, allowCloseTime, allowCloseSlice).first;
};

bool mitk::TemporoSpatialStringProperty::HasValueBySlice(const IndexValueType &zSlice, bool allowClose) const
{
  return HasValue(0, zSlice, true, allowClose);
};

bool mitk::TemporoSpatialStringProperty::HasValueByTimeStep(const TimeStepType &timeStep, bool allowClose) const
{
  return HasValue(timeStep, 0, allowClose, true);
};

std::vector<mitk::TemporoSpatialStringProperty::IndexValueType> mitk::TemporoSpatialStringProperty::GetAvailableSlices(
  const TimeStepType &timeStep) const
{
  std::vector<IndexValueType> result;

  auto timeIter = m_Values.find(timeStep);
  auto timeEnd = m_Values.end();

  if (timeIter != timeEnd)
  {
    for (auto const &element : timeIter->second)
    {
      result.push_back(element.first);
    }
  }

  return result;
};

std::vector<mitk::TimeStepType> mitk::TemporoSpatialStringProperty::GetAvailableTimeSteps() const
{
  std::vector<mitk::TimeStepType> result;

  for (auto const &element : m_Values)
  {
    result.push_back(element.first);
  }

  return result;
};

void mitk::TemporoSpatialStringProperty::SetValue(const TimeStepType &timeStep,
                                                  const IndexValueType &zSlice,
                                                  const ValueType &value)
{
  auto timeIter = m_Values.find(timeStep);
  auto timeEnd = m_Values.end();

  if (timeIter == timeEnd)
  {
    SliceMapType slices{{zSlice, value}};
    m_Values.insert(std::make_pair(timeStep, slices));
  }
  else
  {
    timeIter->second[zSlice] = value;
  }
  this->Modified();
};

void mitk::TemporoSpatialStringProperty::SetValue(const ValueType &value)
{
  this->Modified();
  m_Values.clear();
  this->SetValue(0, 0, value);
};

// Create necessary escape sequences from illegal characters
// REMARK: This code is based upon code from boost::ptree::json_writer.
// The corresponding boost function was not used directly, because it is not part of
// the public interface of ptree::json_writer. :(
// A own serialization strategy was implemented instead of using boost::ptree::json_write because
// currently (<= boost 1.60) everything (even numbers) are converted into string representations
// by the writer, so e.g. it becomes "t":"2" instaed of "t":2
template <class Ch>
std::basic_string<Ch> CreateJSONEscapes(const std::basic_string<Ch> &s)
{
  std::basic_string<Ch> result;
  typename std::basic_string<Ch>::const_iterator b = s.begin();
  typename std::basic_string<Ch>::const_iterator e = s.end();
  while (b != e)
  {
    typedef typename boost::make_unsigned<Ch>::type UCh;
    UCh c(*b);
    // This assumes an ASCII superset.
    // We escape everything outside ASCII, because this code can't
    // handle high unicode characters.
    if (c == 0x20 || c == 0x21 || (c >= 0x23 && c <= 0x2E) || (c >= 0x30 && c <= 0x5B) || (c >= 0x5D && c <= 0x7F))
      result += *b;
    else if (*b == Ch('\b'))
      result += Ch('\\'), result += Ch('b');
    else if (*b == Ch('\f'))
      result += Ch('\\'), result += Ch('f');
    else if (*b == Ch('\n'))
      result += Ch('\\'), result += Ch('n');
    else if (*b == Ch('\r'))
      result += Ch('\\'), result += Ch('r');
    else if (*b == Ch('\t'))
      result += Ch('\\'), result += Ch('t');
    else if (*b == Ch('/'))
      result += Ch('\\'), result += Ch('/');
    else if (*b == Ch('"'))
      result += Ch('\\'), result += Ch('"');
    else if (*b == Ch('\\'))
      result += Ch('\\'), result += Ch('\\');
    else
    {
      const char *hexdigits = "0123456789ABCDEF";
      unsigned long u = (std::min)(static_cast<unsigned long>(static_cast<UCh>(*b)), 0xFFFFul);
      int d1 = u / 4096;
      u -= d1 * 4096;
      int d2 = u / 256;
      u -= d2 * 256;
      int d3 = u / 16;
      u -= d3 * 16;
      int d4 = u;
      result += Ch('\\');
      result += Ch('u');
      result += Ch(hexdigits[d1]);
      result += Ch(hexdigits[d2]);
      result += Ch(hexdigits[d3]);
      result += Ch(hexdigits[d4]);
    }
    ++b;
  }
  return result;
}

::std::string mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(
  const mitk::BaseProperty *prop)
{
  // REMARK: Implemented own serialization instead of using boost::ptree::json_write because
  // currently (<= boost 1.60) everything (even numbers) are converted into string representations
  // by the writer, so e.g. it becomes "t":"2" instaed of "t":2
  // If this problem is fixed with boost, we shoud switch back to json_writer (and remove the custom
  // implementation of CreateJSONEscapes (see above)).
  const auto *tsProp = dynamic_cast<const mitk::TemporoSpatialStringProperty *>(prop);

  if (!tsProp)
  {
    mitkThrow() << "Cannot serialize properties of types other than TemporoSpatialStringProperty.";
  }

  std::ostringstream stream;
  stream << "{\"values\":[";

  std::vector<mitk::TimeStepType> ts = tsProp->GetAvailableTimeSteps();
  bool first = true;
  for (auto t : ts)
  {
    std::vector<mitk::TemporoSpatialStringProperty::IndexValueType> zs = tsProp->GetAvailableSlices(t);
    for (auto z : zs)
    {
      std::string value = CreateJSONEscapes(tsProp->GetValue(t, z));

      if (first)
      {
        first = false;
      }
      else
      {
        stream << ", ";
      }

      stream << "{\"t\":" << t << ", \"z\":" << z << ", \"value\":\"" << value << "\"}";
    }
  }

  stream << "]}";

  return stream.str();
}

mitk::BaseProperty::Pointer mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(
  const std::string &value)
{
  if (value.empty())
    return nullptr;

  mitk::TemporoSpatialStringProperty::Pointer prop = mitk::TemporoSpatialStringProperty::New();

  boost::property_tree::ptree root;

  std::istringstream stream(value);

  boost::property_tree::read_json(stream, root);

  for (boost::property_tree::ptree::value_type &element : root.get_child("values"))
  {
    std::string value = element.second.get("value", "");
    mitk::TemporoSpatialStringProperty::IndexValueType z =
      element.second.get<mitk::TemporoSpatialStringProperty::IndexValueType>("z", 0);
    TimeStepType t = element.second.get<TimeStepType>("t", 0);

    prop->SetValue(t, z, value);
  }

  return prop.GetPointer();
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif