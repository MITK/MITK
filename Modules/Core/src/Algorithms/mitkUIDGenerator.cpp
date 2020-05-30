/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkUIDGenerator.h>

#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


mitk::UIDGenerator::UIDGenerator(const char *prefix)
  : m_Prefix(prefix)
{
}

std::string mitk::UIDGenerator::GetUID()
{
  std::ostringstream s;
  auto gen = boost::uuids::random_generator();
  auto uuid = gen();

  s << m_Prefix << uuid;

  return s.str();
}
