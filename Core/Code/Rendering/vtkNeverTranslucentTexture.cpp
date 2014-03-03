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
#include "vtkObjectFactory.h"
#include "vtkVersion.h"

// Replacement texture code -------------------------------------------------------------

vtkStandardNewMacro(vtkNeverTranslucentTexture);

vtkNeverTranslucentTexture::vtkNeverTranslucentTexture()
:vtkOpenGLTexture()
{
}

vtkNeverTranslucentTexture::~vtkNeverTranslucentTexture()
{
}

int vtkNeverTranslucentTexture::IsTranslucent()
{
  return 0; // THE speedup
}
