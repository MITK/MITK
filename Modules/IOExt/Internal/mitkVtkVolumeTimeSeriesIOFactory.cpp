/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
                           true,
                           itk::CreateObjectFunction<IOAdapter<VtkVolumeTimeSeriesReader>>::New());
  }

  VtkVolumeTimeSeriesIOFactory::~VtkVolumeTimeSeriesIOFactory() {}
  const char *VtkVolumeTimeSeriesIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *VtkVolumeTimeSeriesIOFactory::GetDescription() const
  {
    return "VtkVolumeTimeSeries IO Factory, allows the loading of Vtk surfaces";
  }

} // end namespace mitk
