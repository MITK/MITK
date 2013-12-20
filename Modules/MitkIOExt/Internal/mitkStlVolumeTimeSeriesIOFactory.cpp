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
