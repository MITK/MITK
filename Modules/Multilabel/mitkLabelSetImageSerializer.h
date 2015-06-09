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

#ifndef mitkLabelSetImageSerializer_h_included
#define mitkLabelSetImageSerializer_h_included

#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::LabelSetImage for mitk::SceneIO
*/
class LabelSetImageSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro( LabelSetImageSerializer, BaseDataSerializer );
    itkNewMacro(Self);
    virtual std::string Serialize() override;

  protected:
    LabelSetImageSerializer();
    virtual ~LabelSetImageSerializer();
  };
} // namespace

#endif
