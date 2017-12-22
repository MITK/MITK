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

#ifndef mitkNeverTranslucentTexture_h
#define mitkNeverTranslucentTexture_h

#include <MitkCoreExports.h>

#include <vtkOpenGLTexture.h>

#include <vtkObjectFactory.h>

/**
  \brief VTK Fix to speed up our image rendering.

  The way we render images while changing the contrast via level/window
  extremely slows down rendering.

  The cause of this slowdown is that VTK asks a texture
  (via a call to IsTranslucent) if it is translucent.
  When the texture refers to a lookup table this question
  is answered in a most expensive way: pushing every pixel
  through the lookup table to see whether it would render
  translucent.

  We can speed this up extremely by always answering NO.
  2D Image rendering in the context of MITK is still correct.

  This class is injected into the VTK system by registering
  it with vtkObjectFactory as a replacement for vtkTexture.

  We chose vtkOpenGLTexture as super class, because it seems
  that the other texture super class is deprecated:
  http://www.cmake.org/Wiki/VTK:How_I_mangled_Mesa

  \sa ImageVtkMapper2D
*/
/* NOT exported, this is a 2D image mapper helper */

class MITKCORE_EXPORT vtkNeverTranslucentTexture : public vtkOpenGLTexture
{
public:
  static vtkNeverTranslucentTexture *New();
  vtkTypeMacro(vtkNeverTranslucentTexture, vtkOpenGLTexture);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /**
    \brief The FIX (see class description).

    VTK Description: Is this Texture Translucent?

    Returns false (0) if the texture is either fully opaque or has
    only fully transparent pixels and fully opaque pixels and the
    Interpolate flag is turn off.
  */
  int IsTranslucent() override;

protected:
  vtkNeverTranslucentTexture();

private:
  vtkNeverTranslucentTexture(const vtkNeverTranslucentTexture &); // Not implemented.
  void operator=(const vtkNeverTranslucentTexture &);             // Not implemented.
};

/**
  \brief Factory for vtkNeverTranslucentTexture (see this class!).

  Registered in CoreActivator to replace all instances of vtkTexture
  with vtkNeverTranslucentTexture.

  Required to make rendering of images during level/window operations
  acceptably fast.
*/
class vtkNeverTranslucentTextureFactory : public vtkObjectFactory
{
public:
  vtkNeverTranslucentTextureFactory();

  static vtkNeverTranslucentTextureFactory *New();
  const char *GetVTKSourceVersion() override;
  const char *GetDescription() override;

protected:
  vtkNeverTranslucentTextureFactory(const vtkNeverTranslucentTextureFactory &);
  void operator=(const vtkNeverTranslucentTextureFactory &);
};

#endif
