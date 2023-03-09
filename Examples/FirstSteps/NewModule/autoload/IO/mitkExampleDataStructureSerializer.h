/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExampleDataStructureSerializer_h
#define mitkExampleDataStructureSerializer_h

#include "mitkBaseDataSerializer.h"

namespace mitk
{
  /**
    \brief Serializes mitk::ExampleDataStructure for mitk::SceneIO
  */
  class ExampleDataStructureSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ExampleDataStructureSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    std::string Serialize() override;

  protected:
    ExampleDataStructureSerializer();
    ~ExampleDataStructureSerializer() override;
  };
} // namespace
#endif
