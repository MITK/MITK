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

#include "mitkVtkSurfaceIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtkSurfaceReader.h"

#include "itkVersion.h"

  
namespace mitk
{
VtkSurfaceIOFactory::VtkSurfaceIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkVtkSurfaceReader",
                         "mitk Vtk Surface IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<VtkSurfaceReader> >::New());
}
  
VtkSurfaceIOFactory::~VtkSurfaceIOFactory()
{
}

const char* VtkSurfaceIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* VtkSurfaceIOFactory::GetDescription() const
{
  return "VtkSurface IO Factory, allows the loading of Vtk files";
}

} // end namespace mitk
