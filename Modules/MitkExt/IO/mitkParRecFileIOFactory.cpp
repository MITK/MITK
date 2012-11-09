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

#include "mitkParRecFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkParRecFileReader.h"

#include "itkVersion.h"


namespace mitk
{
ParRecFileIOFactory::ParRecFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkParRecFileReader",
                         "mitk ParRec Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<ParRecFileReader> >::New());
}

ParRecFileIOFactory::~ParRecFileIOFactory()
{
}

const char* ParRecFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ParRecFileIOFactory::GetDescription() const
{
  return "ParRecFile IO Factory, allows the loading of ParRec images";
}

} // end namespace mitk
