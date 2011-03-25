#define MITK_WRAP_PACKAGE "mitkRenderingPython"
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
