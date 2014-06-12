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

#include "mitkVtiFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtiFileReader.h"

#include "itkVersion.h"


namespace mitk
{
VtiFileIOFactory::VtiFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkVtiFileReader",
                         "mitk Vti Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<VtiFileReader> >::New());
}

VtiFileIOFactory::~VtiFileIOFactory()
{
}

const char* VtiFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* VtiFileIOFactory::GetDescription() const
{
  return "VtiFile IO Factory, allows the loading of vti files";
}

} // end namespace mitk
