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
