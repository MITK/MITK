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

// .NAME vtkMitkShaderTexture - OpenGL texture map
// .SECTION Description
// vtkMitkShaderTexture is a concrete implementation of the abstract class
// vtkTexture. vtkMitkShaderTexture interfaces to the OpenGL rendering library.

#ifndef __vtkMesOpenGLTexture_h
#define __vtkMesOpenGLTexture_h

#include "vtkTexture.h"
//BTX
#include "vtkWeakPointer.h" // needed for vtkWeakPointer.
//ETX

class vtkWindow;
class vtkOpenGLRenderer;
class vtkRenderWindow;
class vtkPixelBufferObject;

class vtkMitkShaderTexture : public vtkTexture
{
public:
  static vtkMitkShaderTexture *New();
  vtkTypeMacro(vtkMitkShaderTexture,vtkTexture);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Load(vtkRenderer *ren);

  // Descsription:
  // Clean up after the rendering is complete.
  virtual void PostRender(vtkRenderer *ren);

  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release. Using the same texture object in multiple
  // render windows is NOT currently supported.
  void ReleaseGraphicsResources(vtkWindow *);

  void SetLevelWindow(float lower, float upper);

  float GetLevelWindowLower() const { return LevelWindowLower; }
  float GetLevelWindowUpper() const { return LevelWindowUpper; }
  bool Is16Bit() const { return Format16BIT; }
  // Description:
  // Get the openGL texture name to which this texture is bound.
  // This is available only if GL version >= 1.1
  vtkGetMacro(Index, long);
//BTX
protected:
  vtkMitkShaderTexture();
  ~vtkMitkShaderTexture();

  template <typename T>
  T *ResampleToPowerOfTwo(int &xsize, int &ysize, T *dptr, int numChannels);

  vtkTimeStamp   LoadTime;
  unsigned int Index; // actually GLuint
  vtkWeakPointer<vtkRenderWindow> RenderWindow;   // RenderWindow used for previous render

  bool Format16BIT;
  bool LevelWindowEnabled;
  float LevelWindowLower;
  float LevelWindowUpper;

private:
  vtkMitkShaderTexture(const vtkMitkShaderTexture&);  // Not implemented.
  void operator=(const vtkMitkShaderTexture&);  // Not implemented.

  // Description:
  // Handle loading in extension support
  virtual void Initialize(vtkRenderer * ren);

//ETX
};

#endif
