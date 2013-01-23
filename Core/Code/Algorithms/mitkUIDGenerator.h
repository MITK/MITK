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

#ifndef MITK_UID_GENERATOR_INDCLUDED_FASAWE
#define MITK_UID_GENERATOR_INDCLUDED_FASAWE

#include<string>
#include <ctime>
#include <MitkExports.h>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4251)
#endif

namespace mitk {

/*!
  \brief Generated unique IDs

  Creates (somehow most of the time) unique IDs from a given prefix,
  the current date/time and a random part.

  The prefix is given to the constructor, together with the desired
  length of the random part (minimum 5 digits).

  The current implementation uses the time in milliseconds in combination with an internal counter
  and the process ID to initialze a std::rand random generator. This should at least assure a unique
  ID one the same machine. However, this method is limited and you might get problems if generating
  IDs on different machines. Still, there is also a very small chance to get identical random numbers.
  A feature request for improved UID generation is handled in BUG 14333.
*/
class MITK_CORE_EXPORT UIDGenerator
{
  public:
    UIDGenerator(const char* prefix = "UID_", unsigned int lengthOfRandomPart = 8);

    /** @return Returns a unique ID as string. You will get another unique ID each time you call GetUID. */
    std::string GetUID();
  private:
    std::string m_Prefix;
    unsigned int m_LengthOfRandomPart;
    unsigned int seedhash( time_t t, clock_t c );
};

}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
