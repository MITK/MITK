/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkColourImageProcessor.h"
#include <mitkImage.h>
#include <itkImage.h>
#include <mitkImageCast.h>



namespace mitk
{

mitkColourImageProcessor::mitkColourImageProcessor()
{
}

mitkColourImageProcessor::~mitkColourImageProcessor()
{
}

template <class T> class ScalarToRGBAConverter
{
  const T *dataPtr;
  unsigned char *tmpPtr;
  int sizeX;
  int sizeY;
  int sizeZ;
  int sizeXY;
  int sizeXm1;
  int sizeYm1;
  int sizeZm1;
  mitk::TransferFunction::Pointer tf;
  
  public:

  ScalarToRGBAConverter( const T *_dataPtr,unsigned char *_tmpPtr,int _sizeX,int _sizeY,int _sizeZ,mitk::TransferFunction::Pointer _tf )
  {
    dataPtr=_dataPtr;
    tmpPtr=_tmpPtr;
    sizeX=_sizeX;
    sizeY=_sizeY;
    sizeZ=_sizeZ;

    sizeXY=sizeX*sizeY;
    sizeXm1=sizeX-1;
    sizeYm1=sizeY-1;
    sizeZm1=sizeZ-1;

    tf=_tf;
  }

  inline float sample(int x,int y,int z)
  {
    return float(dataPtr[ x + y * sizeX + z * sizeXY ]);
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

    float t = sqrtf( gx*gx + gy*gy + gz*gz );

    int doff = x + y * sizeX + z * sizeXY;
    
    vtkPiecewiseFunction* opacityTransferFunction = tf->GetScalarOpacityFunction();
    vtkPiecewiseFunction* gradientTransferFunction = tf->GetGradientOpacityFunction();
    vtkColorTransferFunction* colorTransferFunction = tf->GetColorTransferFunction();

    double rgb[3];
    colorTransferFunction->GetColor( double(grayValue), rgb);
    
    double opacity= opacityTransferFunction->GetValue( double(grayValue) );
    
    opacity *= gradientTransferFunction->GetValue( double(0.5f*t) );
    
    tmpPtr[doff*4+0] = int( rgb[0]*255 + 0.5 );
    tmpPtr[doff*4+1] = int( rgb[1]*255 + 0.5 );
    tmpPtr[doff*4+2] = int( rgb[2]*255 + 0.5 );
    tmpPtr[doff*4+3] = int( opacity*255 + 0.5 );
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
  }
  
  inline void fillSlices()
  {
    int z;

    for(z=0;z<sizeZ;z++)
    {
      if(z==0 || z==sizeZ-1)
        computeClamp2D(z);
       else
        compute2D(z);
    }
  }    
};



template<class TType> mitk::Image::Pointer mitkColourImageProcessor::ScalarToRGBA( itk::Image<TType, 3>* input  , mitk::TransferFunction::Pointer tf)
{ 
  
  const TType *inputData=input->GetBufferPointer();

  typename itk::Image<TType, 3>::SizeType ioSize = input->GetLargestPossibleRegion().GetSize();

  MITK_INFO << "size input image: " << ioSize[0] << ", " << ioSize[1] << ", " << ioSize[2];
  MITK_INFO << "size voxel: " << ioSize[0] * ioSize[1] * ioSize[2];
  
  int voxel = ioSize[0] * ioSize[1] * ioSize[2];
  
  unsigned char* RGBABuffer = new unsigned char[4*voxel];
  
  // Convert volume  
  {
    ScalarToRGBAConverter<TType> strc(inputData,RGBABuffer,ioSize[0],ioSize[1],ioSize[2],tf);
    strc.fillSlices();
  }
    
  // Create MITK Image out of the raw data
  {
    mitk::Image::Pointer image = mitk::Image::New();

    unsigned int dimensions[ 3 ];
    dimensions[ 0 ] = ioSize[0];
    dimensions[ 1 ] = ioSize[1];
    dimensions[ 2 ] = ioSize[2];

    mitk::PixelType pixelType( typeid(RGBAPixel), 1, itk::ImageIOBase::RGBA );
    image->Initialize( pixelType, 3, dimensions );
    image->SetImportChannel( RGBABuffer, 0, Image::ManageMemory );

    return image;
  }
}

 mitk::Image::Pointer mitkColourImageProcessor::convertToRGBAImage( mitk::Image::Pointer mitkInput , mitk::TransferFunction::Pointer tf )
{
  MITK_INFO << "convertToRGBAImage" ;
  
  mitk::Image::Pointer mitkResult= mitk::Image::New();
 
  if (*mitkInput->GetPixelType().GetTypeId() == typeid(unsigned char))
  {
    //cast to itkImage
    itk::Image< unsigned char, 3 >::Pointer itkInput;
    mitk::CastToItkImage(mitkInput,itkInput);
    mitkResult = ScalarToRGBA<unsigned char>(itkInput, tf);
  }
  else if (*mitkInput->GetPixelType().GetTypeId() == typeid(short))
  {
    //cast to itkImage
    itk::Image< short, 3 >::Pointer itkInput;
    mitk::CastToItkImage(mitkInput,itkInput);
    mitkResult = ScalarToRGBA<short>(itkInput, tf);
  }

   mitkResult->SetSpacing( mitkInput->GetGeometry()->GetSpacing() );
  
  return mitkResult;
}












template <class T,class B> 
class ScalarBinaryToRGBAConverter
{
  const T *dataPtr;
  const B *data2Ptr;
  unsigned char *tmpPtr;
  int sizeX;
  int sizeY;
  int sizeZ;
  int sizeXY;
  int sizeXm1;
  int sizeYm1;
  int sizeZm1;
  mitk::TransferFunction::Pointer tf;
  
  public:

  ScalarBinaryToRGBAConverter( const T *_dataPtr,const B *_data2Ptr,unsigned char *_tmpPtr,int _sizeX,int _sizeY,int _sizeZ,mitk::TransferFunction::Pointer _tf )
  {
    dataPtr=_dataPtr;
    data2Ptr=_data2Ptr;
    tmpPtr=_tmpPtr;
    sizeX=_sizeX;
    sizeY=_sizeY;
    sizeZ=_sizeZ;

    sizeXY=sizeX*sizeY;
    sizeXm1=sizeX-1;
    sizeYm1=sizeY-1;
    sizeZm1=sizeZ-1;

    tf=_tf;
  }

  inline float sample(int x,int y,int z)
  {
    return float(dataPtr[ x + y * sizeX + z * sizeXY ]);
  }
  
  inline bool sampleBinary(int x,int y,int z)
  {
    return data2Ptr[ x + y * sizeX + z * sizeXY ];
  }
  
  inline int clamp(int x)
  {
    if(x<0) x=0; else if(x>255) x=255;
    return x;
  }
  
  inline void write(int x,int y,int z,float grayValue,float gx,float gy,float gz)
  {
    if(sampleBinary(x,y,z))
    {
      
   /*       
      gx /= aspect[0];
      gy /= aspect[1];
      gz /= aspect[2];
   */
      // Compute the gradient magnitude

      float t = sqrtf( gx*gx + gy*gy + gz*gz );

      int doff = x + y * sizeX + z * sizeXY;
      
      vtkPiecewiseFunction* opacityTransferFunction = tf->GetScalarOpacityFunction();
      vtkPiecewiseFunction* gradientTransferFunction = tf->GetGradientOpacityFunction();
      vtkColorTransferFunction* colorTransferFunction = tf->GetColorTransferFunction();

      double rgb[3];
      colorTransferFunction->GetColor( double(grayValue), rgb);
      
      double opacity= opacityTransferFunction->GetValue( double(grayValue) );
      
      opacity *= gradientTransferFunction->GetValue( double(0.5f*t) );
      
      tmpPtr[doff*4+0] = int( rgb[0]*255 + 0.5 );
      tmpPtr[doff*4+1] = int( rgb[1]*255 + 0.5 );
      tmpPtr[doff*4+2] = int( rgb[2]*255 + 0.5 );
      tmpPtr[doff*4+3] = int( opacity*255 + 0.5 );
    }
    else
    {
      int doff = x + y * sizeX + z * sizeXY;
      tmpPtr[doff*4+0] = 0;
      tmpPtr[doff*4+1] = 0;
      tmpPtr[doff*4+2] = 0;
      tmpPtr[doff*4+3] = 0;
    }
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
  }
  
  inline void fillSlices()
  {
    int z;

    for(z=0;z<sizeZ;z++)
    {
      if(z==0 || z==sizeZ-1)
        computeClamp2D(z);
       else
        compute2D(z);
    }
  }    
};



template<class TType,class BType>   
mitk::Image::Pointer mitkColourImageProcessor::ScalarAndBinaryToRGBA(itk::Image<TType, 3>* input  ,itk::Image<BType, 3>* input2  , mitk::TransferFunction::Pointer tf)
{
  const TType *inputData=input->GetBufferPointer();
  const BType *input2Data=input2->GetBufferPointer();

  typename itk::Image<TType, 3>::SizeType ioSize = input->GetLargestPossibleRegion().GetSize();

  MITK_INFO << "size input image: " << ioSize[0] << ", " << ioSize[1] << ", " << ioSize[2];
  MITK_INFO << "size voxel: " << ioSize[0] * ioSize[1] * ioSize[2];
  
  int voxel= ioSize[0] * ioSize[1] * ioSize[2];
  
  unsigned char* RGBABuffer = new unsigned char[4*voxel];
  
  //for(int i=0;i<voxel;i++)

  // Convert volume  
  {
    ScalarBinaryToRGBAConverter<TType,BType> strc(inputData,input2Data,RGBABuffer,ioSize[0],ioSize[1],ioSize[2],tf);
    strc.fillSlices();
  }
    
  // Create MITK Image out of the raw data
  {
    mitk::Image::Pointer image = mitk::Image::New();

    unsigned int dimensions[ 3 ];
    dimensions[ 0 ] = ioSize[0];
    dimensions[ 1 ] = ioSize[1];
    dimensions[ 2 ] = ioSize[2];

    mitk::PixelType pixelType( typeid(RGBAPixel), 1, itk::ImageIOBase::RGBA );
    image->Initialize( pixelType, 3, dimensions );
    image->SetImportChannel( RGBABuffer, 0, Image::ManageMemory );

    return image;
  }
}

 mitk::Image::Pointer mitkColourImageProcessor::convertWithBinaryToRGBAImage( mitk::Image::Pointer input1 ,mitk::Image::Pointer input2  , mitk::TransferFunction::Pointer tf )
{
  MITK_INFO << "convertWithBinaryToRGBAImage" ;
    
  itk::Image< short, 3 >::Pointer inputCT;
  itk::Image< unsigned char, 3 >::Pointer inputBinary;
 
  if (*input1->GetPixelType().GetTypeId() == typeid(unsigned char) && *input2->GetPixelType().GetTypeId() == typeid(short))
  {
    mitk::CastToItkImage(input1,inputBinary);
    mitk::CastToItkImage(input2,inputCT);
  }
  else if (*input1->GetPixelType().GetTypeId() == typeid(short) && *input2->GetPixelType().GetTypeId() == typeid(unsigned char))
  {
    mitk::CastToItkImage(input1,inputCT);
    mitk::CastToItkImage(input2,inputBinary);
  }
  else
  {
    MITK_ERROR << "unsupported pixel type";
    return 0;
  }
  
  return ScalarAndBinaryToRGBA<short,unsigned char>(inputCT,inputBinary, tf);
}

//////////////////////////////////////////




template <class T,class B> 
class ScalarBinaryColorToRGBAConverter
{
  const T *dataPtr;
  const B *data2Ptr;
  unsigned char *tmpPtr;
  int sizeX;
  int sizeY;
  int sizeZ;
  int sizeXY;
  int sizeXm1;
  int sizeYm1;
  int sizeZm1;
  mitk::TransferFunction::Pointer tf;
  int *color;
  
  public:

  ScalarBinaryColorToRGBAConverter( const T *_dataPtr,const B *_data2Ptr,unsigned char *_tmpPtr,int _sizeX,int _sizeY,int _sizeZ,mitk::TransferFunction::Pointer _tf ,int *_color)
  {
    dataPtr=_dataPtr;
    data2Ptr=_data2Ptr;
    tmpPtr=_tmpPtr;
    sizeX=_sizeX;
    sizeY=_sizeY;
    sizeZ=_sizeZ;

    sizeXY=sizeX*sizeY;
    sizeXm1=sizeX-1;
    sizeYm1=sizeY-1;
    sizeZm1=sizeZ-1;

    tf=_tf;

    color = _color;
  }

  inline float sample(int x,int y,int z)
  {
    return float(dataPtr[ x + y * sizeX + z * sizeXY ]);
  }
  
  inline bool sampleBinary(int x,int y,int z)
  {
    return data2Ptr[ x + y * sizeX + z * sizeXY ];
  }
  
  inline int clamp(int x)
  {
    if(x<0) x=0; else if(x>255) x=255;
    return x;
  }
  
  inline void write(int x,int y,int z,float grayValue,float gx,float gy,float gz)
  {
    if(sampleBinary(x,y,z))
    {
      
   /*       
      gx /= aspect[0];
      gy /= aspect[1];
      gz /= aspect[2];
   */
      // Compute the gradient magnitude

      float t = sqrtf( gx*gx + gy*gy + gz*gz );

      int doff = x + y * sizeX + z * sizeXY;
      
      vtkPiecewiseFunction* opacityTransferFunction = tf->GetScalarOpacityFunction();
      vtkPiecewiseFunction* gradientTransferFunction = tf->GetGradientOpacityFunction();
      vtkColorTransferFunction* colorTransferFunction = tf->GetColorTransferFunction();

      double rgb[3];
      colorTransferFunction->GetColor( double(grayValue), rgb);
      
      double opacity= opacityTransferFunction->GetValue( double(grayValue) );
      
      opacity *= gradientTransferFunction->GetValue( double(0.5f*t) );
      
      tmpPtr[doff*4+0] = int( rgb[0]*255 + 0.5 );
      tmpPtr[doff*4+1] = int( rgb[1]*255 + 0.5 );
      tmpPtr[doff*4+2] = int( rgb[2]*255 + 0.5 );
      tmpPtr[doff*4+3] = int( opacity*255 + 0.5 );

      tmpPtr[doff*4+0] = color[0];
      tmpPtr[doff*4+1] = color[1];
      tmpPtr[doff*4+2] = color[2];
    }
    else
    {
      int doff = x + y * sizeX + z * sizeXY;
      tmpPtr[doff*4+0] = 0;
      tmpPtr[doff*4+1] = 0;
      tmpPtr[doff*4+2] = 0;
      tmpPtr[doff*4+3] = 0;
    }
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
  }
  
  inline void fillSlices()
  {
    int z;

    for(z=0;z<sizeZ;z++)
    {
      if(z==0 || z==sizeZ-1)
        computeClamp2D(z);
       else
        compute2D(z);
    }
  }    
};


template<class TType,class BType>   
mitk::Image::Pointer mitkColourImageProcessor::ScalarAndBinaryAndColorToRGBA(itk::Image<TType, 3>* input  ,itk::Image<BType, 3>* input2  , mitk::TransferFunction::Pointer tf, int * color)
{
  const TType *inputData=input->GetBufferPointer();
  const BType *input2Data=input2->GetBufferPointer();

  typename itk::Image<TType, 3>::SizeType ioSize = input->GetLargestPossibleRegion().GetSize();

  MITK_INFO << "size input image: " << ioSize[0] << ", " << ioSize[1] << ", " << ioSize[2];
  MITK_INFO << "size voxel: " << ioSize[0] * ioSize[1] * ioSize[2];
  
  int voxel= ioSize[0] * ioSize[1] * ioSize[2];
  
  unsigned char* RGBABuffer = new unsigned char[4*voxel];
  
  //for(int i=0;i<voxel;i++)

  // Convert volume  
  {
    ScalarBinaryColorToRGBAConverter<TType,BType> strc(inputData,input2Data,RGBABuffer,ioSize[0],ioSize[1],ioSize[2],tf,color);
    strc.fillSlices();
  }
    
  // Create MITK Image out of the raw data
  {
    mitk::Image::Pointer image = mitk::Image::New();

    unsigned int dimensions[ 3 ];
    dimensions[ 0 ] = ioSize[0];
    dimensions[ 1 ] = ioSize[1];
    dimensions[ 2 ] = ioSize[2];

    mitk::PixelType pixelType( typeid(RGBAPixel), 1, itk::ImageIOBase::RGBA );
    image->Initialize( pixelType, 3, dimensions );
    image->SetImportChannel( RGBABuffer, 0, Image::ManageMemory );

    return image;
  }
}

 mitk::Image::Pointer mitkColourImageProcessor::convertWithBinaryAndColorToRGBAImage( mitk::Image::Pointer input1 ,mitk::Image::Pointer input2  , mitk::TransferFunction::Pointer tf , int * color)
{
  MITK_INFO << "convertWithBinaryToRGBAImage" ;
    
  itk::Image< short, 3 >::Pointer inputCT;
  itk::Image< unsigned char, 3 >::Pointer inputBinary;
 
  if (*input1->GetPixelType().GetTypeId() == typeid(unsigned char) && *input2->GetPixelType().GetTypeId() == typeid(short))
  {
    mitk::CastToItkImage(input1,inputBinary);
    mitk::CastToItkImage(input2,inputCT);
  }
  else if (*input1->GetPixelType().GetTypeId() == typeid(short) && *input2->GetPixelType().GetTypeId() == typeid(unsigned char))
  {
    mitk::CastToItkImage(input1,inputCT);
    mitk::CastToItkImage(input2,inputBinary);
  }
  else
  {
    MITK_ERROR << "unsupported pixel type";
    return 0;
  }
  
  return ScalarAndBinaryAndColorToRGBA<short,unsigned char>(inputCT,inputBinary, tf,color);
}

static  inline int clamp(int x)
  {
    if(x<0) x=0; else if(x>255) x=255;
    return x;
  }
  

mitk::Image::Pointer mitkColourImageProcessor::CombineRGBAImage( unsigned char* input  ,unsigned char* input2, int sizeX,int sizeY,int sizeZ )
{ 
  int voxel= sizeX*sizeY*sizeZ;
  
  unsigned char* RGBABuffer = new unsigned char[4*voxel];
  
  // Convert volume  
  {
    for( int r=0;r<voxel;r++)
    {
      int rgbaInput1[4];
      int rgbaInput2[4];
      
      rgbaInput1[0]=input[r*4+0];
      rgbaInput1[1]=input[r*4+1];
      rgbaInput1[2]=input[r*4+2];
      rgbaInput1[3]=input[r*4+3];
      
      rgbaInput2[0]=input2[r*4+0];
      rgbaInput2[1]=input2[r*4+1];
      rgbaInput2[2]=input2[r*4+2];
      rgbaInput2[3]=input2[r*4+3];

      int result[4];

      /*
      
      float fac1 = rgbaInput1[3]/255.0f;
      float fac2 = rgbaInput2[3]/255.0f;

      float sum = fac1+fac2;
      
      float cor = 1.0f;
      
      if(sum > 1.0f)
        cor = 1.0f/sum;
        
      fac1 *= cor;
      fac2 *= cor;
      
      result[0]= clamp(int(fac1 * rgbaInput1[0] + fac2 * rgbaInput2[0] + 0.5f));
      result[1]= clamp(int(fac1 * rgbaInput1[1] + fac2 * rgbaInput2[1] + 0.5f));
      result[2]= clamp(int(fac1 * rgbaInput1[2] + fac2 * rgbaInput2[2] + 0.5f));
      result[3]= clamp(int(fac1 * rgbaInput1[3] + fac2 * rgbaInput2[3] + 0.5f));
      */
     
      if( rgbaInput1[3] )
      {
        result[0]= rgbaInput1[0];
        result[1]= rgbaInput1[1];
        result[2]= rgbaInput1[2];
        result[3]= rgbaInput1[3];      
      }
      else
      {
        result[0]= rgbaInput2[0];
        result[1]= rgbaInput2[1];
        result[2]= rgbaInput2[2];
        result[3]= rgbaInput2[3];
      }
      
      
      RGBABuffer[r*4+0]= result[0];
      RGBABuffer[r*4+1]= result[1];
      RGBABuffer[r*4+2]= result[2];
      RGBABuffer[r*4+3]= result[3];
    }
    
  }
  
  // Create MITK Image out of the raw data
  {
    mitk::Image::Pointer image = mitk::Image::New();

    unsigned int dimensions[ 3 ];
    dimensions[ 0 ] = sizeX;
    dimensions[ 1 ] = sizeY;
    dimensions[ 2 ] = sizeZ;

    mitk::PixelType pixelType( typeid(RGBAPixel), 1, itk::ImageIOBase::RGBA );
    image->Initialize( pixelType, 3, dimensions );
    image->SetImportChannel( RGBABuffer, 0, Image::ManageMemory );

    return image;
  }
}

mitk::Image::Pointer mitkColourImageProcessor::combineRGBAImage( mitk::Image::Pointer input1 , mitk::Image::Pointer input2)
{
  
  RGBAImage::Pointer itk1,itk2;

  unsigned char *data1=(unsigned char *)input1->GetData();
  unsigned char *data2=(unsigned char *)input2->GetData();

  unsigned int *dim = input1->GetDimensions();
   
  return CombineRGBAImage(data1,data2,dim[0],dim[1],dim[2]);
}


}//end namespace mitk


