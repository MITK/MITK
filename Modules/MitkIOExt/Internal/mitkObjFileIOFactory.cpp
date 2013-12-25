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

#include "mitkObjFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkObjFileReader.h"

#include "itkVersion.h"


namespace mitk
{
ObjFileIOFactory::ObjFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkObjFileReader",
                         "mitk Obj Surface IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<ObjFileReader> >::New());
}

ObjFileIOFactory::~ObjFileIOFactory()
{
}

const char* ObjFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ObjFileIOFactory::GetDescription() const
{
  return "ObjFile IO Factory, allows the loading of Obj files";
}

} // end namespace mitk
