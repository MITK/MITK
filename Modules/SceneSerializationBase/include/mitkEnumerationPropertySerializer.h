/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnumerationPropertySerializer_h_included
#define mitkEnumerationPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkEnumerationProperty.h"

namespace mitk
{
  class MITKSCENESERIALIZATIONBASE_EXPORT EnumerationPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(EnumerationPropertySerializer, BasePropertySerializer);

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc) override;

  protected:
    EnumerationPropertySerializer();
    ~EnumerationPropertySerializer() override;
  };

} // namespace

#endif
