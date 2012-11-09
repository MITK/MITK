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

#ifndef mitkQBallImageSerializer_h_included
#define mitkQBallImageSerializer_h_included

#include "MitkDiffusionImagingExports.h"
#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::Surface for mitk::SceneIO
*/
class MitkDiffusionImaging_EXPORT QBallImageSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( QBallImageSerializer, BaseDataSerializer );
    itkNewMacro(Self);
    virtual std::string Serialize();
  protected:
    QBallImageSerializer();
    virtual ~QBallImageSerializer();
};
} // namespace
#endif
