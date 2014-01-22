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

#include "mitkPicVolumeTimeSeriesIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkPicVolumeTimeSeriesReader.h"

#include "itkVersion.h"


namespace mitk
{
PicVolumeTimeSeriesIOFactory::PicVolumeTimeSeriesIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkPicVolumeTimeSeriesReader",
                         "mitk Pic Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<PicVolumeTimeSeriesReader> >::New());
}

PicVolumeTimeSeriesIOFactory::~PicVolumeTimeSeriesIOFactory()
{
}

const char* PicVolumeTimeSeriesIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PicVolumeTimeSeriesIOFactory::GetDescription() const
{
  return "PicVolumeTimeSeries IO Factory, allows the loading of DKFZ Pic images";
}

} // end namespace mitk
