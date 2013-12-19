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

#include "mitkSTLFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkSTLFileReader.h"

#include "itkVersion.h"


namespace mitk
{
STLFileIOFactory::STLFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkSTLFileReader",
                         "mitk STL Surface IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<STLFileReader> >::New());
}

STLFileIOFactory::~STLFileIOFactory()
{
}

const char* STLFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* STLFileIOFactory::GetDescription() const
{
  return "STLFile IO Factory, allows the loading of STL files";
}

} // end namespace mitk
