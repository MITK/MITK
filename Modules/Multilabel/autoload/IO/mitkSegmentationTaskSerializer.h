/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskSerializer_h
#define mitkSegmentationTaskSerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
  class SegmentationTaskSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(SegmentationTaskSerializer, BaseDataSerializer)
    itkFactorylessNewMacro(Self)

    std::string Serialize() override;

  protected:
    SegmentationTaskSerializer();
    ~SegmentationTaskSerializer() override;
  };
}

#endif
