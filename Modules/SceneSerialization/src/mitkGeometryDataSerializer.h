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

#ifndef mitkGeometryDataSerializer_h_included
#define mitkGeometryDataSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::GeometryData for mitk::SceneIO.

  \warning depends on mitk::GeometryDataWriterService which is first implemented only for the Geometry3D class!
           See current status of that class if you want to use other geomety types.
*/
class GeometryDataSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( GeometryDataSerializer, BaseDataSerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    virtual std::string Serialize() override;
  protected:
    GeometryDataSerializer();
    virtual ~GeometryDataSerializer();
};

} // namespace
#endif
