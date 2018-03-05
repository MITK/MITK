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


#ifndef __mitkMAPRegistrationWrapperSerializer_h
#define __mitkMAPRegistrationWrapperSerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
/**
  \brief Serializes mitk::MAPRegistrationWrapper for mitk::SceneIO
*/
class MAPRegistrationWrapperSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( MAPRegistrationWrapperSerializer, BaseDataSerializer );
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    std::string Serialize() override;

  protected:
    MAPRegistrationWrapperSerializer();
    ~MAPRegistrationWrapperSerializer() override;
};
} // namespace
#endif
