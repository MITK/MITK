/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskListSerializer_h
#define mitkSegmentationTaskListSerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
  class SegmentationTaskListSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(SegmentationTaskListSerializer, BaseDataSerializer)
    itkFactorylessNewMacro(Self)

    std::string Serialize() override;

  protected:
    SegmentationTaskListSerializer();
    ~SegmentationTaskListSerializer() override;
  };
}

#endif
