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

#ifndef mitkSurfaceSerializer_h_included
#define mitkSurfaceSerializer_h_included

#include "SceneSerializationExports.h"

#include "mitkBaseDataSerializer.h"

namespace mitk
{

/**
  \brief Serializes mitk::Surface for mitk::SceneIO
*/
class SceneSerialization_EXPORT SurfaceSerializer : public BaseDataSerializer
{
  public:

    mitkClassMacro( SurfaceSerializer, BaseDataSerializer );
    itkNewMacro(Self);

    virtual std::string Serialize();

  protected:

    SurfaceSerializer();
    virtual ~SurfaceSerializer();
};

} // namespace

#endif

