/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
