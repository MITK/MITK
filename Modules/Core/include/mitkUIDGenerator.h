/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_UID_GENERATOR_INDCLUDED_FASAWE
#define MITK_UID_GENERATOR_INDCLUDED_FASAWE

#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  /*!
    \brief Generated unique IDs

    Creates unique IDs.

    The current implementation uses the UUID specification (https://www.ietf.org/rfc/rfc4122.txt) and
    random generator.
    One may define a prefix for the UID string. But it is not needed do guarantee uniquness. It is
    just a human readable addition to see for which e.g. purpose the UID was generated.
  */
  class MITKCORE_EXPORT UIDGenerator
  {
  public:
    UIDGenerator(const char * prefix = "");

    /** @return Returns a unique ID as string. You will get another unique ID each time you call GetUID. */
    std::string GetUID();

  private:
    std::string m_Prefix;
  };
} // namespace mitk

#endif
