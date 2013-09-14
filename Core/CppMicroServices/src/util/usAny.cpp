/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "usAny.h"

US_BEGIN_NAMESPACE

std::ostream& operator<< (std::ostream& os, const Any& any)
{
  return os << any.ToString();
}

template<typename Iterator>
std::string container_to_string(Iterator i1, Iterator i2)
{
  std::stringstream ss;
  ss << "(";
  const Iterator begin = i1;
  for ( ; i1 != i2; ++i1)
  {
    if (i1 == begin) ss << *i1;
    else ss << "," << *i1;
  }
  ss << ")";
  return ss.str();
}

std::string any_value_to_string(const std::vector<std::string>& val)
{
  return container_to_string(val.begin(), val.end());
}

std::string any_value_to_string(const std::list<std::string>& val)
{
  return container_to_string(val.begin(), val.end());
}

std::string any_value_to_string(const std::vector<Any>& val)
{
  return container_to_string(val.begin(), val.end());
}

std::string any_value_to_string(const std::map<std::string, Any>& val)
{
  std::stringstream ss;
  ss << "[";
  typedef std::map<std::string, Any>::const_iterator Iterator;
  Iterator i1 = val.begin();
  const Iterator begin = i1;
  const Iterator end = val.end();
  for ( ; i1 != end; ++i1)
  {
    if (i1 == begin) ss << "\"" << i1->first << "\" => " << i1->second;
    else ss << ", " << "\"" << i1->first << "\" => " << i1->second;
  }
  ss << "]";
  return ss.str();
}

US_END_NAMESPACE
