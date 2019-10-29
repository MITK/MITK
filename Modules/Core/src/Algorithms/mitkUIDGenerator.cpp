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

#include <mitkLogMacros.h>
#include <mitkUIDGenerator.h>

#include <iomanip>
#include <sstream>

mitk::UIDGenerator::UIDGenerator(const char *prefix, unsigned int lengthOfRandomPart)
  : m_Prefix(prefix),
    m_LengthOfRandomPart(lengthOfRandomPart),
    m_Distribution(std::uniform_int_distribution<unsigned long>(0, std::numeric_limits<unsigned long>::max()))
{
}

std::string mitk::UIDGenerator::GetUID()
{
  std::ostringstream s;
  s << m_Prefix;
  auto time = std::time(nullptr);
  auto tm = std::localtime(&time);

  s << std::put_time(tm, "%Y%m%d%H%M%S");

  std::ostringstream rs;

  static std::random_device rd;        // Will be used to obtain a seed for the random number engine
  static std::mt19937 generator(rd()); // Standard mersenne_twister_engine seeded with rd()

  while (rs.str().length() < m_LengthOfRandomPart)
  {
    rs << m_Distribution(generator);
  }

  auto randomString = rs.str();

  if (randomString.length() > m_LengthOfRandomPart)
  {
    randomString = randomString.substr(randomString.length() - m_LengthOfRandomPart);
  }

  s << randomString;

  return s.str();
}
