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
