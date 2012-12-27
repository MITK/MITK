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

  You will get another quite unique ID each time you call GetUID.
*/
class MITK_CORE_EXPORT UIDGenerator
{
  public:
    UIDGenerator(const char* prefix = "UID_", unsigned int lengthOfRandomPart = 8);

    std::string GetUID();
  private:
    std::string m_Prefix;
    unsigned int m_LengthOfRandomPart;
};

}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
