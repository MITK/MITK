#define MITK_WRAP_PACKAGE "mitkAlgorithmsPython"
//#include "wrap_MITKAlgorithms.cxx"
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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifdef CABLE_CONFIGURATION

namespace _cable_
{
  const char* const package = MITK_WRAP_PACKAGE;
  const char* const groups[] =
  {
    //"ProcessObject",
    "BaseDataSource",
    "BaseProcess",
    "CoreObjectFactory",
    "CoreObjectFactoryBase",
    "DataNodeFactory",
    "DataNodeSource",
    "Geometry2DDataToSurfaceFilter",
    "HistogramGenerator",
    "ImageChannelSelector",
    "ImageSliceSelector",
    "ImageSource",
    "ImageTimeSelector",
    "ImageToImageFilter",
    "ImageToItk",
    "ITKImageImport",
    "PointSetSource",
    "RGBToRGBACastImageFilter",
    "SubImageSelector",
    "SurfaceSource",
    "SurfaceToSurfaceFilter",
    "VolumeCalculator"
  };
}
#endif
