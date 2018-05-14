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

#ifndef mitkTractographyForestSerializer_h_included
#define mitkTractographyForestSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::TractographyForest for mitk::SceneIO
*/
class TractographyForestSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( TractographyForestSerializer, BaseDataSerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    std::string Serialize() override;
  protected:
    TractographyForestSerializer();
    ~TractographyForestSerializer() override;
};
} // namespace
#endif
