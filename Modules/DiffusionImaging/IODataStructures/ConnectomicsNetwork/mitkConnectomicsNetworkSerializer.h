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

#ifndef mitkConnectomicsNetworkSerializer_h_included
#define mitkConnectomicsNetworkSerializer_h_included

#include "MitkDiffusionImagingExports.h"
#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::ConnectomicsNetwork for mitk::SceneIO
*/
class MitkDiffusionImaging_EXPORT ConnectomicsNetworkSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( ConnectomicsNetworkSerializer, BaseDataSerializer );
    itkNewMacro(Self);
    virtual std::string Serialize();
  protected:
    ConnectomicsNetworkSerializer();
    virtual ~ConnectomicsNetworkSerializer();
};
} // namespace
#endif
