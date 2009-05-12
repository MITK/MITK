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

#include "mitkVtkVolumeTimeSeriesIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkVtkVolumeTimeSeriesReader.h"

#include "itkVersion.h"

  
namespace mitk
{
VtkVolumeTimeSeriesIOFactory::VtkVolumeTimeSeriesIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkVtkVolumeTimeSeriesReader",
                         "mitk Vtk Surface IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<VtkVolumeTimeSeriesReader> >::New());
}
  
VtkVolumeTimeSeriesIOFactory::~VtkVolumeTimeSeriesIOFactory()
{
}

const char* VtkVolumeTimeSeriesIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* VtkVolumeTimeSeriesIOFactory::GetDescription() const
{
  return "VtkVolumeTimeSeries IO Factory, allows the loading of Vtk surfaces";
}

} // end namespace mitk
