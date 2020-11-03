/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
                           true,
                           itk::CreateObjectFunction<IOAdapter<StlVolumeTimeSeriesReader>>::New());
  }

  StlVolumeTimeSeriesIOFactory::~StlVolumeTimeSeriesIOFactory() {}
  const char *StlVolumeTimeSeriesIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *StlVolumeTimeSeriesIOFactory::GetDescription() const
  {
    return "StlVolumeTimeSeries IO Factory, allows the loading of Stl surfaces";
  }

} // end namespace mitk
