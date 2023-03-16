/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryDataSerializer_h
#define mitkGeometryDataSerializer_h

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
    mitkClassMacro(GeometryDataSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) std::string Serialize() override;

  protected:
    GeometryDataSerializer();
    ~GeometryDataSerializer() override;
  };

} // namespace
#endif
