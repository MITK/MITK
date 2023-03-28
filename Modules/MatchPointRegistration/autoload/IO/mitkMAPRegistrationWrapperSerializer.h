/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMAPRegistrationWrapperSerializer_h
#define mitkMAPRegistrationWrapperSerializer_h

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
