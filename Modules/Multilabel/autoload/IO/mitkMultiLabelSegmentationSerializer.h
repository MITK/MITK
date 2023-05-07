/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationSerializer_h
#define mitkMultiLabelSegmentationSerializer_h

#include "mitkBaseDataSerializer.h"

namespace mitk
{
  /**
  \brief Serializes mitk::LabelSetImage for mitk::SceneIO
  */
  class LabelSetImageSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(LabelSetImageSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    std::string Serialize() override;

  protected:
    LabelSetImageSerializer();
    ~LabelSetImageSerializer() override;
  };
} // namespace

#endif
