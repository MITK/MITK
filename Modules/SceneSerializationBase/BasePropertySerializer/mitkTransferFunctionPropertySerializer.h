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

#include "mitkTransferFunctionProperty.h"
#include "mitkBasePropertySerializer.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{
  class SceneSerializationBase_EXPORT TransferFunctionPropertySerializer : public BasePropertySerializer
  {
    public:

      mitkClassMacro( TransferFunctionPropertySerializer, BasePropertySerializer );
      itkNewMacro(Self);

      virtual TiXmlElement* Serialize();
      virtual BaseProperty::Pointer Deserialize(TiXmlElement* element);

      static bool SerializeTransferFunction( const char * filename, TransferFunction::Pointer tf );
      static TransferFunction::Pointer DeserializeTransferFunction( const char *filePath );

    protected:

      TransferFunctionPropertySerializer();
      virtual ~TransferFunctionPropertySerializer();
  };
} // namespace

