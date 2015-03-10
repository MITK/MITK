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

// .NAME vtkMitkOpenGLVolumeTextureMapper3D - concrete implementation of 3D volume texture mapping

// .SECTION Description
// vtkMitkOpenGLVolumeTextureMapper3D renders a volume using 3D texture mapping.
// See vtkMitkVolumeTextureMapper3D for full description.

// .SECTION see also
// vtkMitkVolumeTextureMapper3D vtkVolumeMapper

#ifndef __vtkMitkOpenGLVolumeTextureMapper3D_h
#define __vtkMitkOpenGLVolumeTextureMapper3D_h

#include "vtkMitkVolumeTextureMapper3D.h"
#include "mitkBaseRenderer.h"
#include "MitkMapperExtExports.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h" // GLfloat type is used in some method signatures.
#endif


class vtkRenderWindow;
class vtkVolumeProperty;

#include "mitkCommon.h"

class MITKMAPPEREXT_EXPORT vtkMitkOpenGLVolumeTextureMapper3D : public vtkMitkVolumeTextureMapper3D
{
public:
  vtkTypeMacro(vtkMitkOpenGLVolumeTextureMapper3D,vtkMitkVolumeTextureMapper3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkMitkOpenGLVolumeTextureMapper3D *New();

  // Description:
  // Is hardware rendering supported? No if the input data is
  // more than one independent component, or if the hardware does
  // not support the required extensions
  // int IsRenderSupported(vtkVolumeProperty *);
  int IsRenderSupported(vtkRenderer *ren,vtkVolumeProperty *);

//BTX

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Render the volume
  virtual void Render(vtkRenderer *ren, vtkVolume *vol);

//ETX

  // Desciption:
  // Initialize when we go to render, or go to answer the
  // IsRenderSupported question. Don't call unless we have
  // a valid OpenGL context!
  vtkGetMacro( Initialized, int );

  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release.
  // deprecatedSince{2013_12} Use ReleaseGraphicsResources(mitk::BaseRenderer* renderer) instead
  DEPRECATED(void ReleaseGraphicsResources(vtkWindow *));

   // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter renderer could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(mitk::BaseRenderer * renderer);

protected:
  vtkMitkOpenGLVolumeTextureMapper3D();
  ~vtkMitkOpenGLVolumeTextureMapper3D();

  bool RenderPossible;

//BTX

  void GetLightInformation(vtkRenderer *ren,
                           vtkVolume *vol,
                           GLfloat lightDirection[2][4],
                           GLfloat lightDiffuseColor[2][4],
                           GLfloat lightSpecularColor[2][4],
                           GLfloat halfwayVector[2][4],
                           GLfloat *ambient );
//ETX

  int              Initialized;
  GLuint           Volume1Index;
  GLuint           Volume2Index;
  GLuint           Volume3Index;
  GLuint           ColorLookupIndex;
  GLuint           AlphaLookupIndex;

  GLuint prgOneComponentShade;
  GLuint prgRGBAShade;

  vtkRenderWindow *RenderWindow;

  bool SupportsCompressedTexture;

  //void Initialize();
  void Initialize(vtkRenderer *r);

  virtual void RenderFP(vtkRenderer *ren, vtkVolume *vol);

  void SetupOneIndependentTextures( vtkRenderer *ren, vtkVolume *vol );
  void RenderOneIndependentShadeFP( vtkRenderer *ren, vtkVolume *vol );

  void SetupRGBATextures( vtkRenderer *ren, vtkVolume *vol );
  void RenderRGBAShadeFP( vtkRenderer *ren, vtkVolume *vol );

  void DeleteTextureIndex( GLuint *index );
  void CreateTextureIndex( GLuint *index );

  void RenderPolygons( vtkRenderer *ren, vtkVolume *vol, int stages[4] );

  void SetupProgramLocalsForShadingFP( vtkRenderer *ren, vtkVolume *vol );

  void Setup3DTextureParameters( bool linear );

  void ComputeVolumeDimensions();

  bool UpdateVolumes( vtkVolume * );

  bool UpdateVolumesRGBA( vtkVolume * );

private:
  vtkMitkOpenGLVolumeTextureMapper3D(const vtkMitkOpenGLVolumeTextureMapper3D&);  // Not implemented.
  void operator=(const vtkMitkOpenGLVolumeTextureMapper3D&);  // Not implemented.
};


#endif
