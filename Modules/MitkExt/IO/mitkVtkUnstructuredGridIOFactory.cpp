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

#include "mitkVtkUnstructuredGridIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtkUnstructuredGridReader.h"

#include "itkVersion.h"


namespace mitk
{
VtkUnstructuredGridIOFactory::VtkUnstructuredGridIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkVtkUnstructuredGridReader",
                         "mitk Vtk UnstructuredGrid IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<VtkUnstructuredGridReader> >::New());
}

VtkUnstructuredGridIOFactory::~VtkUnstructuredGridIOFactory()
{
}

const char* VtkUnstructuredGridIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* VtkUnstructuredGridIOFactory::GetDescription() const
{
  return "VtkUnstructuredGrid IO Factory, allows the loading of Vtk files containing unstructured grid data";
}

} // end namespace mitk
