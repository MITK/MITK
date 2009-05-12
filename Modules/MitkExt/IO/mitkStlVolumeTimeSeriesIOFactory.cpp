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

#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkStlVolumeTimeSeriesReader.h"

#include "itkVersion.h"

  
namespace mitk
{
StlVolumeTimeSeriesIOFactory::StlVolumeTimeSeriesIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkStlVolumeTimeSeriesReader",
                         "mitk Stl Surface IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<StlVolumeTimeSeriesReader> >::New());
}
  
StlVolumeTimeSeriesIOFactory::~StlVolumeTimeSeriesIOFactory()
{
}

const char* StlVolumeTimeSeriesIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* StlVolumeTimeSeriesIOFactory::GetDescription() const
{
  return "StlVolumeTimeSeries IO Factory, allows the loading of Stl surfaces";
}

} // end namespace mitk
