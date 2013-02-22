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

#include "mitkConnectomicsNetworkIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkConnectomicsNetworkReader.h"

#include "itkVersion.h"


namespace mitk
{

ConnectomicsNetworkIOFactory::ConnectomicsNetworkIOFactory()
{
  typedef ConnectomicsNetworkReader ConnectomicsNetworkReaderType;
  this->RegisterOverride("mitkIOAdapter", //beibehalten
                         "mitkConnectomicsNetworkReader", //umbenennen
                         "Connectomics Network IO", //angezeigter name
                         1,
                         itk::CreateObjectFunction<IOAdapter<ConnectomicsNetworkReaderType> >::New());
}

ConnectomicsNetworkIOFactory::~ConnectomicsNetworkIOFactory()
{
}

const char* ConnectomicsNetworkIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ConnectomicsNetworkIOFactory::GetDescription() const
{
  return "ConnectomicsNetworkIOFactory, allows the loading of Connectomics Networks";
}

} // end namespace mitk
