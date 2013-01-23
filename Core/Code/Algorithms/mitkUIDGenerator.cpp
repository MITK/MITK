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


#ifdef _MSC_VER
#include "process.h"
#else
#include <unistd.h>
#endif

namespace mitk {

UIDGenerator::UIDGenerator(const char* prefix, unsigned int lengthOfRandomPart)
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
  /*
  static int instanceID = 0;
  int processID = 0;
  #ifdef WIN32
    processID = _getpid();
  #else
    processID = getpid();
  #endif
  unsigned int hash = seedhash( time(NULL), clock() );
  unsigned int seed = (hash + processID) * 10 + instanceID;
  instanceID++;

  std::srand(seed);
  */
}

std::string UIDGenerator::GetUID()
{
  std::ostringstream s;
  s << m_Prefix;
  time_t tt = time(0);
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

    for (size_t i = rs.str().length(); i < m_LengthOfRandomPart; ++i)
    {
      s << "X";
    }

    s << rs.str();
  }

  return s.str();
}

unsigned int UIDGenerator::seedhash( time_t t, clock_t c )
{
  unsigned int h1 = 0;
  unsigned char *p = (unsigned char *) &t;
  for( size_t i = 0; i < sizeof(t); ++i )
  {
    h1 *= 255 + 2U;
    h1 += p[i];
  }
  unsigned int h2 = 0;
  p = (unsigned char *) &c;
  for( size_t j = 0; j < sizeof(c); ++j )
  {
    h2 *= 255 + 2U;
    h2 += p[j];
  }
  return h1 ^ h2;
}

}



