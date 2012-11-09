#define MITK_WRAP_PACKAGE "mitkIOPython"
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
      "BaseDataIO",
      "DicomSeriesReader",
      "FileReader",
      "FileSeriesReader",
      "FileWriter",
      "FileWriterWithInformation",
      "ImageWriter",
      "ImageWriterFactory",
      //"IOAdapter",
      //"IpPicGet",
      "ItkImageFileIOFactory",
      "ItkImageFileReader",
      "Log",
      "LookupTableProperty",
      "Operation",
      "OperationActor",
      "PicFileIOFactory",
      "PicFileReader",
      "PicFileWriter",
      "PicHelper",
      "PicVolumeTimeSeriesIOFactory",
      "PicVolumeTimeSeriesReader",
      "PixelType",
      "PointSetIOFactory",
      "PointSetReader",
      "PointSetWriter",
      "PointSetWriterFactory",
      //"RawImageFileReader",
      "StandardFileLocations",
      "STLFileIOFactory",
      "STLFileReader",
      //"SurfaceVtkWriter",
      "SurfaceVtkWriterFactory",
      "VtiFileIOFactory",
      "VtiFileReader",
      "VtkImageIOFactory",
      "VtkImageReader",
      "VtkSurfaceIOFactory",
      "VtkSurfaceReader",
      "vtkPointSetXMLParser"
  };
}
#endif
