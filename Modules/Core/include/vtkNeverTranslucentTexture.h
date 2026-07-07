/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkNeverTranslucentTexture_h
#define vtkNeverTranslucentTexture_h

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
  /** \brief Create a new instance of vtkNeverTranslucentTexture. */
  static vtkNeverTranslucentTexture *New();
  vtkTypeMacro(vtkNeverTranslucentTexture, vtkOpenGLTexture);

  /** \brief Print the object state to the given output stream. */
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /**
   * \brief Always returns 0 (not translucent) to avoid the expensive per-pixel check.
   *
   * This is the core fix: VTK normally pushes every pixel through the lookup table
   * to determine translucency, which is extremely slow during level/window operations.
   * By always returning 0, this check is bypassed entirely.
   *
   * \return Always 0 (not translucent).
   *
   * \sa ImageVtkMapper2D
   */
  int IsTranslucent() override;

protected:
  /** \brief Constructor. */
  vtkNeverTranslucentTexture();

private:
  vtkNeverTranslucentTexture(const vtkNeverTranslucentTexture &); // Not implemented.
  void operator=(const vtkNeverTranslucentTexture &);             // Not implemented.
};

/**
 * \brief Factory that replaces vtkTexture with vtkNeverTranslucentTexture.
 *
 * Registered in CoreActivator to replace all instances of vtkTexture
 * with vtkNeverTranslucentTexture.
 *
 * Required to make rendering of images during level/window operations
 * acceptably fast.
 *
 * \sa vtkNeverTranslucentTexture
 */
class vtkNeverTranslucentTextureFactory : public vtkObjectFactory
{
public:
  /** \brief Constructor. Registers the override for vtkTexture. */
  vtkNeverTranslucentTextureFactory();

  /** \brief Create a new instance of this factory. */
  static vtkNeverTranslucentTextureFactory *New();

  /**
   * \brief Get the VTK source version string.
   *
   * \return The VTK source version.
   */
  const char *GetVTKSourceVersion() override;

  /**
   * \brief Get a description of this factory.
   *
   * \return A human-readable description string.
   */
  const char *GetDescription() override;

protected:
  vtkNeverTranslucentTextureFactory(const vtkNeverTranslucentTextureFactory &);
  void operator=(const vtkNeverTranslucentTextureFactory &);
};

#endif
