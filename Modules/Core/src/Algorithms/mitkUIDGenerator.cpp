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

#include <mitkUIDGenerator.h>
#include <mitkLogMacros.h>

#include <cstdlib>
#include <sstream>
#include <math.h>
#include <stdexcept>
#include <iostream>





mitk::UIDGenerator::UIDGenerator(const char* prefix, unsigned int lengthOfRandomPart)
:m_Prefix(prefix),
 m_LengthOfRandomPart(lengthOfRandomPart),
 m_RandomGenerator(itk::Statistics::MersenneTwisterRandomVariateGenerator::New())
{
  if (lengthOfRandomPart < 5)
  {
    MITK_ERROR << "To few digits requested (" <<lengthOfRandomPart<< " digits)";
    throw std::invalid_argument("To few digits requested");
  }

  m_RandomGenerator->Initialize();

}

std::string mitk::UIDGenerator::GetUID()
{
  std::ostringstream s;
  s << m_Prefix;
  time_t tt = time(nullptr);
  tm* t = gmtime(&tt);

  if (t)
  {


    s << t->tm_year + 1900;

    if (t->tm_mon < 9) s << "0"; // add a 0 for months 1 to 9
    s << t->tm_mon + 1;

    if (t->tm_mday < 10) s << "0"; // add a 0 for days 1 to 9
    s << t->tm_mday;

    if (t->tm_hour < 10) s << "0"; // add a 0 for hours 1 to 9
    s << t->tm_hour;

    if (t->tm_min < 10) s << "0"; // add a 0 for minutes 1 to 9
    s << t->tm_min;

    if (t->tm_sec < 10) s << "0"; // add a 0 for seconds 1 to 9
    s << t->tm_sec;

    std::ostringstream rs;
    rs << (long int)( pow(10.0, double(m_LengthOfRandomPart)) / double(RAND_MAX) * double(m_RandomGenerator->GetUniformVariate(0, RAND_MAX)) );

    for (size_t i = rs.str().length(); i < m_LengthOfRandomPart; ++i) //add zeros for non available digits
    {
      s << "0";
    }

    s << rs.str();
  }

  return s.str();
}



