/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkUIDGenerator.h>

#include <sstream>

#include <mutex>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

boost::uuids::random_generator uuidGen;
std::mutex uuidGen_mutex;

mitk::UIDGenerator::UIDGenerator(const char *prefix)
  : m_Prefix(prefix)
{
}

std::string mitk::UIDGenerator::GetUID()
{
  std::ostringstream s;

  {
    std::lock_guard<std::mutex> guard(uuidGen_mutex);
    auto uuid = uuidGen();
    s << m_Prefix << uuid;
  }

  return s.str();
}
