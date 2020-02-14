/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

      TiXmlElement *Serialize() override;
    BaseProperty::Pointer Deserialize(TiXmlElement *element) override;

    static bool SerializeTransferFunction(const char *filename, TransferFunction::Pointer tf);
    static TransferFunction::Pointer DeserializeTransferFunction(const char *filePath);

  protected:
    TransferFunctionPropertySerializer();
    ~TransferFunctionPropertySerializer() override;
  };
} // namespace
