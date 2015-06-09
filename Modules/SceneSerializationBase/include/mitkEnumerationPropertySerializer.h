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

#ifndef mitkEnumerationPropertySerializer_h_included
#define mitkEnumerationPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkEnumerationProperty.h"

namespace mitk
{

class MITKSCENESERIALIZATIONBASE_EXPORT EnumerationPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( EnumerationPropertySerializer, BasePropertySerializer );

    virtual TiXmlElement* Serialize() override;

  protected:

    EnumerationPropertySerializer();
    virtual ~EnumerationPropertySerializer();
};

} // namespace

#endif

