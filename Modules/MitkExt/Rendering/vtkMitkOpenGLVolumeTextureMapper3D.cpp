/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMitkOpenGLVolumeTextureMapper3D.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkWindows.h"
#include "vtkMitkOpenGLVolumeTextureMapper3D.h"
#include "mitkCommon.h"

#define GPU_INFO LOG_INFO("mapper.vr")
#define GPU_WARN LOG_WARN("mapper.vr")

#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkVolumeProperty.h"
#include "vtkTransform.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkgl.h"

char *vtkMitkVolumeTextureMapper3D_OneComponentShadeFP =
"!!ARBfp1.0\n"
	
//# This is the fragment program for one
//# component data with shading	

//# We need some temporary variables		
"TEMP index, normal, finalColor;\n"
"TEMP temp,temp1, temp2, temp3,temp4; \n"
"TEMP sampleColor;\n"
"TEMP ndotl, ndoth, ndotv; \n"
"TEMP lightInfo, lightResult;\n"
	 
//# We are going to use the first 
//# texture coordinate		
"ATTRIB tex0 = fragment.texcoord[0];\n"

//# This is the lighting information
"PARAM lightDirection = program.local[0];\n"
"PARAM halfwayVector  = program.local[1];\n"
"PARAM coefficient    = program.local[2];\n"
"PARAM lightDiffColor = program.local[3]; \n"
"PARAM lightSpecColor = program.local[4]; \n"
"PARAM viewVector     = program.local[5];\n"
"PARAM constants      = program.local[6];\n"
	
//# This is our output color
"OUTPUT out = result.color;\n"

//# Look up the gradient direction
//# in the third volume	
"TEX temp2, tex0, texture[0], 3D;\n"


// Gradient Compution

//# Look up the scalar value / gradient
//# magnitude in the first volume	
//"TEX temp1, tex0, texture[0], 3D;\n"

              /*

"ADD temp3,tex0,{-0.005,0,0};\n"
"TEX temp2,temp3, texture[0], 3D;\n"

//"ADD temp3,tex0,{ 0.005,0,0};\n"
//"TEX temp1,temp3, texture[0], 3D;\n"

"SUB normal.x,temp2.y,temp1.y;\n"


"ADD temp3,tex0,{0,-0.005,0};\n"
"TEX temp2,temp3, texture[0], 3D;\n"

//"ADD temp3,tex0,{0, 0.005,0};\n"
//"TEX temp1,temp3, texture[0], 3D;\n"

"SUB normal.y,temp2.y,temp1.y;\n"


"ADD temp3,tex0,{0,0,-0.005};\n"
"TEX temp2,temp3, texture[0], 3D;\n"

//"ADD temp3,tex0,{0,0, 0.005};\n"
//"TEX temp1,temp3, texture[0], 3D;\n"

"SUB normal.z,temp2.y,temp1.y;\n"


                */
                
//"MOV normal,{1,1,1};\n"

"MOV index.x,temp2.a;\n"

//# This normal is stored 0 to 1, change to -1 to 1
//# by multiplying by 2.0 then adding -1.0.	
"MAD normal, temp2, constants.x, constants.y;\n"

//# Swizzle this to use (a,r) as texture
//# coordinates	
//"SWZ index, temp1, a, r, 1, 1;\n"

//# Use this coordinate to look up a 
//# final color in the third texture
//# (this is a 2D texture)			

"DP3 temp4, normal, normal;\n"

"RSQ temp, temp4.x;\n"

"RCP temp4,temp.x;\n"

"MUL normal, normal, temp;\n"

"MOV index.y, temp4.x;\n"

"TEX sampleColor, index, texture[1], 2D;\n"

//"MUL sampleColor.w,sampleColor.w,temp4.x;\n"

//# Take the dot product of the light
//# direction and the normal
"DP3 ndotl, normal, lightDirection;\n"

//# Take the dot product of the halfway
//# vector and the normal		
"DP3 ndoth, normal, halfwayVector;\n"

"DP3 ndotv, normal, viewVector;\n"

//# flip if necessary for two sided lighting
"MUL temp3, ndotl, constants.y; \n"
"CMP ndotl, ndotv, ndotl, temp3;\n"
"MUL temp3, ndoth, constants.y; \n"
"CMP ndoth, ndotv, ndoth, temp3;\n"
	 
//# put the pieces together for a LIT operation		
"MOV lightInfo.x, ndotl.x; \n"
"MOV lightInfo.y, ndoth.x; \n"
"MOV lightInfo.w, coefficient.w; \n"

//# compute the lighting	
"LIT lightResult, lightInfo;\n"

//# COLOR FIX
"MUL lightResult, lightResult, 4.0;\n"

//# This is the ambient contribution	
"MUL finalColor, coefficient.x, sampleColor;\n"

//# This is the diffuse contribution	
"MUL temp3, lightDiffColor, sampleColor;\n"
"MUL temp3, temp3, lightResult.y;\n"
"ADD finalColor, finalColor, temp3;\n"

//# This is th specular contribution	
"MUL temp3, lightSpecColor, lightResult.z; \n"
	
//# Add specular into result so far, and replace
//# with the original alpha.	
"ADD out, finalColor, temp3;\n"
"MOV out.w, sampleColor.w;\n"
	 
"END\n";

char *vtkMitkVolumeTextureMapper3D_FourDependentShadeFP =
"!!ARBfp1.0\n"
// This is the fragment program for rgb data with shading	

// We need some temporary variables		
"TEMP index1, index2, normal, finalColor;\n"
"TEMP temp1, temp2, temp3;\n" 
"TEMP sampleColor, sampleOpacity;\n"
"TEMP ndotl, ndoth, ndotv;\n" 
"TEMP lightInfo, lightResult;\n"
	 
// We are going to use the first texture coordinate		
"ATTRIB tex0 = fragment.texcoord[0];\n"

// This is the lighting information
"PARAM lightDirection = program.local[0];\n"
"PARAM halfwayVector  = program.local[1];\n"
"PARAM coefficient    = program.local[2];\n"
"PARAM lightDiffColor = program.local[3];\n"
"PARAM lightSpecColor = program.local[4];\n"
"PARAM viewVector     = program.local[5];\n"
"PARAM constants      = program.local[6];\n"
	
// This is our output color
"OUTPUT out = result.color;\n"

// Look up color in the first volume
"TEX sampleColor, tex0, texture[0], 3D;\n"
	
// Look up the fourth scalar value / gradient magnitude in the second volume	
"TEX temp1, tex0, texture[1], 3D;\n"

// Look up the gradient direction in the third volume	
"TEX temp2, tex0, texture[2], 3D;\n"

// This normal is stored 0 to 1, change to -1 to 1 by multiplying by 2.0 then adding -1.0.	
"MAD normal, temp2, constants.x, constants.y;\n"
	
// Swizzle this to use (a,r) as texture coordinates for opacity
"SWZ index1, temp1, a, r, 1, 1;\n"

// Use this coordinate to look up a final opacity in the fourth texture
"TEX sampleOpacity, index1, texture[3], 2D;\n"

// Take the dot product of the light direction and the normal
"DP3 ndotl, normal, lightDirection;\n"

// Take the dot product of the halfway vector and the normal		
"DP3 ndoth, normal, halfwayVector;\n"

"DP3 ndotv, normal, viewVector;\n"
	 
// flip if necessary for two sided lighting
"MUL temp3, ndotl, constants.y;\n" 
"CMP ndotl, ndotv, ndotl, temp3;\n"
"MUL temp3, ndoth, constants.y;\n" 
"CMP ndoth, ndotv, ndoth, temp3;\n"
	 
// put the pieces together for a LIT operation		
"MOV lightInfo.x, ndotl.x;\n" 
"MOV lightInfo.y, ndoth.x;\n" 
"MOV lightInfo.w, coefficient.w;\n" 

// compute the lighting	
"LIT lightResult, lightInfo;\n"

// This is the ambient contribution	
"MUL finalColor, coefficient.x, sampleColor;\n"

// This is the diffuse contribution	
"MUL temp3, lightDiffColor, sampleColor;\n"
"MUL temp3, temp3, lightResult.y;\n"
"ADD finalColor, finalColor, temp3;\n"

// This is th specular contribution	
"MUL temp3, lightSpecColor, lightResult.z;\n" 
	
// Add specular into result so far, and replace with the original alpha.	
"ADD out, finalColor, temp3;\n"
"MOV out.w, sampleOpacity.w;\n"
	 
"END\n";



//#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkMitkOpenGLVolumeTextureMapper3D, "$Revision: 1.21 $");
vtkStandardNewMacro(vtkMitkOpenGLVolumeTextureMapper3D);
//#endif

vtkMitkOpenGLVolumeTextureMapper3D::vtkMitkOpenGLVolumeTextureMapper3D()
{
  //GPU_INFO << "vtkMitkOpenGLVolumeTextureMapper3D";

  this->Initialized                  =  0;
  this->Volume1Index                 =  0;
  this->Volume2Index                 =  0;
  this->Volume3Index                 =  0;
  this->ColorLookupIndex             =  0;
  this->AlphaLookupIndex             =  0;
  this->RenderWindow                 = NULL;
  this->SupportsCompressedTexture    = false;
  
  prgOneComponentShade = 0;
  
}

vtkMitkOpenGLVolumeTextureMapper3D::~vtkMitkOpenGLVolumeTextureMapper3D()
{
  //GPU_INFO << "~vtkMitkOpenGLVolumeTextureMapper3D";


  if(prgOneComponentShade)  
    vtkgl::DeleteProgramsARB( 1, &prgOneComponentShade );
}

// Release the graphics resources used by this texture.  
void vtkMitkOpenGLVolumeTextureMapper3D::ReleaseGraphicsResources(vtkWindow 
                                                                *renWin)
{
  //GPU_INFO << "ReleaseGraphicsResources";

  if (( this->Volume1Index || this->Volume2Index || 
        this->Volume3Index || this->ColorLookupIndex) && renWin)
    {
    static_cast<vtkRenderWindow *>(renWin)->MakeCurrent();
#ifdef GL_VERSION_1_1
    // free any textures
    this->DeleteTextureIndex( &this->Volume1Index );
    this->DeleteTextureIndex( &this->Volume2Index );
    this->DeleteTextureIndex( &this->Volume3Index );
    this->DeleteTextureIndex( &this->ColorLookupIndex );
    this->DeleteTextureIndex( &this->AlphaLookupIndex );    
#endif
    }
  this->Volume1Index     = 0;
  this->Volume2Index     = 0;
  this->Volume3Index     = 0;
  this->ColorLookupIndex = 0;
  this->RenderWindow     = NULL;
  this->SupportsCompressedTexture=false;
  this->SupportsNonPowerOfTwoTextures=false;

  this->Modified();
}

void vtkMitkOpenGLVolumeTextureMapper3D::Render(vtkRenderer *ren, vtkVolume *vol)
{  
  //GPU_INFO << "Render";

  ren->GetRenderWindow()->MakeCurrent();
  
  if ( !this->Initialized )
    {
    this->Initialize();
    }
  
  if ( !this->RenderPossible )
    {
    vtkErrorMacro( "required extensions not supported" );
    return;
    }

    
  vtkMatrix4x4       *matrix = vtkMatrix4x4::New();
  vtkPlaneCollection *clipPlanes;
  vtkPlane           *plane;
  int                numClipPlanes = 0;
  double             planeEquation[4];


  // build transformation 
  vol->GetMatrix(matrix);
  matrix->Transpose();

  glPushAttrib(GL_ENABLE_BIT   | 
               GL_COLOR_BUFFER_BIT   |
               GL_STENCIL_BUFFER_BIT |
               GL_DEPTH_BUFFER_BIT   | 
               GL_POLYGON_BIT        | 
               GL_TEXTURE_BIT);
  
  int i;
  
  // Use the OpenGL clip planes
  clipPlanes = this->ClippingPlanes;
  if ( clipPlanes )
    {
    numClipPlanes = clipPlanes->GetNumberOfItems();
    if (numClipPlanes > 6)
      {
      vtkErrorMacro(<< "OpenGL guarantees only 6 additional clipping planes");
      }

    for (i = 0; i < numClipPlanes; i++)
      {
      glEnable(static_cast<GLenum>(GL_CLIP_PLANE0+i));

      plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(i));

      planeEquation[0] = plane->GetNormal()[0]; 
      planeEquation[1] = plane->GetNormal()[1]; 
      planeEquation[2] = plane->GetNormal()[2];
      planeEquation[3] = -(planeEquation[0]*plane->GetOrigin()[0]+
                           planeEquation[1]*plane->GetOrigin()[1]+
                           planeEquation[2]*plane->GetOrigin()[2]);
      glClipPlane(static_cast<GLenum>(GL_CLIP_PLANE0+i),planeEquation);
      }
    }


  
  // insert model transformation 
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMultMatrixd(matrix->Element[0]);

  glColor4f( 1.0, 1.0, 1.0, 1.0 );

  // Turn lighting off - the polygon textures already have illumination
  glDisable( GL_LIGHTING );
  
  vtkGraphicErrorMacro(ren->GetRenderWindow(),"Before actual render method");
  
  this->RenderFP(ren,vol);
  
  // pop transformation matrix
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  matrix->Delete();
  glPopAttrib();

          /*
  glFlush();
  glFinish();
            */
  
  this->Timer->StopTimer();      

  this->TimeToDraw = static_cast<float>(this->Timer->GetElapsedTime());

  // If the timer is not accurate enough, set it to a small
  // time so that it is not zero
  if ( this->TimeToDraw == 0.0 )
    {
    this->TimeToDraw = 0.0001;
    }   
}

void vtkMitkOpenGLVolumeTextureMapper3D::RenderFP(vtkRenderer *ren,
                                              vtkVolume *vol)
{
  //GPU_INFO << "RenderFP";

/*
  glAlphaFunc (GL_GREATER, static_cast<GLclampf>(1.0/255.0));
  glEnable (GL_ALPHA_TEST);
  */
  
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  
  int components = this->GetInput()->GetNumberOfScalarComponents();   
  switch ( components )
    {
    case 1:
      this->RenderOneIndependentShadeFP(ren,vol);
      break;
      
    case 2:
      this->RenderTwoDependentShadeFP(ren,vol);
      break;
      
    case 3:
    case 4:
      this->RenderFourDependentShadeFP(ren,vol);
      break;
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE2);
  glDisable( GL_TEXTURE_2D );
  glDisable( vtkgl::TEXTURE_3D );
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE1);
  glDisable( GL_TEXTURE_2D );
  glDisable( vtkgl::TEXTURE_3D );
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE0);
  glDisable( GL_TEXTURE_2D );
  glDisable( vtkgl::TEXTURE_3D );  

  glDisable( GL_BLEND );
}

void vtkMitkOpenGLVolumeTextureMapper3D::DeleteTextureIndex( GLuint *index )
{
  //GPU_INFO << "DeleteTextureIndex";

  if (glIsTexture(*index))
    {
    GLuint tempIndex;
    tempIndex = *index;
    glDeleteTextures(1, &tempIndex);
    *index = 0;
    }
}

void vtkMitkOpenGLVolumeTextureMapper3D::CreateTextureIndex( GLuint *index )
{
  //GPU_INFO << "CreateTextureIndex";

  GLuint tempIndex=0;    
  glGenTextures(1, &tempIndex);
  *index = static_cast<long>(tempIndex);
}

void vtkMitkOpenGLVolumeTextureMapper3D::RenderPolygons( vtkRenderer *ren,
                                                       vtkVolume *vol,
                                                       int stages[4] )
{
  //GPU_INFO << "RenderPolygons";

  vtkRenderWindow *renWin = ren->GetRenderWindow();
  
  if ( renWin->CheckAbortStatus() )
    {
    return;
    }
  
  double bounds[27][6];
  float distance2[27];
  
  int   numIterations;
  int i, j, k;
  
  // No cropping case - render the whole thing
  if ( !this->Cropping )
    {
    // Use the input data bounds - we'll take care of the volume's
    // matrix during rendering
    this->GetInput()->GetBounds(bounds[0]);
    numIterations = 1;
    }
  // Simple cropping case - render the subvolume
  else if ( this->CroppingRegionFlags == 0x2000 )
    {
    this->GetCroppingRegionPlanes(bounds[0]);
    numIterations = 1;
    }
  // Complex cropping case - render each region in back-to-front order
  else
    {
    // Get the camera position
    double camPos[4];
    ren->GetActiveCamera()->GetPosition(camPos);
    
    double volBounds[6];
    this->GetInput()->GetBounds(volBounds);
    
    // Pass camera through inverse volume matrix
    // so that we are in the same coordinate system
    vtkMatrix4x4 *volMatrix = vtkMatrix4x4::New();
    vol->GetMatrix( volMatrix );
    camPos[3] = 1.0;
    volMatrix->Invert();
    volMatrix->MultiplyPoint( camPos, camPos );
    volMatrix->Delete();
    if ( camPos[3] )
      {
      camPos[0] /= camPos[3];
      camPos[1] /= camPos[3];
      camPos[2] /= camPos[3];
      }
    
    // These are the region limits for x (first four), y (next four) and
    // z (last four). The first region limit is the lower bound for
    // that axis, the next two are the region planes along that axis, and
    // the final one in the upper bound for that axis.
    float limit[12];    
    for ( i = 0; i < 3; i++ )
      {
      limit[i*4  ] = volBounds[i*2];
      limit[i*4+1] = this->CroppingRegionPlanes[i*2];
      limit[i*4+2] = this->CroppingRegionPlanes[i*2+1];
      limit[i*4+3] = volBounds[i*2+1];
      }
    
    // For each of the 27 possible regions, find out if it is enabled,
    // and if so, compute the bounds and the distance from the camera
    // to the center of the region.
    int numRegions = 0;
    int region;
    for ( region = 0; region < 27; region++ )
      {
      int regionFlag = 1<<region;
      
      if ( this->CroppingRegionFlags & regionFlag )
        {
        // what is the coordinate in the 3x3x3 grid
        int loc[3];
        loc[0] = region%3;
        loc[1] = (region/3)%3;
        loc[2] = (region/9)%3;

        // compute the bounds and center
        float center[3];
        for ( i = 0; i < 3; i++ )
          {
          bounds[numRegions][i*2  ] = limit[4*i+loc[i]];
          bounds[numRegions][i*2+1] = limit[4*i+loc[i]+1];
          center[i] = 
            (bounds[numRegions][i*2  ] + 
             bounds[numRegions][i*2+1])/2.0;
          }
        
        // compute the distance squared to the center
        distance2[numRegions] = 
          (camPos[0]-center[0])*(camPos[0]-center[0]) +
          (camPos[1]-center[1])*(camPos[1]-center[1]) +
          (camPos[2]-center[2])*(camPos[2]-center[2]);
        
        // we've added one region
        numRegions++;
        }
      }
    
    // Do a quick bubble sort on distance
    for ( i = 1; i < numRegions; i++ )
      {
      for ( j = i; j > 0 && distance2[j] > distance2[j-1]; j-- )
        {
        float tmpBounds[6];
        float tmpDistance2;
        
        for ( k = 0; k < 6; k++ )
          {
          tmpBounds[k] = bounds[j][k];
          }
        tmpDistance2 = distance2[j];

        for ( k = 0; k < 6; k++ )
          {
          bounds[j][k] = bounds[j-1][k];
          }
        distance2[j] = distance2[j-1];

        for ( k = 0; k < 6; k++ )
          {
          bounds[j-1][k] = tmpBounds[k];
          }
        distance2[j-1] = tmpDistance2;
        
        }
      }
    
    numIterations = numRegions;
    }

  // loop over all regions we need to render
  for ( int loop = 0; 
        loop < numIterations;
        loop++ )
    {
    // Compute the set of polygons for this region 
    // according to the bounds
    this->ComputePolygons( ren, vol, bounds[loop] );

    // Loop over the polygons
    for ( i = 0; i < this->NumberOfPolygons; i++ )
      {
      
      if ( renWin->CheckAbortStatus() )
        {
        return;
        }
      
      float *ptr = this->PolygonBuffer + 36*i;
      
      glBegin( GL_TRIANGLE_FAN );
      
      for ( j = 0; j < 6; j++ )
        {
        if ( ptr[0] < 0.0 )
          {
          break;
          }

        for ( k = 0; k < 4; k++ )
          {
          if ( stages[k] )
            {
            vtkgl::MultiTexCoord3fv( vtkgl::TEXTURE0 + k, ptr );
            }
          }
        glVertex3fv( ptr+3 );
        
        ptr += 6;
        }
      glEnd();         
      }
    }
}

// This method moves the scalars from the input volume into volume1 (and
// possibly volume2) which are the 3D texture maps used for rendering.
//
// In the case where our volume is a power of two, the copy is done 
// directly. If we need to resample, then trilinear interpolation is used.
//
// A shift/scale is applied to the input scalar value to produce an 8 bit
// value for the texture volume.
//
// When the input data is one component, the scalar value is placed in the
// second component of the two component volume1. The first component is
// filled in later with the gradient magnitude.
// 
// When the input data is two component non-independent, the first component
// of the input data is placed in the first component of volume1, and the
// second component of the input data is placed in the third component of
// volume1. Volume1 has three components - the second is filled in later with
// the gradient magnitude.
//
// When the input data is four component non-independent, the first three
// components of the input data are placed in volume1 (which has three
// components), and the fourth component is placed in the second component
// of volume2. The first component of volume2 is later filled in with the
// gradient magnitude.



static int _clamp(int value, int top)
{
  if(value<0) value=0;
  if(value>top) value=top;
  return value;
}


template <class T> 
class ScalarGradientCompute
{
  T *dataPtr;
  unsigned char *tmpPtr;
  unsigned char *tmpPtr2;
  int sizeX;
  int sizeY;
  int sizeZ;
  int sizeXY;
  int sizeXm1;
  int sizeYm1;
  int sizeZm1;
  int fullX;
  int fullY;
  int fullZ;
  int fullXY;
  int currentChunkStart;
  int currentChunkEnd;

  int offZ;
  
  float offset;
  float scale;
  
  public:

  ScalarGradientCompute( T *_dataPtr,unsigned char *_tmpPtr,unsigned char *_tmpPtr2,int _sizeX,int _sizeY,int _sizeZ,int _fullX,int _fullY,int _fullZ,float _offset,float _scale)
  {
    dataPtr=_dataPtr;
    tmpPtr=_tmpPtr;
    tmpPtr2=_tmpPtr2;
    sizeX=_sizeX;
    sizeY=_sizeY;
    sizeZ=_sizeZ;
    fullX=_fullX;
    fullY=_fullY;
    fullZ=_fullZ;
    offset=_offset;
    scale=_scale;

    sizeXY=sizeX*sizeY;
    sizeXm1=sizeX-1;
    sizeYm1=sizeY-1;
    sizeZm1=sizeZ-1;
    
    fullXY=fullX*fullY;
  }

  inline float sample(int x,int y,int z)
  {
    return float(dataPtr[ x + y * sizeX + z * sizeXY ]);
  }
  
  inline void fill(int x,int y,int z)
  {
    int doff = x + y * fullX + (z-offZ) * fullXY;
    
    tmpPtr[doff*4+0]= 0;
    tmpPtr[doff*4+1]= 0;
    tmpPtr[doff*4+2]= 0;
    tmpPtr[doff*4+3]= 0;
          /*
    tmpPtr2[doff*3+0]= 0;
    tmpPtr2[doff*3+1]= 0;
    tmpPtr2[doff*3+2]= 0;       
    */
  }
  
  inline int clamp(int x)
  {
    if(x<0) x=0; else if(x>255) x=255;
    return x;
  }
  
  inline void write(int x,int y,int z,float grayValue,float gx,float gy,float gz)
  {
  
 /*       
    gx /= aspect[0];
    gy /= aspect[1];
    gz /= aspect[2];
 */
    // Compute the gradient magnitude

    int iGrayValue = static_cast<int>( (grayValue + offset) * scale + 0.5f );

    gx *= scale;
    gy *= scale;
    gz *= scale;

    float t = sqrtf( gx*gx + gy*gy + gz*gz );

    if ( t > 0.01f )
    {
      if( t < 2.0f )
      {
        float fac = 2.0f/t;
        gx *= fac;
        gy *= fac;
        gz *= fac;
      } 
      else if( t > 255.0f)
      {
        float fac = 255.0f/t;
        gx *= fac;
        gy *= fac;
        gz *= fac;
      } 
    }
    else
    {
      gx=gy=gz=0.0f;
    }

    int nx = static_cast<int>(0.5f*gx+127.5f);
    int ny = static_cast<int>(0.5f*gy+127.5f);
    int nz = static_cast<int>(0.5f*gz+127.5f);
    
    int doff = x + y * fullX + (z-offZ) * fullXY;
    
    //tmpPtr[doff*2+0]= 0;
       
    tmpPtr[doff*4+0]= clamp(nx);
    tmpPtr[doff*4+1]= clamp(ny);
    tmpPtr[doff*4+2]= clamp(nz);       
    tmpPtr[doff*4+3]= clamp(iGrayValue);

/*
    if( z == fullZ/2 )
    if( y == fullY/2 )
      LOG_INFO << x << " " << y << " " << z << " : " << iGrayValue << " : " << iGradient;
  */  
  }
  
  
  inline void compute(int x,int y,int z)
  {
    float grayValue = sample(x,y,z);
    float gx,gy,gz;

    gx = sample(x+1,y,z) - sample(x-1,y,z);
    gy = sample(x,y+1,z) - sample(x,y-1,z);
    gz = sample(x,y,z+1) - sample(x,y,z-1);
    
    write( x, y, z, grayValue, gx, gy, gz );
 
  }

  inline void computeClamp(int x,int y,int z)
  {
    float grayValue = sample(x,y,z);
    float gx,gy,gz;

    if(x==0)            gx = 2.0f * ( sample(x+1,y,z) - grayValue       );
    else if(x==sizeXm1) gx = 2.0f * ( grayValue       - sample(x-1,y,z) );
    else                gx =          sample(x+1,y,z) - sample(x-1,y,z);
    
    if(y==0)            gy = 2.0f * ( sample(x,y+1,z) - grayValue       );
    else if(y==sizeYm1) gy = 2.0f * ( grayValue       - sample(x,y-1,z) );
    else                gy =          sample(x,y+1,z) - sample(x,y-1,z);
    
    if(z==0)            gz = 2.0f * ( sample(x,y,z+1) - grayValue       );
    else if(z==sizeZm1) gz = 2.0f * ( grayValue       - sample(x,y,z-1) );
    else                gz =          sample(x,y,z+1) - sample(x,y,z-1);
    
    write( x, y, z, grayValue, gx, gy, gz );
  }

  inline void compute1D(int y,int z)
  {
    int x;
    
    x=0;
    computeClamp(x,y,z);
    x++;
    
    while(x<sizeX-1)
    {
      compute(x,y,z);     
      x++;
    }
    
    if(x<sizeX)
    {
      computeClamp(x,y,z);
      x++;
    }

    while(x<fullX)
    {
      fill(x,y,z);
      x++;
    }
  }      

  inline void fill1D(int y,int z)
  {
    int x;
    
    x=0;
    while(x<fullX)
    {
      fill(x,y,z);
      x++;
    }
  }      
        

  inline void computeClamp1D(int y,int z)
  {
    int x;
    
    x=0;
    
    while(x<sizeX)
    {
      computeClamp(x,y,z);     
      x++;
    }

    while(x<fullX)
    {
      fill(x,y,z);
      x++;
    }
  }      
        
  inline void computeClamp2D(int z)
  {
    int y;

    y=0;

    while(y<sizeY)
    {
      computeClamp1D(y,z);     
      y++;
    }
    
    while(y<fullY)
    {
      fill1D(y,z);
      y++;
    }      
  }
  
  inline void compute2D(int z)
  {
    int y;

    y=0;
    computeClamp1D(y,z);     
    y++;
    
    while(y<sizeY-1)
    {
      compute1D(y,z);     
      y++;
    }
    
    if(y<sizeY)
    {
      computeClamp1D(y,z);
      y++;
    }

    while(y<fullY)
    {
      fill1D(y,z);
      y++;
    }      
  }
  
  inline void fill2D(int z)
  {
    int y;

    y=0;
    while(y<fullY)
    {
      fill1D(y,z);
      y++;
    }      
  }
  
  inline void fillSlices(int currentChunkStart,int currentChunkEnd)
  {
    int z;

    offZ=currentChunkStart;

    for(z=currentChunkStart;z<=currentChunkEnd;z++)
    {
      if(z==0 || z==sizeZ-1)
        computeClamp2D(z);
      else if(z>=sizeZ)
        fill2D(z);
      else
        compute2D(z);
    }
  }    
};



template <class T>
void vtkVolumeTextureMapper3DComputeScalars( T *dataPtr,
                                               vtkMitkVolumeTextureMapper3D *me,
                                               float offset, float scale,
                                               GLuint volume1,
                                               GLuint volume2)
{
  //GPU_INFO << "vtkVolumeTextureMapper3DComputeScalars";

  T              *inPtr;
  // unsigned char  *outPtr, *outPtr2;
  // int             i, j, k;
  // int             idx;

  int   inputDimensions[3];
  double inputSpacing[3];
  vtkImageData *input = me->GetInput();

  input->GetDimensions( inputDimensions );
  input->GetSpacing( inputSpacing );

  int   outputDimensions[3];
  float outputSpacing[3];
  me->GetVolumeDimensions( outputDimensions );
  me->GetVolumeSpacing( outputSpacing );

  int components = input->GetNumberOfScalarComponents();

  // double wx, wy, wz;
  // double fx, fy, fz;
  // int x, y, z;

  double sampleRate[3];
  sampleRate[0] = outputSpacing[0] / static_cast<double>(inputSpacing[0]);
  sampleRate[1] = outputSpacing[1] / static_cast<double>(inputSpacing[1]);
  sampleRate[2] = outputSpacing[2] / static_cast<double>(inputSpacing[2]);

  int fullX = outputDimensions[0];
  int fullY = outputDimensions[1];
  int fullZ = outputDimensions[2];

  int sizeX = inputDimensions[0];
  int sizeY = inputDimensions[1];
  int sizeZ = inputDimensions[2];

  int chunkSize = 32;
  
  if(fullZ < chunkSize) chunkSize=fullZ;

  int numChunks = ( fullZ + (chunkSize-1) ) / chunkSize;

  inPtr = dataPtr;

  unsigned char *tmpPtr  = new unsigned char[fullX*fullY*chunkSize*4];
  unsigned char *tmpPtr2 = 0;//new unsigned char[fullX*fullY*chunkSize*3];

  // For each Chunk
  {
    ScalarGradientCompute<T> sgc(dataPtr,tmpPtr,tmpPtr2,sizeX,sizeY,sizeZ,fullX,fullY,fullZ,offset,scale);

    int currentChunk = 0;

    while(currentChunk < numChunks)
    {
      LOG_INFO << "processing chunk " << currentChunk;
      
      int currentChunkStart = currentChunk * chunkSize;
      int currentChunkEnd   = currentChunkStart + chunkSize - 1 ;
      
      if( currentChunkEnd > (fullZ-1) )
        currentChunkEnd = (fullZ-1);
      
      int currentChunkSize = currentChunkEnd - currentChunkStart + 1;
    
      sgc.fillSlices( currentChunkStart , currentChunkEnd );

      glBindTexture(vtkgl::TEXTURE_3D, volume1);
      vtkgl::TexSubImage3D(vtkgl::TEXTURE_3D,0,0,0,currentChunkStart,fullX,fullY,currentChunkSize,GL_RGBA,GL_UNSIGNED_BYTE,tmpPtr);
                                                  /*
      glBindTexture(vtkgl::TEXTURE_3D, volume2);
      vtkgl::TexSubImage3D(vtkgl::TEXTURE_3D,0,0,0,currentChunkStart,fullX,fullY,currentChunkSize,GL_RGB,GL_UNSIGNED_BYTE,tmpPtr2);
                                                    */
      currentChunk ++;
    }
  }
  
  delete tmpPtr;
 // delete tmpPtr2;
  
  /*
  if ( components == 1 )
  {
    LOG_INFO << "converting, compressing and uploading volume data to gpu (1 component)";

    outPtr  = tmpPtr ;
    outPtr2 = tmpPtr2;

    // traverse all slices
    for(z=0;z<sizeZ;z++)
    {
      

      for(y=0;y<sizeY;y++)
      {
        for(x=0;x<sizeX;x++)
        {
          idx = static_cast<int>((SAMPLE(x,y,z) + offset) * scale);
          *(outPtr++) = 0;
          *(outPtr++) = idx;
        }
        for(   ;x<fullX;x++)
        {
          *(outPtr++) = 0;
          *(outPtr++) = 0;
        }
      }
      for(   ;y<fullY;y++)
      {
        for(x=0;x<fullX;x++)
        {
          *(outPtr++) = 0;
          *(outPtr++) = 0;
        }
      }
      
      
    }
    for(   ;z<fullZ;z++)
    {
      for(y=0;y<fullY;y++)
      {
        for(x=0;x<fullX;x++)
        {
          *(outPtr++) = 0;
          *(outPtr++) = 0;
        }
      }
    }

    LOG_INFO << "converting finished";
  }
  else
    LOG_ERROR << components << " components are currently unsupported";
    
  delete tmpPtr;
  delete tmpPtr2;*/
  
}

  /*
//-----------------------------------------------------------------------------
template <class T>
void vtkVolumeTextureMapper3DComputeGradients( T *dataPtr,
                                                 vtkMitkVolumeTextureMapper3D *me,
                                                 double scalarRange[2],
                                                 unsigned char *volume1,
                                                 unsigned char *volume2,
                                                 unsigned char *volume3)
{
  //GPU_INFO << "vtkVolumeTextureMapper3DComputeGradients";


  int                 x, y, z;
  int                 offset, outputOffset;
  int                 x_start, x_limit;
  int                 y_start, y_limit;
  int                 z_start, z_limit;
  T                   *dptr;
  double               n[3], t;
  double               gvalue;
  double               zeroNormalThreshold;
  int                 xlow, xhigh;
  double              aspect[3];
  unsigned char       *outPtr1, *outPtr2;
  unsigned char       *normals, *gradmags;
  int                 gradmagIncrement;
  int                 gradmagOffset;
  double              floc[3];
  int                 loc[3];

  float outputSpacing[3];
  me->GetVolumeSpacing( outputSpacing );

  double spacing[3];
  vtkImageData *input = me->GetInput();

  input->GetSpacing( spacing );

  double sampleRate[3];
  sampleRate[0] = outputSpacing[0] / static_cast<double>(spacing[0]);
  sampleRate[1] = outputSpacing[1] / static_cast<double>(spacing[1]);
  sampleRate[2] = outputSpacing[2] / static_cast<double>(spacing[2]);
 
  int components = input->GetNumberOfScalarComponents();
 
  int dim[3];
  input->GetDimensions(dim);

  int outputDim[3];
  me->GetVolumeDimensions( outputDim );

  double avgSpacing = (spacing[0] + spacing[1] + spacing[2]) / 3.0;

  // adjust the aspect
  aspect[0] = spacing[0] * 2.0 / avgSpacing;
  aspect[1] = spacing[1] * 2.0 / avgSpacing;
  aspect[2] = spacing[2] * 2.0 / avgSpacing;
  
  double scale = 255.0 / (0.25*(scalarRange[1] - scalarRange[0]));

  // Get the length at or below which normals are considered to
  // be "zero"
  zeroNormalThreshold =.001 * (scalarRange[1] - scalarRange[0]);

  int thread_id = 0;
  int thread_count = 1;

  x_start = 0;
  x_limit = outputDim[0];
  y_start = 0;
  y_limit = outputDim[1];
  z_start = static_cast<int>(( thread_id / static_cast<float>(thread_count) ) *
                  outputDim[2] );
  z_limit = static_cast<int>(( (thread_id + 1) / static_cast<float>(thread_count) ) *
                  outputDim[2] );

  // Do final error checking on limits - make sure they are all within bounds
  // of the scalar input

  x_start = (x_start<0)?(0):(x_start);
  y_start = (y_start<0)?(0):(y_start);
  z_start = (z_start<0)?(0):(z_start);

  x_limit = (x_limit>dim[0])?(outputDim[0]):(x_limit);
  y_limit = (y_limit>dim[1])?(outputDim[1]):(y_limit);
  z_limit = (z_limit>dim[2])?(outputDim[2]):(z_limit);

  if ( components == 1 || components == 2 )
    {
    normals = volume2;
    gradmags = volume1;
    gradmagIncrement = components+1;
    gradmagOffset = components-1;
    }
  else 
    {
    normals = volume3;
    gradmags = volume2;
    gradmagIncrement = 2;
    gradmagOffset = 0;
    }

  double wx, wy, wz;

  LOG_INFO << "computing gradient";

  // Loop through all the data and compute the encoded normal and
  // gradient magnitude for each scalar location
  for ( z = z_start; z < z_limit; z++ )
    {
    floc[2] = z*sampleRate[2];
    floc[2] = (floc[2]>=(dim[2]-1))?(dim[2]-1.001):(floc[2]);
    loc[2]  = vtkMath::Floor(floc[2]);
    wz = floc[2] - loc[2];

    for ( y = y_start; y < y_limit; y++ )
      {
      floc[1] = y*sampleRate[1];
      floc[1] = (floc[1]>=(dim[1]-1))?(dim[1]-1.001):(floc[1]);
      loc[1]  = vtkMath::Floor(floc[1]);
      wy = floc[1] - loc[1];

      xlow = x_start;
      xhigh = x_limit;
      outputOffset = z * outputDim[0] * outputDim[1] + y * outputDim[0] + xlow;

      // Set some pointers
      outPtr1 = gradmags + gradmagIncrement*outputOffset;
      outPtr2 = normals + 3*outputOffset;

      for ( x = xlow; x < xhigh; x++ )
        {
        floc[0] = x*sampleRate[0];
        floc[0] = (floc[0]>=(dim[0]-1))?(dim[0]-1.001):(floc[0]);
        loc[0]  = vtkMath::Floor(floc[0]);
        wx = floc[0] - loc[0];

        offset = loc[2] * dim[0] * dim[1] + loc[1] * dim[0] + loc[0];

        dptr = dataPtr + components*offset + components - 1;

        // Use a central difference method if possible,
        // otherwise use a forward or backward difference if
        // we are on the edge
        int sampleOffset[6];
        sampleOffset[0] = (loc[0]<1)        ?(0):(-components);
        sampleOffset[1] = (loc[0]>=dim[0]-2)?(0):( components);
        sampleOffset[2] = (loc[1]<1)        ?(0):(-components*dim[0]);
        sampleOffset[3] = (loc[1]>=dim[1]-2)?(0):( components*dim[0]);
        sampleOffset[4] = (loc[2]<1)        ?(0):(-components*dim[0]*dim[1]);
        sampleOffset[5] = (loc[2]>=dim[2]-2)?(0):( components*dim[0]*dim[1]);

        float sample[6];
        for ( int i = 0; i < 6; i++ )
          {
          float A, B, C, D, E, F, G, H;
          T *samplePtr = dptr + sampleOffset[i];

          A = static_cast<float>(*(samplePtr));
          B = static_cast<float>(*(samplePtr+components));
          C = static_cast<float>(*(samplePtr+components*dim[0]));
          D = static_cast<float>(*(samplePtr+components*dim[0]+components));
          E = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]));
          F = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]+components));
          G = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]+components*dim[0]));
          H = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]+components*dim[0]+components));

          sample[i] = 
            (1.0-wx)*(1.0-wy)*(1.0-wz)*A +
            (    wx)*(1.0-wy)*(1.0-wz)*B +
            (1.0-wx)*(    wy)*(1.0-wz)*C +
            (    wx)*(    wy)*(1.0-wz)*D +
            (1.0-wx)*(1.0-wy)*(    wz)*E +
            (    wx)*(1.0-wy)*(    wz)*F +
            (1.0-wx)*(    wy)*(    wz)*G +
            (    wx)*(    wy)*(    wz)*H;
          }

        n[0] = ((sampleOffset[0]==0 || sampleOffset[1]==0)?(2.0):(1.0))*(sample[0] -sample[1]);
        n[1] = ((sampleOffset[2]==0 || sampleOffset[3]==0)?(2.0):(1.0))*(sample[2] -sample[3]);
        n[2] = ((sampleOffset[4]==0 || sampleOffset[5]==0)?(2.0):(1.0))*(sample[4] -sample[5]);

        // Take care of the aspect ratio of the data
        // Scaling in the vtkVolume is isotropic, so this is the
        // only place we have to worry about non-isotropic scaling.
        n[0] /= aspect[0];
        n[1] /= aspect[1];
        n[2] /= aspect[2];

        // Compute the gradient magnitude
        t = sqrt( n[0]*n[0] + n[1]*n[1] + n[2]*n[2] );

        // Encode this into an 4 bit value 
        gvalue = t * scale; 

        gvalue = (gvalue<0.0)?(0.0):(gvalue);
        gvalue = (gvalue>255.0)?(255.0):(gvalue);


        *(outPtr1+gradmagOffset) = static_cast<unsigned char>(gvalue + 0.5);

        // Normalize the gradient direction
        if ( t > zeroNormalThreshold )
          {
          n[0] /= t;
          n[1] /= t;
          n[2] /= t;
          }
        else
          {
          n[0] = n[1] = n[2] = 0.0;
          }

        int nx = static_cast<int>((n[0] / 2.0 + 0.5)*255.0 + 0.5);
        int ny = static_cast<int>((n[1] / 2.0 + 0.5)*255.0 + 0.5);
        int nz = static_cast<int>((n[2] / 2.0 + 0.5)*255.0 + 0.5);

        nx = (nx<0)?(0):(nx);
        ny = (ny<0)?(0):(ny);
        nz = (nz<0)?(0):(nz);

        nx = (nx>255)?(255):(nx);
        ny = (ny>255)?(255):(ny);
        nz = (nz>255)?(255):(nz);

        *(outPtr2  ) = nx;
        *(outPtr2+1) = ny;
        *(outPtr2+2) = nz;

        outPtr1 += gradmagIncrement;
        outPtr2 += 3;
        }
      }
//    if ( z%8 == 7 )
//      {
//      float args[1];
//      args[0] = 
//        static_cast<float>(z - z_start) / 
//        static_cast<float>(z_limit - z_start - 1);
//      me->InvokeEvent( vtkEvent::VolumeMapperComputeGradientsProgressEvent, args );
//      }
    }
//  me->InvokeEvent( vtkEvent::VolumeMapperComputeGradientsEndEvent, NULL );

  LOG_INFO << "computing gradient finished";
}
    */
bool vtkMitkOpenGLVolumeTextureMapper3D::NeedUpdateVolumes()
{
  // Get the image data
  vtkImageData *input = this->GetInput();
  input->Update();
 
  // Has the volume changed in some way?
  if ( this->SavedTextureInput != input || this->SavedTextureMTime.GetMTime() < input->GetMTime() )
    return true;
  
  return false;
}


//-----------------------------------------------------------------------------
int vtkMitkOpenGLVolumeTextureMapper3D::UpdateVolumes(vtkVolume *vtkNotUsed(vol))
{
  vtkImageData *input = this->GetInput();

  // How big does the Volume need to be?
  int dim[3];
  input->GetDimensions(dim);

  int components = input->GetNumberOfScalarComponents();
  
  int powerOfTwoDim[3];

  if(this->SupportsNonPowerOfTwoTextures) 
  {
    for ( int i = 0; i < 3; i++ )
    {
      powerOfTwoDim[i]=(dim[i]+1)&~1;
    }                             

    LOG_INFO << "using non-power-two even textures (" << (1.0-double(dim[0]*dim[1]*dim[2])/double(powerOfTwoDim[0]*powerOfTwoDim[1]*powerOfTwoDim[2])) * 100.0 << "% memory wasted)";
  }
  else
  {
    for ( int i = 0; i < 3; i++ )
    {
      powerOfTwoDim[i] = 4;
      while ( powerOfTwoDim[i] < dim[i] )
        powerOfTwoDim[i] *= 2;
    }

    LOG_INFO << "using power-two textures (" << (1.0-double(dim[0]*dim[1]*dim[2])/double(powerOfTwoDim[0]*powerOfTwoDim[1]*powerOfTwoDim[2])) * 100.0 << "% memory wasted)";
  }
 
 /*
  while ( ! this->IsTextureSizeSupported( powerOfTwoDim,components ) )
    {
      //GPU_INFO << "Downsampling";

    if ( powerOfTwoDim[0] >= powerOfTwoDim[1] &&
         powerOfTwoDim[0] >= powerOfTwoDim[2] )
      {
      powerOfTwoDim[0] /= 2;
      }
    else if ( powerOfTwoDim[1] >= powerOfTwoDim[0] &&
              powerOfTwoDim[1] >= powerOfTwoDim[2] )
      {
      powerOfTwoDim[1] /= 2;
      }
    else
      {
      powerOfTwoDim[2] /= 2;
      }
    }
   */
    //GPU_INFO << "ALLOCATING TEXTURE VOLUMEs " << powerOfTwoDim[0] << "/" << powerOfTwoDim[1] << "/" << powerOfTwoDim[2];

  int neededSize = powerOfTwoDim[0] * powerOfTwoDim[1] * 1;
 
  // What is the spacing?
  double spacing[3];
  input->GetSpacing(spacing);

  //GPU_INFO << input->GetMTime();

/* 
  // Is it the right size? If not, allocate it.
  if ( this->VolumeSize != neededSize || this->VolumeComponents != components ) {
    delete [] this->Volume1;
    delete [] this->Volume2;
    delete [] this->Volume3;
    switch (components)
    {
      case 1:
        //GPU_INFO << "allocate 1";
        this->Volume1 = new unsigned char [2*neededSize];
        this->Volume2 = new unsigned char [3*neededSize];
        this->Volume3 = NULL;
        break;
      case 2:
        //GPU_INFO << "allocate 2";
        this->Volume1 = new unsigned char [3*neededSize];
        this->Volume2 = new unsigned char [3*neededSize];
        this->Volume3 = NULL;
        break;
      case 3:
      case 4:
        //GPU_INFO << "allocate 3/4";
        this->Volume1 = new unsigned char [3*neededSize];
        this->Volume2 = new unsigned char [2*neededSize];
        this->Volume3 = new unsigned char [3*neededSize];
        break;
    }

    this->VolumeSize       = neededSize;
    this->VolumeComponents = components;
  }
 */
 
  // Find the scalar range
  double scalarRange[2];
  input->GetPointData()->GetScalars()->GetRange(scalarRange, components-1);
 
  // Is the difference between max and min less than 4096? If so, and if
  // the data is not of float or double type, use a simple offset mapping.
  // If the difference between max and min is 4096 or greater, or the data
  // is of type float or double, we must use an offset / scaling mapping.
  // In this case, the array size will be 4096 - we need to figure out the 
  // offset and scale factor.
  float offset;
  float scale;
 
  int arraySizeNeeded;
 
  int scalarType = input->GetScalarType();


  if ( scalarType == VTK_FLOAT ||
       scalarType == VTK_DOUBLE ||
       scalarRange[1] - scalarRange[0] > 255 )
  {
    arraySizeNeeded = 256;
    offset          = -scalarRange[0];
    scale           = 255.0 / (scalarRange[1] - scalarRange[0]);
  }
  else
  {
    arraySizeNeeded = static_cast<int>(scalarRange[1] - scalarRange[0] + 1);
    offset          = -scalarRange[0]; 
    scale           = 1.0;
  }
 
  this->ColorTableSize   = arraySizeNeeded;
  this->ColorTableOffset = offset;
  this->ColorTableScale  = scale;

  // Save the volume size
  this->VolumeDimensions[0] = powerOfTwoDim[0];
  this->VolumeDimensions[1] = powerOfTwoDim[1];
  this->VolumeDimensions[2] = powerOfTwoDim[2];
 
  // Compute the new spacing
  this->VolumeSpacing[0] = ( dim[0] -1.01)*spacing[0] / static_cast<double>(this->VolumeDimensions[0]-1);
  this->VolumeSpacing[1] = ( dim[1] -1.01)*spacing[1] / static_cast<double>(this->VolumeDimensions[1]-1);
  this->VolumeSpacing[2] = ((dim[2])-1.01)*spacing[2] / static_cast<double>(this->VolumeDimensions[2]-1);





    {
    // Deleting old textures
    this->DeleteTextureIndex(&this->Volume1Index);
    this->DeleteTextureIndex(&this->Volume2Index);
    this->DeleteTextureIndex(&this->Volume3Index);

    this->CreateTextureIndex(&this->Volume1Index);
    //this->CreateTextureIndex(&this->Volume2Index);

    int dim[3]; this->GetVolumeDimensions(dim);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE0 );

    LOG_INFO << "allocating volume on gpu";
    
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGBA,dim[0],dim[1],dim[2],0,GL_RGBA,GL_UNSIGNED_BYTE,0);
    this->Setup3DTextureParameters( true );
  /*                    
    LOG_INFO << "allocating gradient direction volume";
    
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,dim[0],dim[1],dim[2],0,GL_RGB,GL_UNSIGNED_BYTE,0);
    this->Setup3DTextureParameters( true );
*/
    }
    

  // Transfer the input volume to the RGBA volume
  void *dataPtr = input->GetScalarPointer();

  switch ( scalarType )
    {
    vtkTemplateMacro(
      vtkVolumeTextureMapper3DComputeScalars(
        static_cast<VTK_TT *>(dataPtr), this,
        offset, scale,
        this->Volume1Index,
        this->Volume2Index));
    }

/*
  switch ( scalarType )
    {
    vtkTemplateMacro( 
      vtkVolumeTextureMapper3DComputeGradients(
        static_cast<VTK_TT *>(dataPtr), this,
        scalarRange,
        this->Volume1,
        this->Volume2,
        this->Volume3));
    }
*/

  this->SavedTextureInput = input;
  this->SavedTextureMTime.Modified();

  return 1;
}


void vtkMitkOpenGLVolumeTextureMapper3D::Setup3DTextureParameters( bool linear )
{
  //GPU_INFO << "Setup3DTextureParameters";
  
  if( linear )
  {
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }
  else
  {
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  }

  glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( vtkgl::TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
}

void vtkMitkOpenGLVolumeTextureMapper3D::SetupOneIndependentTextures( vtkRenderer *vtkNotUsed(ren),
                    vtkVolume *vol )
{ 
 // LOG_INFO << "SetupOneIndependentTextures";

  // Update the volume containing the 2 byte scalar / gradient magnitude
  if( this->NeedUpdateVolumes() || !this->Volume1Index /*|| !this->Volume2Index*/ )
  {    
    if(this->UseCompressedTexture && SupportsCompressedTexture)
    {
      //LOG_INFO << "using compressed textures";
      #define myGL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
      #define myGL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT           0x8C72
      #define myGL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
      this->InternalAlpha=vtkgl::COMPRESSED_ALPHA;
      this->InternalLA=myGL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT; //vtkgl::COMPRESSED_LUMINANCE_ALPHA;
      this->InternalRGB=myGL_COMPRESSED_RGB_S3TC_DXT1_EXT; //vtkgl::COMPRESSED_RGB;
      this->InternalRGBA=myGL_COMPRESSED_RGBA_S3TC_DXT5_EXT; //vtkgl::COMPRESSED_RGBA;
    }
    else
    {
      //LOG_INFO << "using uncompressed textures";
      this->InternalAlpha=GL_ALPHA8;
      this->InternalLA=GL_LUMINANCE8_ALPHA8;
      this->InternalRGB=GL_RGB8;
      this->InternalRGBA=GL_RGBA8;
    }
    


    this->UpdateVolumes( vol );
    

  }

  // Update the dependent 2D color table mapping scalar value and
  // gradient magnitude to RGBA
  if ( this->UpdateColorLookup( vol ) || !this->ColorLookupIndex )
  {
    this->DeleteTextureIndex( &this->ColorLookupIndex );
    this->DeleteTextureIndex( &this->AlphaLookupIndex );
    
    this->CreateTextureIndex( &this->ColorLookupIndex );

    vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
    glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    //LOG_INFO << "uploading transferfunction";

    glTexImage2D( GL_TEXTURE_2D, 0,this->InternalRGBA, 256, 256, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, this->ColorLookup );    
  }
  
}


void vtkMitkOpenGLVolumeTextureMapper3D::SetupTwoDependentTextures(
  vtkRenderer *vtkNotUsed(ren),
  vtkVolume *vol )
{
  LOG_INFO << "SetupTwoDependentTextures";
                              /*
  vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
  glDisable( GL_TEXTURE_2D );
  glEnable( vtkgl::TEXTURE_3D );

  vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
  glDisable( GL_TEXTURE_2D );
  glEnable( vtkgl::TEXTURE_3D );
                                */
                                /*
  // Update the volume containing the 3 byte scalars / gradient magnitude
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || !this->Volume2Index )
    {    
    int dim[3];
    this->GetVolumeDimensions(dim);
    this->DeleteTextureIndex(&this->Volume3Index);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,dim[0],dim[1],
                      dim[2],0,GL_RGB,GL_UNSIGNED_BYTE,this->Volume1);
    this->Setup3DTextureParameters( true );
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume2Index);
    this->CreateTextureIndex(&this->Volume2Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,dim[0],dim[1],
                      dim[2],0,GL_RGB,GL_UNSIGNED_BYTE,this->Volume2);
    this->Setup3DTextureParameters( true );

    }
                                  */
/*  
  vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);   

  vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
  this->Setup3DTextureParameters( true );
    
  vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
  glEnable( GL_TEXTURE_2D );
  glDisable( vtkgl::TEXTURE_3D );

  vtkgl::ActiveTexture( vtkgl::TEXTURE3 );
  glEnable( GL_TEXTURE_2D );
  glDisable( vtkgl::TEXTURE_3D );
  */
  // Update the dependent 2D color table mapping scalar value and
  // gradient magnitude to RGBA
  /*
  if ( this->UpdateColorLookup( vol ) || 
       !this->ColorLookupIndex || !this->AlphaLookupIndex )
    {    
    vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
    glBindTexture(GL_TEXTURE_2D,0);
    this->DeleteTextureIndex(&this->ColorLookupIndex);
    this->CreateTextureIndex(&this->ColorLookupIndex);
    glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    //LOG_INFO << "uploading transferfunction / scalar";

    glTexImage2D(GL_TEXTURE_2D,0,this->InternalRGB, 256, 256, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, this->ColorLookup );    
      
    vtkgl::ActiveTexture( vtkgl::TEXTURE3 );
    glBindTexture(GL_TEXTURE_2D,0);
    this->DeleteTextureIndex(&this->AlphaLookupIndex);
    this->CreateTextureIndex(&this->AlphaLookupIndex);
    glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    //LOG_INFO << "uploading transferfunction / gradient";

    glTexImage2D(GL_TEXTURE_2D, 0,this->InternalAlpha, 256, 256, 0,
                 GL_ALPHA, GL_UNSIGNED_BYTE, this->AlphaLookup );      
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
  glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

  vtkgl::ActiveTexture( vtkgl::TEXTURE3 );
  glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   
  
  */
}


void vtkMitkOpenGLVolumeTextureMapper3D::SetupFourDependentTextures(
  vtkRenderer *vtkNotUsed(ren),
  vtkVolume *vol )
{
  LOG_INFO << "SetupFourDependentTextures";
    /*
  vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
  glDisable( GL_TEXTURE_2D );
  glEnable( vtkgl::TEXTURE_3D );

  vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
  glDisable( GL_TEXTURE_2D );
  glEnable( vtkgl::TEXTURE_3D );

  vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
  glDisable( GL_TEXTURE_2D );
  glEnable( vtkgl::TEXTURE_3D );

  // Update the volume containing the 3 byte scalars / gradient magnitude
  if ( this->UpdateVolumes( vol ) || !this->Volume1Index || 
       !this->Volume2Index || !this->Volume3Index )
    {    
    int dim[3];
    this->GetVolumeDimensions(dim);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume1Index);
    this->CreateTextureIndex(&this->Volume1Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,dim[0],dim[1],
                      dim[2],0,GL_RGB,GL_UNSIGNED_BYTE,this->Volume1);

    vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume2Index);
    this->CreateTextureIndex(&this->Volume2Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalLA,dim[0],dim[1],
                      dim[2],0,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,
                      this->Volume2);

    vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
    glBindTexture(vtkgl::TEXTURE_3D,0);
    this->DeleteTextureIndex(&this->Volume3Index);
    this->CreateTextureIndex(&this->Volume3Index);
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume3Index);
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,dim[0],dim[1],
                      dim[2],0,GL_RGB,GL_UNSIGNED_BYTE,this->Volume3);
    }
  
  vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);
  this->Setup3DTextureParameters( true );

  vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
  glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
  this->Setup3DTextureParameters( true );

  vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
  glBindTexture(vtkgl::TEXTURE_3D_EXT, this->Volume3Index);   
  this->Setup3DTextureParameters( true );

  vtkgl::ActiveTexture( vtkgl::TEXTURE3 );
  glEnable( GL_TEXTURE_2D );
  glDisable( vtkgl::TEXTURE_3D );

  // Update the dependent 2D table mapping scalar value and
  // gradient magnitude to opacity
  if ( this->UpdateColorLookup( vol ) || !this->AlphaLookupIndex )
    {    
    this->DeleteTextureIndex(&this->ColorLookupIndex);
    
    vtkgl::ActiveTexture( vtkgl::TEXTURE3 );
    glBindTexture(GL_TEXTURE_2D,0);
    this->DeleteTextureIndex(&this->AlphaLookupIndex);
    this->CreateTextureIndex(&this->AlphaLookupIndex);
    glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    //LOG_INFO << "uploading transferfunction";

    glTexImage2D(GL_TEXTURE_2D,0,this->InternalAlpha, 256, 256, 0,
                 GL_ALPHA, GL_UNSIGNED_BYTE, this->AlphaLookup );      
    }

  vtkgl::ActiveTexture( vtkgl::TEXTURE3 );
  glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   
  
  */
}

void vtkMitkOpenGLVolumeTextureMapper3D::RenderOneIndependentShadeFP(
  vtkRenderer *ren,
  vtkVolume *vol )
{
  //GPU_INFO << "RenderOneIndependentShadeFP";

  this->SetupOneIndependentTextures( ren, vol );


  glEnable( vtkgl::FRAGMENT_PROGRAM_ARB );

  vtkgl::BindProgramARB( vtkgl::FRAGMENT_PROGRAM_ARB, prgOneComponentShade );
           
  this->SetupProgramLocalsForShadingFP( ren, vol );

  // Bind Textures
  {
    vtkgl::ActiveTexture( vtkgl::TEXTURE0 );
    glDisable( GL_TEXTURE_2D );
    glEnable( vtkgl::TEXTURE_3D );
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume1Index);   

    vtkgl::ActiveTexture( vtkgl::TEXTURE1 );
    glEnable( GL_TEXTURE_2D );
    glDisable( vtkgl::TEXTURE_3D );
    glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);

    vtkgl::ActiveTexture( vtkgl::TEXTURE2 );
    glDisable( GL_TEXTURE_2D );
    glEnable( vtkgl::TEXTURE_3D );
    glBindTexture(vtkgl::TEXTURE_3D, this->Volume2Index);   
  }
  
  // Start the timer now
  this->Timer->StartTimer();

  
  int stages[4] = {1,1,1,0};
  this->RenderPolygons( ren, vol, stages );  

  glDisable( vtkgl::FRAGMENT_PROGRAM_ARB );
  
}

void vtkMitkOpenGLVolumeTextureMapper3D::RenderTwoDependentShadeFP(
  vtkRenderer *ren,
  vtkVolume *vol )
{
  //GPU_INFO << "RenderTwoDependentShadeFP";

  glEnable( vtkgl::FRAGMENT_PROGRAM_ARB );

  GLuint fragmentProgram;
  vtkgl::GenProgramsARB( 1, &fragmentProgram );

  vtkgl::BindProgramARB( vtkgl::FRAGMENT_PROGRAM_ARB, fragmentProgram );

  vtkgl::ProgramStringARB( vtkgl::FRAGMENT_PROGRAM_ARB,
          vtkgl::PROGRAM_FORMAT_ASCII_ARB, 
          static_cast<GLsizei>(strlen(vtkMitkVolumeTextureMapper3D_OneComponentShadeFP)),
          vtkMitkVolumeTextureMapper3D_OneComponentShadeFP );

  this->SetupTwoDependentTextures(ren, vol);
  this->SetupProgramLocalsForShadingFP( ren, vol );

  // Start the timer now
  this->Timer->StartTimer();
  
  int stages[4] = {1,0,1,0};
  this->RenderPolygons( ren, vol, stages );  

  glDisable( vtkgl::FRAGMENT_PROGRAM_ARB );
  
  vtkgl::DeleteProgramsARB( 1, &fragmentProgram );
}

void vtkMitkOpenGLVolumeTextureMapper3D::RenderFourDependentShadeFP(
  vtkRenderer *ren,
  vtkVolume *vol )
{
  //GPU_INFO << "RenderFourDependentShadeFP";

  glEnable( vtkgl::FRAGMENT_PROGRAM_ARB );

  GLuint fragmentProgram;
  vtkgl::GenProgramsARB( 1, &fragmentProgram );

  vtkgl::BindProgramARB( vtkgl::FRAGMENT_PROGRAM_ARB, fragmentProgram );

  vtkgl::ProgramStringARB( vtkgl::FRAGMENT_PROGRAM_ARB,
          vtkgl::PROGRAM_FORMAT_ASCII_ARB, 
          static_cast<GLsizei>(strlen(vtkMitkVolumeTextureMapper3D_FourDependentShadeFP)),
          vtkMitkVolumeTextureMapper3D_FourDependentShadeFP );

  this->SetupFourDependentTextures(ren, vol);
  this->SetupProgramLocalsForShadingFP( ren, vol );

  // Start the timer now
  this->Timer->StartTimer();
  
  int stages[4] = {1,1,1,0};
  this->RenderPolygons( ren, vol, stages );  

  glDisable( vtkgl::FRAGMENT_PROGRAM_ARB );
  
  vtkgl::DeleteProgramsARB( 1, &fragmentProgram );
}


void vtkMitkOpenGLVolumeTextureMapper3D::GetLightInformation(
  vtkRenderer *ren,
  vtkVolume *vol,
  GLfloat lightDirection[2][4],
  GLfloat lightDiffuseColor[2][4],
  GLfloat lightSpecularColor[2][4],
  GLfloat halfwayVector[2][4],
  GLfloat ambientColor[4] )
{
  //GPU_INFO << "GetLightInformation";


  float ambient = vol->GetProperty()->GetAmbient();
  float diffuse  = vol->GetProperty()->GetDiffuse();
  float specular = vol->GetProperty()->GetSpecular();
  
  vtkTransform *volumeTransform = vtkTransform::New();
  
  volumeTransform->SetMatrix( vol->GetMatrix() );
  volumeTransform->Inverse();
  
  vtkLightCollection *lights = ren->GetLights();
  lights->InitTraversal();
  
  vtkLight *light[2];
  light[0] = lights->GetNextItem();
  light[1] = lights->GetNextItem();
  
  int lightIndex = 0;
  
  double cameraPosition[3];
  double cameraFocalPoint[3];
  
  ren->GetActiveCamera()->GetPosition( cameraPosition );
  ren->GetActiveCamera()->GetFocalPoint( cameraFocalPoint );
  
  double viewDirection[3];
  
  volumeTransform->TransformPoint( cameraPosition, cameraPosition );
  volumeTransform->TransformPoint( cameraFocalPoint, cameraFocalPoint );
  
  viewDirection[0] = cameraFocalPoint[0] - cameraPosition[0];
  viewDirection[1] = cameraFocalPoint[1] - cameraPosition[1];
  viewDirection[2] = cameraFocalPoint[2] - cameraPosition[2];
  
  vtkMath::Normalize( viewDirection );
  

  ambientColor[0] = 0.0;
  ambientColor[1] = 0.0;
  ambientColor[2] = 0.0;  
  ambientColor[3] = 0.0;  
  
  for ( lightIndex = 0; lightIndex < 2; lightIndex++ )
    {
    float dir[3] = {0,0,0};
    float half[3] = {0,0,0};
    
    if ( light[lightIndex] == NULL ||
         light[lightIndex]->GetSwitch() == 0 )
      {
      lightDiffuseColor[lightIndex][0] = 0.0;
      lightDiffuseColor[lightIndex][1] = 0.0;
      lightDiffuseColor[lightIndex][2] = 0.0;
      lightDiffuseColor[lightIndex][3] = 0.0;

      lightSpecularColor[lightIndex][0] = 0.0;
      lightSpecularColor[lightIndex][1] = 0.0;
      lightSpecularColor[lightIndex][2] = 0.0;
      lightSpecularColor[lightIndex][3] = 0.0;
      }
    else
      {
      float lightIntensity = light[lightIndex]->GetIntensity();
      double lightColor[3];
      
      light[lightIndex]->GetColor( lightColor );
      
      double lightPosition[3];
      double lightFocalPoint[3];
      light[lightIndex]->GetTransformedPosition( lightPosition );
      light[lightIndex]->GetTransformedFocalPoint( lightFocalPoint );

      volumeTransform->TransformPoint( lightPosition, lightPosition );
      volumeTransform->TransformPoint( lightFocalPoint, lightFocalPoint );
      
      dir[0] = lightPosition[0] - lightFocalPoint[0];
      dir[1] = lightPosition[1] - lightFocalPoint[1];
      dir[2] = lightPosition[2] - lightFocalPoint[2];
      
      vtkMath::Normalize( dir );
      
      lightDiffuseColor[lightIndex][0] = lightColor[0]*diffuse*lightIntensity;
      lightDiffuseColor[lightIndex][1] = lightColor[1]*diffuse*lightIntensity;
      lightDiffuseColor[lightIndex][2] = lightColor[2]*diffuse*lightIntensity;
      lightDiffuseColor[lightIndex][3] = 1.0;
      
      lightSpecularColor[lightIndex][0]= lightColor[0]*specular*lightIntensity;
      lightSpecularColor[lightIndex][1]= lightColor[1]*specular*lightIntensity;
      lightSpecularColor[lightIndex][2]= lightColor[2]*specular*lightIntensity;
      lightSpecularColor[lightIndex][3] = 0.0;

      half[0] = dir[0] - viewDirection[0];
      half[1] = dir[1] - viewDirection[1];
      half[2] = dir[2] - viewDirection[2];
      
      vtkMath::Normalize( half );
      
      ambientColor[0] += ambient*lightColor[0];
      ambientColor[1] += ambient*lightColor[1];
      ambientColor[2] += ambient*lightColor[2];      
      }

    lightDirection[lightIndex][0] = (dir[0]+1.0)/2.0;
    lightDirection[lightIndex][1] = (dir[1]+1.0)/2.0;
    lightDirection[lightIndex][2] = (dir[2]+1.0)/2.0;
    lightDirection[lightIndex][3] = 0.0;
    
    halfwayVector[lightIndex][0] = (half[0]+1.0)/2.0;
    halfwayVector[lightIndex][1] = (half[1]+1.0)/2.0;
    halfwayVector[lightIndex][2] = (half[2]+1.0)/2.0;
    halfwayVector[lightIndex][3] = 0.0;    
    }
  
  volumeTransform->Delete();
  
}

void vtkMitkOpenGLVolumeTextureMapper3D::SetupProgramLocalsForShadingFP(
  vtkRenderer *ren,
  vtkVolume *vol )
{
  //GPU_INFO << "SetupProgramLocalsForShadingFP";

  GLfloat lightDirection[2][4];
  GLfloat lightDiffuseColor[2][4];
  GLfloat lightSpecularColor[2][4];
  GLfloat halfwayVector[2][4];
  GLfloat ambientColor[4];

  float ambient       = vol->GetProperty()->GetAmbient();
  float diffuse       = vol->GetProperty()->GetDiffuse();
  float specular      = vol->GetProperty()->GetSpecular();
  float specularPower = vol->GetProperty()->GetSpecularPower();
  
  vtkTransform *volumeTransform = vtkTransform::New();
  
  volumeTransform->SetMatrix( vol->GetMatrix() );
  volumeTransform->Inverse();
  
  vtkLightCollection *lights = ren->GetLights();
  lights->InitTraversal();
  
  vtkLight *light[2];
  light[0] = lights->GetNextItem();
  light[1] = lights->GetNextItem();
  
  int lightIndex = 0;
  
  double cameraPosition[3];
  double cameraFocalPoint[3];
  
  ren->GetActiveCamera()->GetPosition( cameraPosition );
  ren->GetActiveCamera()->GetFocalPoint( cameraFocalPoint );
  
  volumeTransform->TransformPoint( cameraPosition, cameraPosition );
  volumeTransform->TransformPoint( cameraFocalPoint, cameraFocalPoint );
  
  double viewDirection[4];
  
  viewDirection[0] = cameraFocalPoint[0] - cameraPosition[0];
  viewDirection[1] = cameraFocalPoint[1] - cameraPosition[1];
  viewDirection[2] = cameraFocalPoint[2] - cameraPosition[2];
  viewDirection[3] = 0.0;
  
  vtkMath::Normalize( viewDirection );
  
  ambientColor[0] = 0.0;
  ambientColor[1] = 0.0;
  ambientColor[2] = 0.0;  
  ambientColor[3] = 0.0;  
  
  for ( lightIndex = 0; lightIndex < 2; lightIndex++ )
    {
    float dir[3] = {0,0,0};
    float half[3] = {0,0,0};
    
    if ( light[lightIndex] == NULL ||
         light[lightIndex]->GetSwitch() == 0 )
      {
      lightDiffuseColor[lightIndex][0] = 0.0;
      lightDiffuseColor[lightIndex][1] = 0.0;
      lightDiffuseColor[lightIndex][2] = 0.0;
      lightDiffuseColor[lightIndex][3] = 0.0;

      lightSpecularColor[lightIndex][0] = 0.0;
      lightSpecularColor[lightIndex][1] = 0.0;
      lightSpecularColor[lightIndex][2] = 0.0;
      lightSpecularColor[lightIndex][3] = 0.0;
      }
    else
      {
      float lightIntensity = light[lightIndex]->GetIntensity();
      double lightColor[3];
      
      light[lightIndex]->GetColor( lightColor );
      
      double lightPosition[3];
      double lightFocalPoint[3];
      light[lightIndex]->GetTransformedPosition( lightPosition );
      light[lightIndex]->GetTransformedFocalPoint( lightFocalPoint );
      
      volumeTransform->TransformPoint( lightPosition, lightPosition );
      volumeTransform->TransformPoint( lightFocalPoint, lightFocalPoint );
      
      dir[0] = lightPosition[0] - lightFocalPoint[0];
      dir[1] = lightPosition[1] - lightFocalPoint[1];
      dir[2] = lightPosition[2] - lightFocalPoint[2];
      
      vtkMath::Normalize( dir );
      
      lightDiffuseColor[lightIndex][0] = lightColor[0]*diffuse*lightIntensity;
      lightDiffuseColor[lightIndex][1] = lightColor[1]*diffuse*lightIntensity;
      lightDiffuseColor[lightIndex][2] = lightColor[2]*diffuse*lightIntensity;
      lightDiffuseColor[lightIndex][3] = 0.0;
      
      lightSpecularColor[lightIndex][0]= lightColor[0]*specular*lightIntensity;
      lightSpecularColor[lightIndex][1]= lightColor[1]*specular*lightIntensity;
      lightSpecularColor[lightIndex][2]= lightColor[2]*specular*lightIntensity;
      lightSpecularColor[lightIndex][3] = 0.0;

      half[0] = dir[0] - viewDirection[0];
      half[1] = dir[1] - viewDirection[1];
      half[2] = dir[2] - viewDirection[2];
      
      vtkMath::Normalize( half );
      
      ambientColor[0] += ambient*lightColor[0];
      ambientColor[1] += ambient*lightColor[1];
      ambientColor[2] += ambient*lightColor[2];      
      }

    lightDirection[lightIndex][0] = dir[0];
    lightDirection[lightIndex][1] = dir[1];
    lightDirection[lightIndex][2] = dir[2];
    lightDirection[lightIndex][3] = 0.0;
    
    halfwayVector[lightIndex][0] = half[0];
    halfwayVector[lightIndex][1] = half[1];
    halfwayVector[lightIndex][2] = half[2];
    halfwayVector[lightIndex][3] = 0.0;    
    }
  
  volumeTransform->Delete();

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 0, 
              lightDirection[0][0],
              lightDirection[0][1],
              lightDirection[0][2],
              lightDirection[0][3] );

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 1, 
              halfwayVector[0][0],
              halfwayVector[0][1],
              halfwayVector[0][2],
              halfwayVector[0][3] );

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 2,
              ambient, diffuse, specular, specularPower );

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 3,
              lightDiffuseColor[0][0],
              lightDiffuseColor[0][1],
              lightDiffuseColor[0][2],
              lightDiffuseColor[0][3] );

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 4,
              lightSpecularColor[0][0],
              lightSpecularColor[0][1],
              lightSpecularColor[0][2],
              lightSpecularColor[0][3] );

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 5,
              viewDirection[0],
              viewDirection[1],
              viewDirection[2],
              viewDirection[3] );

  vtkgl::ProgramLocalParameter4fARB( vtkgl::FRAGMENT_PROGRAM_ARB, 6,
                                     2.0, -1.0, 0.0, 0.0 );
}

int  vtkMitkOpenGLVolumeTextureMapper3D::IsRenderSupported(
  vtkVolumeProperty *property )
{
  //GPU_INFO << "IsRenderSupported";

  if ( !this->Initialized )
    {
    this->Initialize();
    }
  
  if ( !this->RenderPossible )
    {
    return 0;
    }
  
  if ( !this->GetInput() )
    {
    return 0;
    }
  
  if ( this->GetInput()->GetNumberOfScalarComponents() > 1 &&
       property->GetIndependentComponents() )
    {
    return 0;
    }
  
  return 1;
}

void vtkMitkOpenGLVolumeTextureMapper3D::Initialize()
{
  //GPU_INFO << "Initialize";

  this->Initialized = 1;
  vtkOpenGLExtensionManager * extensions = vtkOpenGLExtensionManager::New();
  extensions->SetRenderWindow(NULL); // set render window to the current one.
  
  int supports_texture3D=extensions->ExtensionSupported( "GL_VERSION_1_2" );
  if(supports_texture3D)
    {
    extensions->LoadExtension("GL_VERSION_1_2");
    }
  else
    {
    supports_texture3D=extensions->ExtensionSupported( "GL_EXT_texture3D" );
    if(supports_texture3D)
      {
      extensions->LoadCorePromotedExtension("GL_EXT_texture3D");
      }
    }
  
  int supports_multitexture=extensions->ExtensionSupported( "GL_VERSION_1_3" );
  if(supports_multitexture)
    {
    extensions->LoadExtension("GL_VERSION_1_3");
    }
  else
    {
    supports_multitexture=
      extensions->ExtensionSupported("GL_ARB_multitexture");
    if(supports_multitexture)
      {
      extensions->LoadCorePromotedExtension("GL_ARB_multitexture");
      }
    }
  
  this->SupportsCompressedTexture=
    extensions->ExtensionSupported("GL_VERSION_1_3")==1;
  
  if(!this->SupportsCompressedTexture)
    {
    this->SupportsCompressedTexture=
      extensions->ExtensionSupported("GL_ARB_texture_compression")==1;
    if(this->SupportsCompressedTexture)
      {
      extensions->LoadCorePromotedExtension("GL_ARB_texture_compression");
      }
    }
    
  //GPU_INFO(this->SupportsCompressedTexture) << "supporting compressed textures";
  
  const char *gl_version=
    reinterpret_cast<const char *>(glGetString(GL_VERSION));
  const char *mesa_version=strstr(gl_version,"Mesa");
  
  
  // Workaround for broken Mesa
  if(mesa_version!=0) // any Mesa
    {
    this->SupportsCompressedTexture=false;
    }
  
  this->SupportsNonPowerOfTwoTextures=
        extensions->ExtensionSupported("GL_VERSION_2_0")
        || extensions->ExtensionSupported("GL_ARB_texture_non_power_of_two");

  //GPU_INFO << "np2: " << (this->SupportsNonPowerOfTwoTextures?1:0);
  
  bool brokenMesa=false;
  
  if(mesa_version!=0)
    {
    // Workaround for broken Mesa (dash16-sql):
    // GL_VENDOR="Mesa project: www.mesa3d.org"
    // GL_VERSION="1.4 (2.1 Mesa 7.0.4)"
    // GL_RENDERER="Mesa GLX Indirect"
    // there is no problem with (dash6):
    // GL_VENDOR="Brian Paul"
    // GL_VERSION="2.0 Mesa 7.0.4"
    // GL_RENDERER="Mesa X11"
    // glGetIntegerv(vtkgl::MAX_3D_TEXTURE_SIZE,&maxSize) return some
    // uninitialized value and a loading a Luminance-alpha 16x16x16 just
    // crashes glx.
    int mesa_major=0;
    int mesa_minor=0;
    int mesa_patch=0;
    int opengl_major=0;
    int opengl_minor=0;
    if(sscanf(gl_version,"%d.%d",&opengl_major, &opengl_minor)>=2)
      {
      if(opengl_major==1 && opengl_minor==4)
        {
        if(sscanf(mesa_version,"Mesa %d.%d.%d",&mesa_major,
                  &mesa_minor,&mesa_patch)>=3)
          {
          brokenMesa=mesa_major==7 && mesa_minor==0 && mesa_patch==4;
          }
        }
      }
    }
  
  int supports_GL_NV_texture_shader2     = extensions->ExtensionSupported( "GL_NV_texture_shader2" );
  int supports_GL_NV_register_combiners2 = extensions->ExtensionSupported( "GL_NV_register_combiners2" );
  int supports_GL_ATI_fragment_shader    = extensions->ExtensionSupported( "GL_ATI_fragment_shader" );
  int supports_GL_ARB_fragment_program   = extensions->ExtensionSupported( "GL_ARB_fragment_program" );
  int supports_GL_ARB_vertex_program     = extensions->ExtensionSupported( "GL_ARB_vertex_program" );
  int supports_GL_NV_register_combiners  = extensions->ExtensionSupported( "GL_NV_register_combiners" );
  
  if(supports_GL_NV_texture_shader2) 
    {
    extensions->LoadExtension("GL_NV_texture_shader2" );
    }
  
  if(supports_GL_NV_register_combiners2)  
    {
    extensions->LoadExtension( "GL_NV_register_combiners2" );
    }
  
  if(supports_GL_ATI_fragment_shader)     
    {
    extensions->LoadExtension( "GL_ATI_fragment_shader" );
    }
  
  if(supports_GL_ARB_fragment_program)    
    {
    extensions->LoadExtension( "GL_ARB_fragment_program" );
    }
  
  if(supports_GL_ARB_vertex_program)    
    {
    extensions->LoadExtension( "GL_ARB_vertex_program" );
    }
  
  if(supports_GL_NV_register_combiners)  
    {
    extensions->LoadExtension( "GL_NV_register_combiners" );
    }

  extensions->Delete();
  
  
  int canDoFP = 0;
  int canDoNV = 0;
  
  if ( !brokenMesa &&
       supports_texture3D          &&
       supports_multitexture       &&
       supports_GL_ARB_fragment_program   &&
       supports_GL_ARB_vertex_program     &&
       vtkgl::TexImage3D               &&
       vtkgl::ActiveTexture            &&
       vtkgl::MultiTexCoord3fv         &&
       vtkgl::GenProgramsARB              &&
       vtkgl::DeleteProgramsARB           &&
       vtkgl::BindProgramARB              &&
       vtkgl::ProgramStringARB            &&
       vtkgl::ProgramLocalParameter4fARB )
    {    
    canDoFP = 1;
    }
    else
    {
      std::string errString = "no gpu-acceleration possible cause following extensions/methods are missing or unsupported:";
            
      if(!supports_texture3D) errString += " EXT_TEXTURE3D";
      if(!supports_multitexture) errString += " EXT_MULTITEXTURE";
      if(!supports_GL_ARB_fragment_program) errString += " ARB_FRAGMENT_PROGRAM";
      if(!supports_GL_ARB_vertex_program) errString += " ARB_VERTEX_PROGRAM";
      if(!vtkgl::TexImage3D) errString += " glTexImage3D";
      if(!vtkgl::ActiveTexture) errString += " glActiveTexture";
      if(!vtkgl::MultiTexCoord3fv) errString += " glMultiTexCoord3fv";
      if(!vtkgl::GenProgramsARB) errString += " glGenProgramsARB";
      if(!vtkgl::DeleteProgramsARB) errString += " glDeleteProgramsARB";
      if(!vtkgl::BindProgramARB) errString += " glBindProgramARB";
      if(!vtkgl::ProgramStringARB) errString += " glProgramStringARB";
      if(!vtkgl::ProgramLocalParameter4fARB) errString += " glProgramLocalParameter4fARB";
  
      GPU_WARN << errString;
    };

  this->RenderPossible=canDoFP;  
  
  if(RenderPossible)
  {
    vtkgl::GenProgramsARB( 1, &prgOneComponentShade );
    vtkgl::BindProgramARB( vtkgl::FRAGMENT_PROGRAM_ARB, prgOneComponentShade );
    vtkgl::ProgramStringARB( vtkgl::FRAGMENT_PROGRAM_ARB,
          vtkgl::PROGRAM_FORMAT_ASCII_ARB, 
          static_cast<GLsizei>(strlen(vtkMitkVolumeTextureMapper3D_OneComponentShadeFP)),
          vtkMitkVolumeTextureMapper3D_OneComponentShadeFP );
  }

}

// ----------------------------------------------------------------------------
int vtkMitkOpenGLVolumeTextureMapper3D::IsTextureSizeSupported(int size[3],
                                                           int components)
{
  //GPU_INFO << "IsTextureSizeSupported";

  GLint maxSize;
  glGetIntegerv(vtkgl::MAX_3D_TEXTURE_SIZE,&maxSize);
  
  if(size[0]>maxSize || size[1]>maxSize || size[2]>maxSize)
    {
    return 0;
    }
  
  GLuint id1;
  glGenTextures(1,&id1);
  glBindTexture(vtkgl::TEXTURE_3D,id1);
  if(components==1)
    {
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalLA,size[0], 
                      size[1], size[2], 0, GL_LUMINANCE_ALPHA,
                      GL_UNSIGNED_BYTE,0);
    }
  else
    {
    vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,size[0], 
                      size[1], size[2], 0, GL_RGB,
                      GL_UNSIGNED_BYTE,0);
    }
  bool result=glGetError()==GL_NO_ERROR;
  if(result) // ie. not GL_OUT_OF_MEMORY
    {
    GLuint id2;
    glGenTextures(1,&id2);
    glBindTexture(vtkgl::TEXTURE_3D,id2);
    if(components==4)
      {
      vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalLA,size[0], 
                        size[1], size[2], 0, GL_LUMINANCE_ALPHA,
                        GL_UNSIGNED_BYTE,0);
      }
    else
      {
      vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,size[0], 
                        size[1], size[2], 0, GL_RGB,
                        GL_UNSIGNED_BYTE,0);
      }
    result=glGetError()==GL_NO_ERROR;
    if(result && components==4) // ie. not GL_OUT_OF_MEMORY
      {
      GLuint id3;
      glGenTextures(1,&id3);
      glBindTexture(vtkgl::TEXTURE_3D,id3);
      vtkgl::TexImage3D(vtkgl::TEXTURE_3D,0,this->InternalRGB,size[0], 
                        size[1], size[2], 0, GL_RGB,
                        GL_UNSIGNED_BYTE,0);
      result=glGetError()==GL_NO_ERROR;
      glBindTexture(vtkgl::TEXTURE_3D,0); // bind to default texture object.
      glDeleteTextures(1,&id3);
      }
    glBindTexture(vtkgl::TEXTURE_3D,0); // bind to default texture object.
    glDeleteTextures(1,&id2);
    }
  glBindTexture(vtkgl::TEXTURE_3D,0); // bind to default texture object.
  glDeleteTextures(1,&id1);
  return result;
}

// ----------------------------------------------------------------------------
// Print the vtkMitkOpenGLVolumeTextureMapper3D
void vtkMitkOpenGLVolumeTextureMapper3D::PrintSelf(ostream& os, vtkIndent indent)
{
 
  vtkOpenGLExtensionManager * extensions = vtkOpenGLExtensionManager::New();
  extensions->SetRenderWindow(NULL); // set render window to current render window
  
  os << indent << "Initialized " << this->Initialized << endl;
  if ( this->Initialized )
    {
    os << indent << "Supports GL_VERSION_1_2:" 
       << extensions->ExtensionSupported( "GL_VERSION_1_2" ) << endl;
    os << indent << "Supports GL_EXT_texture3D:" 
       << extensions->ExtensionSupported( "GL_EXT_texture3D" ) << endl;
     os << indent << "Supports GL_VERSION_1_3:" 
       << extensions->ExtensionSupported( "GL_VERSION_1_3" ) << endl;
    os << indent << "Supports GL_ARB_multitexture: " 
       << extensions->ExtensionSupported( "GL_ARB_multitexture" ) << endl;
    os << indent << "Supports GL_NV_texture_shader2: " 
       << extensions->ExtensionSupported( "GL_NV_texture_shader2" ) << endl;
    os << indent << "Supports GL_NV_register_combiners2: " 
       << extensions->ExtensionSupported( "GL_NV_register_combiners2" )
       << endl;
    os << indent << "Supports GL_ATI_fragment_shader: " 
       << extensions->ExtensionSupported( "GL_ATI_fragment_shader" ) << endl;
    os << indent << "Supports GL_ARB_fragment_program: "
       << extensions->ExtensionSupported( "GL_ARB_fragment_program" ) << endl;
    os << indent << "Supports GL_ARB_texture_compression: "
       << extensions->ExtensionSupported( "GL_ARB_texture_compression" )
       << endl;
    os << indent << "Supports GL_VERSION_2_0:"
       << extensions->ExtensionSupported( "GL_VERSION_2_0" )
       << endl;
    os << indent << "Supports GL_ARB_texture_non_power_of_two:"
       << extensions->ExtensionSupported( "GL_ARB_texture_non_power_of_two" )
       << endl;
    }
  extensions->Delete();
  
  this->Superclass::PrintSelf(os,indent);
}



