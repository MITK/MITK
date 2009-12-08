/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMitkVolumeTextureMapper3D.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMitkVolumeTextureMapper3D.h"
#include "mitkCommon.h"

#define GPU_INFO LOG_INFO("mapper.vr")
#define GPU_WARN LOG_WARN("mapper.vr")

#include "vtkCamera.h"
#include "vtkColorTransferFunction.h"
#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPointData.h"
#include "vtkRenderer.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"
#include "vtkVolumeRenderingFactory.h"

vtkCxxRevisionMacro(vtkMitkVolumeTextureMapper3D, "$Revision: 1.16 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkMitkVolumeTextureMapper3D);
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
vtkMitkVolumeTextureMapper3D::vtkMitkVolumeTextureMapper3D()
{
  //GPU_INFO << "vtkMitkVolumeTextureMapper3D";

  this->PolygonBuffer                 = NULL;
  this->IntersectionBuffer            = NULL;
  this->NumberOfPolygons              = 0;
  this->BufferSize                    = 0;

  // The input used when creating the textures
  this->SavedTextureInput             = NULL;
  
  // The input used when creating the color tables
  this->SavedParametersInput           = NULL;
  
  this->SavedRGBFunction              = NULL;
  this->SavedGrayFunction             = NULL;
  this->SavedScalarOpacityFunction    = NULL;
  this->SavedGradientOpacityFunction  = NULL;
  this->SavedColorChannels            = 0;
  this->SavedSampleDistance           = 0;
  this->SavedScalarOpacityDistance    = 0;

/*
  this->Volume1                       = NULL;
  this->Volume2                       = NULL;
  this->Volume3                       = NULL;
  */
  /*
  this->VolumeSize                    = 0;
  this->VolumeComponents              = 0;
    */
  
  this->VolumeSpacing[0] = this->VolumeSpacing[1] = this->VolumeSpacing[2] = 0;
  this->VolumeDimensions[0]=0;
  this->VolumeDimensions[1]=0;
  this->VolumeDimensions[2]=0;
  
  this->SampleDistance                = 1.0;
  this->ActualSampleDistance          = 1.0;
  
  this->UseCompressedTexture          = false;
  this->SupportsNonPowerOfTwoTextures = false;

  //GPU_INFO << "np2: " << (this->SupportsNonPowerOfTwoTextures?1:0);
}

//-----------------------------------------------------------------------------
vtkMitkVolumeTextureMapper3D::~vtkMitkVolumeTextureMapper3D()
{
  //GPU_INFO << "~vtkMitkVolumeTextureMapper3D";

  delete [] this->PolygonBuffer;
  delete [] this->IntersectionBuffer;
  /*
  delete [] this->Volume1;
  delete [] this->Volume2;
  delete [] this->Volume3;
  */
}


//-----------------------------------------------------------------------------
vtkMitkVolumeTextureMapper3D *vtkMitkVolumeTextureMapper3D::New()
{
  //GPU_INFO << "New";
 
   // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkVolumeRenderingFactory::CreateInstance("vtkMitkVolumeTextureMapper3D");
  return static_cast<vtkMitkVolumeTextureMapper3D *>(ret);
}

//-----------------------------------------------------------------------------
void vtkMitkVolumeTextureMapper3D::ComputePolygons( vtkRenderer *ren, 
                                                vtkVolume *vol,
                                                double inBounds[6] )
{
  //GPU_INFO << "ComputePolygons";

  // Get the camera position and focal point
  double focalPoint[4], position[4];
  double plane[4];
  vtkCamera *camera = ren->GetActiveCamera();

  camera->GetPosition( position );
  camera->GetFocalPoint( focalPoint );
 
  position[3]   = 1.0;
  focalPoint[3] = 1.0;
  
  // Pass the focal point and position through the inverse of the 
  // volume's matrix to map back into the data coordinates. We
  // are going to compute these polygons in the coordinate system
  // of the input data - this is easiest since this data must be 
  // axis aligned. Then we'll use OpenGL to transform these polygons
  // into the world coordinate system through the use of the
  // volume's matrix.
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  vol->GetMatrix( matrix );
  matrix->Invert();
  matrix->MultiplyPoint( position, position );
  matrix->MultiplyPoint( focalPoint, focalPoint );
  matrix->Delete();
  
  if ( position[3] )
    {
    position[0] /= position[3];
    position[1] /= position[3];
    position[2] /= position[3];
    }

  if ( focalPoint[3] )
    {
    focalPoint[0] /= focalPoint[3];
    focalPoint[1] /= focalPoint[3];
    focalPoint[2] /= focalPoint[3];
    }

  // Create a plane equation using the direction and position of the camera
  plane[0] = focalPoint[0] - position[0];
  plane[1] = focalPoint[1] - position[1];
  plane[2] = focalPoint[2] - position[2];
  
  vtkMath::Normalize( plane );
  
  plane[3] = -(plane[0] * position[0] + plane[1] * position[1] +
               plane[2] * position[2]);
 
  // Find the min and max distances of the boundary points of the volume
  double minDistance = VTK_DOUBLE_MAX;
  double maxDistance = VTK_DOUBLE_MIN;
 
  // The inBounds parameter is the bounds we are using for clipping the
  // texture planes against. First we need to clip these against the bounds
  // of the volume to make sure they don't exceed it.
  double volBounds[6];
  this->GetInput()->GetBounds( volBounds );

  double bounds[6];
  bounds[0] = (inBounds[0]>volBounds[0])?(inBounds[0]):(volBounds[0]);
  bounds[1] = (inBounds[1]<volBounds[1])?(inBounds[1]):(volBounds[1]);
  bounds[2] = (inBounds[2]>volBounds[2])?(inBounds[2]):(volBounds[2]);
  bounds[3] = (inBounds[3]<volBounds[3])?(inBounds[3]):(volBounds[3]);
  bounds[4] = (inBounds[4]>volBounds[4])?(inBounds[4]):(volBounds[4]);
  bounds[5] = (inBounds[5]<volBounds[5])?(inBounds[5]):(volBounds[5]);
 
  // Create 8 vertices for the bounding box we are rendering
  int i, j, k;
  double vertices[8][3];
 
  int idx = 0;
 
  for ( k = 0; k < 2; k++ )
    {
    for ( j = 0; j < 2; j++ )
      {
      for ( i = 0; i < 2; i++ )
        {
        vertices[idx][2] = bounds[4+k];
        vertices[idx][1] = bounds[2+j];
        vertices[idx][0] = bounds[i];

        double d = 
          plane[0] * vertices[idx][0] +
          plane[1] * vertices[idx][1] +
          plane[2] * vertices[idx][2] +
          plane[3];

        idx++;

        // Keep track of closest and farthest point
        minDistance = (d<minDistance)?(d):(minDistance);
        maxDistance = (d>maxDistance)?(d):(maxDistance);

        }
      }
    }

  int dim[6];
  this->GetVolumeDimensions(dim);
 
  float tCoordOffset[3], tCoordScale[3];

  tCoordOffset[0] = 0.5 / dim[0];
  tCoordOffset[1] = 0.5 / dim[1];
  tCoordOffset[2] = 0.5 / dim[2];
 
  tCoordScale[0] =  (dim[0]-1) / static_cast<float>(dim[0]);
  tCoordScale[1] =  (dim[1]-1) / static_cast<float>(dim[1]);
  tCoordScale[2] =  (dim[2]-1) / static_cast<float>(dim[2]);

  float spacing[3];
  this->GetVolumeSpacing( spacing );
 
  double offset = 
    0.333 * 0.5 * (spacing[0] + spacing[1] + spacing[2]);
 
  minDistance += 0.1*offset;
  maxDistance -= 0.1*offset;
 
  minDistance = (minDistance < offset)?(offset):(minDistance);
 
  double stepSize = this->ActualSampleDistance;
 
  // Determine the number of polygons
  int numPolys = static_cast<int>(
    (maxDistance - minDistance)/static_cast<double>(stepSize));
 
  // Check if we have space, free old space only if it is too small 
  if ( this->BufferSize < numPolys )
    {
    delete [] this->PolygonBuffer;
    delete [] this->IntersectionBuffer;

    this->BufferSize = numPolys;

    this->PolygonBuffer = new float [36*this->BufferSize];
    this->IntersectionBuffer = new float [12*this->BufferSize];
    }
 
  this->NumberOfPolygons = numPolys;
 
  // Compute the intersection points for each edge of the volume
  int lines[12][2] = { {0,1}, {1,3}, {2,3}, {0,2},
                       {4,5}, {5,7}, {6,7}, {4,6},
                       {0,4}, {1,5}, {3,7}, {2,6} };
 
  float *iptr, *pptr;
 
  for ( i = 0; i < 12; i++ )
    {
    double line[3];

    line[0] = vertices[lines[i][1]][0] - vertices[lines[i][0]][0];
    line[1] = vertices[lines[i][1]][1] - vertices[lines[i][0]][1];
    line[2] = vertices[lines[i][1]][2] - vertices[lines[i][0]][2];
 
    double d = maxDistance;
 
    iptr = this->IntersectionBuffer + i;

    double planeDotLineOrigin = vtkMath::Dot( plane, vertices[lines[i][0]] );
    double planeDotLine       = vtkMath::Dot( plane, line );
 
    double t, increment;

    if ( planeDotLine != 0.0 )
      {
      t = (d - planeDotLineOrigin - plane[3] ) / planeDotLine;
      increment = -stepSize / planeDotLine;
      }
    else
      {
      t         = -1.0;
      increment =  0.0;
      }

    for ( j = 0; j < numPolys; j++ )
      {
      *iptr = (t > 0.0 && t < 1.0)?(t):(-1.0);

      t += increment;
      iptr += 12;
      }
    }

  // Compute the polygons by determining which edges were intersected
  int neighborLines[12][6] = 
  { {  1,  2,  3,  4,  8,  9}, {  0,  2,  3,  5,  9, 10},
    {  0,  1,  3,  6, 10, 11}, {  0,  1,  2,  7,  8, 11},
    {  0,  5,  6,  7,  8,  9}, {  1,  4,  6,  7,  9, 10},
    {  2,  4,  5,  7, 10, 11}, {  3,  4,  5,  6,  8, 11},
    {  0,  3,  4,  7,  9, 11}, {  0,  1,  4,  5,  8, 10},
    {  1,  2,  5,  6,  9, 11}, {  2,  3,  6,  7,  8, 10} };

  float tCoord[12][4] =
  {{0,0,0,0}, {1,0,0,1}, {0,1,0,0}, {0,0,0,1},
   {0,0,1,0}, {1,0,1,1}, {0,1,1,0}, {0,0,1,1},
   {0,0,0,2}, {1,0,0,2}, {1,1,0,2}, {0,1,0,2}};
 
  double low[3];
  double high[3];

  low[0]  = (bounds[0] - volBounds[0]) / (volBounds[1] - volBounds[0]);
  high[0] = (bounds[1] - volBounds[0]) / (volBounds[1] - volBounds[0]);
  low[1]  = (bounds[2] - volBounds[2]) / (volBounds[3] - volBounds[2]);
  high[1] = (bounds[3] - volBounds[2]) / (volBounds[3] - volBounds[2]);
  low[2]  = (bounds[4] - volBounds[4]) / (volBounds[5] - volBounds[4]);
  high[2] = (bounds[5] - volBounds[4]) / (volBounds[5] - volBounds[4]);

  for ( i = 0; i < 12; i++ )
    {
    tCoord[i][0] = (tCoord[i][0])?(high[0]):(low[0]);
    tCoord[i][1] = (tCoord[i][1])?(high[1]):(low[1]);
    tCoord[i][2] = (tCoord[i][2])?(high[2]):(low[2]);
    }

  iptr = this->IntersectionBuffer;
  pptr = this->PolygonBuffer;
 
  for ( i = 0; i < numPolys; i++ )
    {
    // Look for a starting point
    int start = 0;

    while ( start < 12 && iptr[start] == -1.0 )
      {
      start++;
      }

    if ( start == 12 )
      {
      pptr[0] = -1.0;
      }
    else
      {
      int current = start;
      int previous = -1;
      int errFlag = 0;

      idx   = 0;

      while ( idx < 6 && !errFlag && ( idx == 0 || current != start) )
        {
        double t = iptr[current];

        *(pptr + idx*6)     = 
          tCoord[current][0] * tCoordScale[0] + tCoordOffset[0];
        *(pptr + idx*6 + 1) = 
          tCoord[current][1] * tCoordScale[1] + tCoordOffset[1];
        *(pptr + idx*6 + 2) = 
          tCoord[current][2] * tCoordScale[2] + tCoordOffset[2];
        
        int coord = static_cast<int>(tCoord[current][3]);
        *(pptr + idx*6 + coord) = 
          (low[coord] + t*(high[coord]-low[coord]))*tCoordScale[coord] + tCoordOffset[coord];

        *(pptr + idx*6 + 3) = static_cast<float>(
          vertices[lines[current][0]][0] + 
          t*(vertices[lines[current][1]][0] - vertices[lines[current][0]][0]));
        
        *(pptr + idx*6 + 4) = static_cast<float>(
          vertices[lines[current][0]][1] + 
          t*(vertices[lines[current][1]][1] - vertices[lines[current][0]][1]));
        
        *(pptr + idx*6 + 5) = static_cast<float>(
          vertices[lines[current][0]][2] + 
          t*(vertices[lines[current][1]][2] - vertices[lines[current][0]][2]));

        idx++;

        j = 0;

        while ( j < 6 &&
                (*(this->IntersectionBuffer + i*12 + 
                   neighborLines[current][j]) < 0 || 
                 neighborLines[current][j] == previous) ) 
          {
          j++;
          }

        if ( j >= 6 )
          {
          errFlag = 1;
          }
        else
          {
          previous = current;
          current = neighborLines[current][j];
          }
        }

      if ( idx < 6 )
        {
        *(pptr + idx*6) = -1;
        }
      }

    iptr += 12;
    pptr += 36;
    }
}

void vtkMitkVolumeTextureMapper3D::UpdateMTime()
{
  this->SavedTextureMTime.Modified();
}




//-----------------------------------------------------------------------------
int vtkMitkVolumeTextureMapper3D::UpdateColorLookup( vtkVolume *vol )
{
  //GPU_INFO << "UpdateColorLookup";

  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetInput();
  input->Update();

  // Has the volume changed in some way?
  if ( this->SavedParametersInput != input ||
       this->SavedParametersMTime.GetMTime() < input->GetMTime() )
    {
    needToUpdate = 1;
    }
 
  // What sample distance are we going to use for rendering? If we
  // have to render quickly according to our allocated render time,
  // don't necessary obey the sample distance requested by the user.
  // Instead set the sample distance to the average spacing.
  this->ActualSampleDistance = this->SampleDistance;
  if ( vol->GetAllocatedRenderTime() < 1.0 )
    {
    float spacing[3];
    this->GetVolumeSpacing(spacing);
    this->ActualSampleDistance = 
      0.333 * (static_cast<double>(spacing[0]) + static_cast<double>(spacing[1]) + static_cast<double>(spacing[2]));
    }

  // How many components?
  int components = input->GetNumberOfScalarComponents();

  // Has the sample distance changed?
  if ( this->SavedSampleDistance != this->ActualSampleDistance )
    {
    needToUpdate = 1;
    }

  vtkColorTransferFunction *rgbFunc  = NULL;
  vtkPiecewiseFunction     *grayFunc = NULL;
 
  // How many color channels for this component?
  int colorChannels = vol->GetProperty()->GetColorChannels(0);

  if ( components < 3 )
    {
    // Has the number of color channels changed?
    if ( this->SavedColorChannels != colorChannels )
      {
      needToUpdate = 1;
      }

    // Has the color transfer function changed in some way,
    // and we are using it?
    if ( colorChannels == 3 )
      {
      rgbFunc  = vol->GetProperty()->GetRGBTransferFunction(0);
      if ( this->SavedRGBFunction != rgbFunc ||
           this->SavedParametersMTime.GetMTime() < rgbFunc->GetMTime() )
        {
        needToUpdate = 1;
        }
      }

    // Has the gray transfer function changed in some way,
    // and we are using it?
    if ( colorChannels == 1 )
      {
      grayFunc = vol->GetProperty()->GetGrayTransferFunction(0);
      if ( this->SavedGrayFunction != grayFunc ||
           this->SavedParametersMTime.GetMTime() < grayFunc->GetMTime() )
        {
        needToUpdate = 1;
        }
      }
    }
 
  // Has the scalar opacity transfer function changed in some way?
  vtkPiecewiseFunction *scalarOpacityFunc = 
    vol->GetProperty()->GetScalarOpacity(0);
  if ( this->SavedScalarOpacityFunction != scalarOpacityFunc ||
       this->SavedParametersMTime.GetMTime() < 
       scalarOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }

  // Has the gradient opacity transfer function changed in some way?
  vtkPiecewiseFunction *gradientOpacityFunc = 
    vol->GetProperty()->GetGradientOpacity(0);
  if ( this->SavedGradientOpacityFunction != gradientOpacityFunc ||
       this->SavedParametersMTime.GetMTime() < 
       gradientOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }


  double scalarOpacityDistance = 
    vol->GetProperty()->GetScalarOpacityUnitDistance(0);
  if ( this->SavedScalarOpacityDistance != scalarOpacityDistance )
    {
    needToUpdate = 1;
    }
  
  // If we have not found any need to update, return now
  if ( !needToUpdate )
    {
    return 0;
    }

  this->SavedRGBFunction             = rgbFunc;
  this->SavedGrayFunction            = grayFunc;
  this->SavedScalarOpacityFunction   = scalarOpacityFunc;
  this->SavedGradientOpacityFunction = gradientOpacityFunc;
  this->SavedColorChannels           = colorChannels;
  this->SavedSampleDistance          = this->ActualSampleDistance;
  this->SavedScalarOpacityDistance   = scalarOpacityDistance;
  this->SavedParametersInput         = input;
  
  this->SavedParametersMTime.Modified();

  // Find the scalar range
  double scalarRange[2];
  input->GetPointData()->GetScalars()->GetRange(scalarRange, components-1);
  
  int arraySizeNeeded = this->ColorTableSize;

  if ( components < 3 )
    {
    // Sample the transfer functions between the min and max.
    if ( colorChannels == 1 )
      {
      grayFunc->GetTable( scalarRange[0], scalarRange[1], 
                          arraySizeNeeded, this->TempArray1 );
      }
    else
      {
      rgbFunc->GetTable( scalarRange[0], scalarRange[1], 
                         arraySizeNeeded, this->TempArray1 );
      }
    }
  
  scalarOpacityFunc->GetTable( scalarRange[0], scalarRange[1], 
                               arraySizeNeeded, this->TempArray2 );

  float goArray[256];
  gradientOpacityFunc->GetTable( 0, (scalarRange[1] - scalarRange[0])*0.25, 
                                 256, goArray );

  // Correct the opacity array for the spacing between the planes.
  int i;

  float *fptr2 = this->TempArray2;
  double factor = this->ActualSampleDistance / scalarOpacityDistance;
  for ( i = 0; i < arraySizeNeeded; i++ )
    {
    if ( *fptr2 > 0.0001 )
      {
      *fptr2 =  1.0-pow(static_cast<double>(1.0-(*fptr2)),factor);
      }
    fptr2++;
    }

  int goLoop;
  unsigned char *ptr, *rgbptr, *aptr;
  float *fptr1;
 
  switch (components)
    {
    case 1:
      // Move the two temp float arrays into one RGBA unsigned char array
      ptr = this->ColorLookup;
      for ( goLoop = 0; goLoop < 256; goLoop++ )
        {
        fptr1 = this->TempArray1;
        fptr2 = this->TempArray2;
        if ( colorChannels == 1 )
          {
          for ( i = 0; i < arraySizeNeeded; i++ )
            {
            *(ptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(ptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
            *(ptr++) = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
            }
          }
        else
          {
          for ( i = 0; i < arraySizeNeeded; i++ )
            {
            *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
            *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
            *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
            *(ptr++) = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
            }
          }

        for ( ; i < 256; i++ )
          {
          *(ptr++) = 0;
          *(ptr++) = 0;
          *(ptr++) = 0;
          *(ptr++) = 0;
          }
        }
      break;

    case 2:
      // Move the two temp float arrays into one RGB unsigned char array and
      // one alpha array. 
      rgbptr = this->ColorLookup;
      aptr   = this->AlphaLookup;
      
      if ( colorChannels == 1 )
        {  
        for ( i = 0; i < arraySizeNeeded; i++ )
          {
          fptr1 = this->TempArray1;
          fptr2 = this->TempArray2;
          for ( goLoop = 0; goLoop < 256; goLoop++ )
            {
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
            *(aptr++)   = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
            }
          }
        }
      else
        {
        fptr1 = this->TempArray1;
        fptr2 = this->TempArray2;
        for ( i = 0; i < arraySizeNeeded; i++ )
          {
          for ( goLoop = 0; goLoop < 256; goLoop++ )
            {
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1+1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1+2)*255.0 + 0.5);
            *(aptr++)   = static_cast<unsigned char>(*(fptr2)*goArray[goLoop]*255.0 + 0.5);
            }
          fptr1+=3;
          fptr2++;
          }
        }

      for ( ; i < 256; i++ )
        {
        for ( goLoop = 0; goLoop < 256; goLoop++ )
          {
          *(rgbptr++) = 0;
          *(rgbptr++) = 0;
          *(rgbptr++) = 0;
          *(aptr++)   = 0;
          }
        }
      break;
      
    case 3:
    case 4:
      // Move the two temp float arrays into one alpha array
      aptr   = this->AlphaLookup;
      
      for ( goLoop = 0; goLoop < 256; goLoop++ )
        {
        fptr2 = this->TempArray2;
        for ( i = 0; i < arraySizeNeeded; i++ )
          {
          *(aptr++)   = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
          }
        for ( ; i < 256; i++ )
          {
          *(aptr++)   = 0;
          }
        }

      break;
    }
  return 1;
}


//-----------------------------------------------------------------------------
// Print the vtkMitkVolumeTextureMapper3D
void vtkMitkVolumeTextureMapper3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Sample Distance: " << this->SampleDistance << endl;
  os << indent << "NumberOfPolygons: " << this->NumberOfPolygons << endl;
  os << indent << "ActualSampleDistance: " 
     << this->ActualSampleDistance << endl;
  os << indent << "VolumeDimensions: " << this->VolumeDimensions[0] << " "
     << this->VolumeDimensions[1] << " " << this->VolumeDimensions[2] << endl;
  os << indent << "VolumeSpacing: " << this->VolumeSpacing[0] << " "
     << this->VolumeSpacing[1] << " " << this->VolumeSpacing[2] << endl;
  
  os << indent << "UseCompressedTexture: " << this->UseCompressedTexture
     << endl;
}



