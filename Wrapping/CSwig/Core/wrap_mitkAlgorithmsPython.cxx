#define MITK_WRAP_PACKAGE "mitkAlgorithmsPython"
//#include "wrap_MITKAlgorithms.cxx"
/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: wrap_ITKAlgorithms.cxx,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
