/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageSerializer_h_included
#define mitkImageSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
  /**
    \brief Serializes mitk::Image for mitk::SceneIO
  */
  class ImageSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ImageSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      std::string Serialize() override;

  protected:
    ImageSerializer();
    ~ImageSerializer() override;
  };

} // namespace

#endif
