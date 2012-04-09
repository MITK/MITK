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
