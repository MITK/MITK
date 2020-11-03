/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetSerializer_h_included
#define mitkPointSetSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
  /**
    \brief Serializes mitk::Surface for mitk::SceneIO
  */
  class PointSetSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(PointSetSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) std::string Serialize() override;

  protected:
    PointSetSerializer();
    ~PointSetSerializer() override;
  };
} // namespace
#endif
