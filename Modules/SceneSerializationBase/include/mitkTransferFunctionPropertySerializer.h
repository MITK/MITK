/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionPropertySerializer_h
#define mitkTransferFunctionPropertySerializer_h

#include "mitkBasePropertySerializer.h"
#include "mitkTransferFunctionProperty.h"

namespace mitk
{
  class MITKSCENESERIALIZATIONBASE_EXPORT TransferFunctionPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(TransferFunctionPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override;
    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override;

    static bool SerializeTransferFunction(const char *filename, TransferFunction::Pointer tf);
    static TransferFunction::Pointer DeserializeTransferFunction(const char *filePath);

  protected:
    TransferFunctionPropertySerializer();
    ~TransferFunctionPropertySerializer() override;
  };
}

#endif
