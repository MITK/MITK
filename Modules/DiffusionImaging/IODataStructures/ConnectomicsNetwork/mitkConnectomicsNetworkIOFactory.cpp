/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 6607 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
