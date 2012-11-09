#define MITK_WRAP_PACKAGE "mitkRenderingPython"
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
      //"BaseRenderer",
      "BaseVtkMapper2D",
      "BaseVtkMapper3D",
      "ColoredRectangleRendering",
      "Geometry2DDataMapper2D",
      "Geometry2DDataVtkMapper3D",
      "GLMapper2D",
      "GradientBackground",
      "ImageMapper2D",
      "LogoRendering",
      "ManufacturerLogo",
      "Mapper",
      "Mapper2D",
      "Mapper3D",
      "PointSetMapper2D",
      "PointSetVtkMapper3D",
      "PolyDataGLMapper2D",
      "PolyDataGLMapper2D",
      "ShaderRepository",
      "SurfaceMapper2D",
      "SurfaceVtkMapper3D",
      //"VolumeDataVtkMapper3D",
      "VtkPropRenderer",
      "VtkWidgetRendering",
      "vtkMitkRectangleProp",
      //"vtkMitkRenderProp",
      "vtkMitkThickSlicesFilter"
  };
}
#endif
