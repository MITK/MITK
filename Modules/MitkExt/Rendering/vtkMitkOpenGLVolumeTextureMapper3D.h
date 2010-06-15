/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// .NAME vtkMitkOpenGLVolumeTextureMapper3D - concrete implementation of 3D volume texture mapping

// .SECTION Description
// vtkMitkOpenGLVolumeTextureMapper3D renders a volume using 3D texture mapping.
// See vtkMitkVolumeTextureMapper3D for full description.

// .SECTION see also
// vtkMitkVolumeTextureMapper3D vtkVolumeMapper

#ifndef __vtkMitkOpenGLVolumeTextureMapper3D_h
#define __vtkMitkOpenGLVolumeTextureMapper3D_h

#include "vtkMitkVolumeTextureMapper3D.h"
#include "MitkExtExports.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
# include "vtkOpenGL.h" // GLfloat type is used in some method signatures.
#endif
 

class vtkRenderWindow;
class vtkVolumeProperty;

#include "mitkCommon.h"

class MitkExt_EXPORT vtkMitkOpenGLVolumeTextureMapper3D : public vtkMitkVolumeTextureMapper3D
{
public:
  vtkTypeRevisionMacro(vtkMitkOpenGLVolumeTextureMapper3D,vtkMitkVolumeTextureMapper3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkMitkOpenGLVolumeTextureMapper3D *New();

  // Description:
  // Is hardware rendering supported? No if the input data is
  // more than one independent component, or if the hardware does
  // not support the required extensions
  int IsRenderSupported(vtkVolumeProperty *);
  
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
  void ReleaseGraphicsResources(vtkWindow *);
  
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

  vtkRenderWindow *RenderWindow;
  
  bool SupportsCompressedTexture;
  
  void Initialize();

  virtual void RenderFP(vtkRenderer *ren, vtkVolume *vol);

  void SetupOneIndependentTextures( vtkRenderer *ren, vtkVolume *vol );
  void RenderOneIndependentShadeFP( vtkRenderer *ren, vtkVolume *vol );

  void SetupFourDependentTextures( vtkRenderer *ren, vtkVolume *vol );
  void RenderFourDependentShadeFP( vtkRenderer *ren, vtkVolume *vol );

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



