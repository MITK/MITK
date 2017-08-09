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

#include "vtkNeverTranslucentTexture.h"

#include "vtkVersion.h"

// Replacement texture code -------------------------------------------------------------

vtkStandardNewMacro(vtkNeverTranslucentTexture);

vtkNeverTranslucentTexture::vtkNeverTranslucentTexture() : vtkOpenGLTexture()
{
}

int vtkNeverTranslucentTexture::IsTranslucent()
{
  return 0; // THE speedup
}

void vtkNeverTranslucentTexture::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Translucent: NEVER\n";
}

// Factory code -------------------------------------------------------------

VTK_CREATE_CREATE_FUNCTION(vtkNeverTranslucentTexture);

vtkNeverTranslucentTextureFactory::vtkNeverTranslucentTextureFactory()
{
  this->RegisterOverride("vtkTexture",
                         "vtkNeverTranslucentTextureFactory",
                         "less translucent texture",
                         1,
                         vtkObjectFactoryCreatevtkNeverTranslucentTexture);
}

vtkNeverTranslucentTextureFactory *vtkNeverTranslucentTextureFactory::New()
{
  return new vtkNeverTranslucentTextureFactory;
}

const char *vtkNeverTranslucentTextureFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}

const char *vtkNeverTranslucentTextureFactory::GetDescription()
{
  return "Factory for a quickly decided, never translucent, texture";
}
