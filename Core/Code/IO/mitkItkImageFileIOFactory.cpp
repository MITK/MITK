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

#include "mitkItkImageFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkItkImageFileReader.h"

#include "itkVersion.h"


namespace mitk
{
ItkImageFileIOFactory::ItkImageFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkItkImageFileReader",
                         "itk Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<ItkImageFileReader> >::New());
}

ItkImageFileIOFactory::~ItkImageFileIOFactory()
{
}

const char* ItkImageFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ItkImageFileIOFactory::GetDescription() const
{
  return "ItkImageFile IO Factory, allows the loading of images supported by ITK";
}

} // end namespace mitk
