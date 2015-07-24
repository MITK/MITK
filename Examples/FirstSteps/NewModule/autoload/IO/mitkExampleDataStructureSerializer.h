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

#ifndef mitkExampleDataStructureSerializer_h_included
#define mitkExampleDataStructureSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::ExampleDataStructure for mitk::SceneIO
*/
class ExampleDataStructureSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( ExampleDataStructureSerializer, BaseDataSerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    virtual std::string Serialize() override;
  protected:
    ExampleDataStructureSerializer();
    virtual ~ExampleDataStructureSerializer();
};
} // namespace
#endif
