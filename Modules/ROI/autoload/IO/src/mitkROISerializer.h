/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROISerializer_h
#define mitkROISerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
  class ROISerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ROISerializer, BaseDataSerializer)
    itkFactorylessNewMacro(Self)

    std::string Serialize() override;

  protected:
    ROISerializer();
    ~ROISerializer() override;
  };
}

#endif
