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

#include "mitkVtkImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtkImageReader.h"

#include "itkVersion.h"

  
namespace mitk
{
VtkImageIOFactory::VtkImageIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkVtkImageReader",
                         "mitk Vtk Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<VtkImageReader> >::New());
}
  
VtkImageIOFactory::~VtkImageIOFactory()
{
}

const char* VtkImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* VtkImageIOFactory::GetDescription() const
{
  return "VtkImage IO Factory, allows the loading of pvtk files";
}

} // end namespace mitk
