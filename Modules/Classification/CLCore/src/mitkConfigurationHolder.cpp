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

#include <mitkConfigurationHolder.h>

#include <mitkExceptionMacro.h>
#include <sstream>

mitk::ConfigurationHolder::ConfigurationHolder() :
   m_ValueType(DT_UNINIZIALIZED)
{
  m_GroupValue.clear();
}

void mitk::ConfigurationHolder::SetBool(bool value)
{
  m_BoolValue = value;
  m_ValueType = DT_BOOL;
}

void mitk::ConfigurationHolder::SetUnsignedInt(unsigned int value)
{
  m_UIntValue = value;
  m_ValueType = DT_UINT;
}

void mitk::ConfigurationHolder::SetInt(int value)
{
  m_IntValue = value;
  m_ValueType = DT_INT;
}

void mitk::ConfigurationHolder::SetDouble(double value)
{
  m_DoubleValue = value;
  m_ValueType = DT_DOUBLE;
}

void mitk::ConfigurationHolder::SetString(std::string value)
{
  m_StringValue = value;
  m_ValueType = DT_STRING;
}

void mitk::ConfigurationHolder::ClearGroup()
{
  m_GroupValue.clear();
  m_ValueType = DT_GROUP;
}

void mitk::ConfigurationHolder::AddToGroup(std::string id, const ConfigurationHolder &value)
{
  m_GroupValue[id] = value;
  m_ValueType = DT_GROUP;
}

bool mitk::ConfigurationHolder::AsBool()
{
  switch (m_ValueType)
  {
  case DT_UNINIZIALIZED:
    mitkThrow() << "No value stored";
    break;
  case DT_BOOL:
    return m_BoolValue;
    break;
  case DT_UINT:
    return m_UIntValue;
    break;
  case DT_INT:
    return m_IntValue;
    break;
  case DT_DOUBLE:
    return (m_DoubleValue > 1 || m_DoubleValue < -1);
    break;
  case DT_STRING:
    return (m_StringValue != "0");
    break;
  case DT_GROUP:
    mitkThrow() << "Cannot convert group data to bool";
    break;
  default:
    mitkThrow() << "Unkown Data Type.";
    break;
  }
}

unsigned int mitk::ConfigurationHolder::AsUnsignedInt()
{
  unsigned int result;
  std::istringstream ss(m_StringValue);
  ss >> result;

  switch (m_ValueType)
  {
  case DT_UNINIZIALIZED:
    mitkThrow() << "No value stored";
    break;
  case DT_BOOL:
    return m_BoolValue;
    break;
  case DT_UINT:
    return m_UIntValue;
    break;
  case DT_INT:
    return m_IntValue;
    break;
  case DT_DOUBLE:
    return m_DoubleValue;
    break;
  case DT_STRING:
    return result;
    break;
  case DT_GROUP:
    mitkThrow() << "Cannot convert group data to bool";
    break;
  default:
    mitkThrow() << "Unkown Data Type.";
    break;
  }
}

int mitk::ConfigurationHolder::AsInt()
{
  int result;
  std::istringstream ss(m_StringValue);
  ss >> result;

  switch (m_ValueType)
  {
  case DT_UNINIZIALIZED:
    mitkThrow() << "No value stored";
    break;
  case DT_BOOL:
    return m_BoolValue;
    break;
  case DT_UINT:
    return m_UIntValue;
    break;
  case DT_INT:
    return m_IntValue;
    break;
  case DT_DOUBLE:
    return m_DoubleValue;
    break;
  case DT_STRING:
    return result;
    break;
  case DT_GROUP:
    mitkThrow() << "Cannot convert group data to bool";
    break;
  default:
    mitkThrow() << "Unkown Data Type.";
    break;
  }
}

double mitk::ConfigurationHolder::AsDouble()
{
  double result;
  std::istringstream ss(m_StringValue);
  ss >> result;

  switch (m_ValueType)
  {
  case DT_UNINIZIALIZED:
    mitkThrow() << "No value stored";
    break;
  case DT_BOOL:
    return m_BoolValue;
    break;
  case DT_UINT:
    return m_UIntValue;
    break;
  case DT_INT:
    return m_IntValue;
    break;
  case DT_DOUBLE:
    return m_DoubleValue > 1;
    break;
  case DT_STRING:
    return result;
    break;
  case DT_GROUP:
    mitkThrow() << "Cannot convert group data to bool";
    break;
  default:
    mitkThrow() << "Unkown Data Type.";
    break;
  }
}

std::string mitk::ConfigurationHolder::AsString()

{
  std::ostringstream strs;

  switch (m_ValueType)
  {
  case DT_UNINIZIALIZED:
    mitkThrow() << "No value stored";
    break;
  case DT_BOOL:
    strs << m_BoolValue;
    break;
  case DT_UINT:
    strs << m_UIntValue;
    break;
  case DT_INT:
    strs << m_IntValue;
    break;
  case DT_DOUBLE:
    strs << m_DoubleValue;
    break;
  case DT_STRING:
    return m_StringValue;
    break;
  case DT_GROUP:
    mitkThrow() << "Cannot convert group data to bool";
    break;
  default:
    mitkThrow() << "Unkown Data Type.";
    break;
  }
  return strs.str();
}

std::vector<std::string> mitk::ConfigurationHolder::AsStringVector()
{
  if (m_ValueType != DT_GROUP)
    mitkThrow() << "No Group Data, cannot convert to String Vector";

  std::vector<std::string> result;
  for (auto iter = m_GroupValue.begin(); iter != m_GroupValue.end(); ++iter)
  {
    result.push_back((*iter).second.AsString());
  }
  return result;
}

mitk::ConfigurationHolder& mitk::ConfigurationHolder::At(std::string id)
{
  return m_GroupValue[id];
}

bool mitk::ConfigurationHolder::AsBool(bool value)
{
  try {
    return this->AsBool();
  }
  catch (const mitk::Exception &)
  {
    return value;
  }
}

unsigned int mitk::ConfigurationHolder::AsUnsignedInt(unsigned int value)
{
  try {
    return this->AsUnsignedInt();
  }
  catch (const mitk::Exception &)
  {
    return value;
  }
}

int mitk::ConfigurationHolder::AsInt(int value)
{
  try {
    return this->AsInt();
  }
  catch (const mitk::Exception &)
  {
    return value;
  }
}

double mitk::ConfigurationHolder::AsDouble(double value)
{
  try {
    return this->AsDouble();
  }
  catch (const mitk::Exception &)
  {
    return value;
  }
}

std::string mitk::ConfigurationHolder::AsString(std::string value)
{
  try {
    return this->AsString();
  }
  catch (const mitk::Exception &)
  {
    return value;
  }
}
