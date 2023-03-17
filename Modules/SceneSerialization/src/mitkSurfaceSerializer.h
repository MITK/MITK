/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceSerializer_h
#define mitkSurfaceSerializer_h

#include "mitkBaseDataSerializer.h"

namespace mitk
{
  /**
    \brief Serializes mitk::Surface for mitk::SceneIO
  */
  class SurfaceSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(SurfaceSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      std::string Serialize() override;

  protected:
    SurfaceSerializer();
    ~SurfaceSerializer() override;
  };

} // namespace

#endif
