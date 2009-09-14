/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkVolumeVisualizationImagePreprocessor.h"

#include <itkAddConstantToImageFilter.h>



namespace mitk
{

VolumeVisualizationImagePreprocessor::VolumeVisualizationImagePreprocessor() 
: m_OutOfLiverValue(-512),
  m_surfaceValue(-256),
  m_realSurfaceValue(0),
  m_EstimatedThreshold( 150.0 ),
  m_MinThreshold( 0.0 ),
  m_MaxThreshold( 250.0 )
{
}

VolumeVisualizationImagePreprocessor::~VolumeVisualizationImagePreprocessor()
{
}


Image::Pointer
VolumeVisualizationImagePreprocessor::Process(
  Image::Pointer m_originalCT, Image::Pointer m_originalLiverMask)
{
  LOG_INFO << "Processing...";

  // converting mitk image -> itk image
  CTImage::Pointer CTImageWork = CTImage::New();
  CastToItkImage( m_originalCT, CTImageWork );

  // converting mitk image -> itk image
  BinImage::Pointer BinImageMask = BinImage::New();
  CastToItkImage( m_originalLiverMask, BinImageMask );

  BinImage::Pointer BinImageMaskDilate = Dilate(BinImageMask);

  BinImage::Pointer BinImageMaskErode = Erode(BinImageMask);

  // Image::Pointer imageBorder = ImportItkImage(CTImageWork);


  return Crop(ImportItkImage(Gaussian(Composite(CTImageWork,BinImageMask,BinImageMaskDilate,BinImageMaskErode))));
}


TransferFunction::Pointer
VolumeVisualizationImagePreprocessor::GetTransferFunction( double treshold )
{
  double opacity = 0.005;

  double maskValue = m_OutOfLiverValue;
  double surfaceValue = m_surfaceValue;
  double realSurfaceValue = m_realSurfaceValue;

  double surfaceSteepness = 0.0;

  LOG_INFO << "using threshold of " << treshold << " and opacity of " << opacity;

  TransferFunction::Pointer tf = TransferFunction::New();

  // grayvalue->opacity
  {   
    vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint(maskValue,0);
    f->AddPoint(maskValue+1,0);
    f->AddPoint(surfaceValue,0.05);
    f->AddPoint(realSurfaceValue,opacity);
    f->AddPoint(treshold-1,opacity); 
    f->AddPoint(treshold+4,0.8); 
    f->ClampingOn();
    f->Modified();
  }  

  // gradient at grayvalue->opacity
  {  
    vtkPiecewiseFunction *f=tf->GetGradientOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint( -1000.0, 1.0 );
    f->AddPoint( 1000, 1.0 );
    f->ClampingOn();
    f->Modified();
  }

  // grayvalue->color
  {  
    vtkColorTransferFunction *ctf=tf->GetColorTransferFunction();
    ctf->RemoveAllPoints();
    ctf->AddRGBPoint( maskValue, 0.5, 0.0, 0.0 );
    ctf->AddRGBPoint( maskValue+1, 0.5, 0.0, 0.0 );
    ctf->AddRGBPoint( surfaceValue, 1.0, 0.0, 0.0 ); //0.5
    ctf->AddRGBPoint( realSurfaceValue, 0.2, 0.0, 0.0 );

    ctf->AddRGBPoint( treshold-32, 0.2, 0.0, 0.0 );
    ctf->AddRGBPoint( treshold, 251/255.0, 1.0, 0.0 );
    ctf->ClampingOn();
    ctf->Modified();
  }

  return tf;
}


VolumeVisualizationImagePreprocessor::BinImage::Pointer 
VolumeVisualizationImagePreprocessor::Dilate(BinImage::Pointer src)
{
  LOG_INFO << "Dilating...";

  BinImage::Pointer dst = BinImage::New();

  typedef itk::BinaryDilateImageFilter< BinImage, BinImage,itk::BinaryBallStructuringElement< unsigned char, 3> > BinaryDilateImageType;
  BinaryDilateImageType::KernelType myKernel;
  myKernel.SetRadius(1);
  myKernel.CreateStructuringElement();
  BinaryDilateImageType::Pointer DilateFilter = BinaryDilateImageType::New();
  DilateFilter->SetInput(src);
  DilateFilter->SetKernel(myKernel);  
  DilateFilter->SetDilateValue(1); //to be dilated to 
  DilateFilter->Update();
  dst = DilateFilter->GetOutput(); 
  dst->DisconnectPipeline();
  
  return dst;
}

VolumeVisualizationImagePreprocessor::BinImage::Pointer 
VolumeVisualizationImagePreprocessor::Erode(BinImage::Pointer src)
{
  LOG_INFO << "Eroding...";

  BinImage::Pointer dst = BinImage::New();

  typedef itk::BinaryErodeImageFilter< BinImage, BinImage,itk::BinaryBallStructuringElement< unsigned char, 3> > BinaryErodeImageType;
  BinaryErodeImageType::KernelType myKernel;
  myKernel.SetRadius(1);
  myKernel.CreateStructuringElement();
  BinaryErodeImageType::Pointer ErodeFilter = BinaryErodeImageType::New();
  ErodeFilter->SetInput(src);
  ErodeFilter->SetKernel(myKernel);  
  ErodeFilter->SetErodeValue(0); //to be Eroded to 
  ErodeFilter->Update();
  dst = ErodeFilter->GetOutput(); 
  dst->DisconnectPipeline();
  
  return dst;
}


VolumeVisualizationImagePreprocessor::CTImage::Pointer 
VolumeVisualizationImagePreprocessor::Gaussian(CTImage::Pointer src)
{
  LOG_INFO << "Gaussian...";

  typedef itk::DiscreteGaussianImageFilter< CTImage, CTImage>					GaussianFilterType;

  GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
  gaussianFilter->SetInput( src );
  gaussianFilter->SetVariance( 2 );
  //   gaussianFilter->SetMaximumError( 0.1 );

  gaussianFilter->SetMaximumKernelWidth ( 32 ); 
  gaussianFilter->UpdateLargestPossibleRegion();
  return gaussianFilter->GetOutput();

}

Image::Pointer VolumeVisualizationImagePreprocessor::Crop(Image::Pointer src )
{
  LOG_INFO << "Cropping...";

  AutoCropImageFilter::Pointer cropFilter = AutoCropImageFilter::New();
  cropFilter->SetInput ( src );
  cropFilter->SetBackgroundValue ( m_OutOfLiverValue );
  cropFilter->SetMarginFactor ( 1.0 );
  cropFilter->UpdateLargestPossibleRegion();
  
  Image::Pointer dest = cropFilter->GetOutput();
  dest->DisconnectPipeline();
  return dest;
}

/*
  CTImage::Pointer CTImageWork = CTImage::New();
CastToItkImage( imageOrg, CTImageWork );

BinImage::Pointer BinImageMask = BinImage::New();
CastToItkImage( imageMask, BinImageMask );

BinImage::Pointer BinImageMaskDilate = BinImage::New();
BinImage::Pointer BinImageMaskErode = BinImage::New();
*/

mitk::VolumeVisualizationImagePreprocessor::CTImage::Pointer VolumeVisualizationImagePreprocessor::Composite( CTImage::Pointer input,
                       BinImage::Pointer mask,
                       BinImage::Pointer dilated,
                       BinImage::Pointer eroded)
{
  LOG_INFO << "Compositing...";
  itk::AddConstantToImageFilter<CTImage, short, CTImage>::Pointer nullFilter= itk::AddConstantToImageFilter<CTImage, short, CTImage>::New();
  nullFilter->SetInput( input );
  nullFilter->SetConstant( 0 );
  nullFilter->UpdateLargestPossibleRegion();
  CTImage::Pointer work = nullFilter->GetOutput();

  CTIteratorType workIt( work, work->GetRequestedRegion() );
  BinIteratorType maskIt( mask, mask->GetRequestedRegion() );
  BinIteratorType dilateIt( dilated, dilated->GetRequestedRegion() );
  BinIteratorType erodeIt( eroded, eroded->GetRequestedRegion() );
  
  workIt.GoToBegin();
  maskIt.GoToBegin();
  dilateIt.GoToBegin();
  erodeIt.GoToBegin();

  double sum=0;
  int num=0;

  int *histogramm= new int[65536];
  int total=0;

  int _min=32767,_max=-32768;

  memset(histogramm,0,sizeof(int)*65536);

  while ( ! ( workIt.IsAtEnd() || maskIt.IsAtEnd() || dilateIt.IsAtEnd() || erodeIt.IsAtEnd() ) )
  {
    int value = workIt.Get();
    unsigned char mask = maskIt.Get();
    unsigned char dilate = dilateIt.Get();
    unsigned char erode = erodeIt.Get();

    if(mask != 0)
    {
      histogramm[32768+(int)value]++;
      total++;
    }

    if(erode != 0 && mask != 0 )
    {
      sum+=value;
      num++;
      if(value>_max) _max=value;
      if(value<_min) _min=value;
    }
    
    
    if(erode == 0 && dilate != 0 )
    {
      value = -256;
    }
    else if( erode != 0 && mask != 0 )
    {
    
    }
    else
    {
      value = -512;
    }
  
    workIt.Set(value);
        
    ++workIt;
    ++maskIt;
    ++dilateIt;
    ++erodeIt;
  }

  LOG_INFO << "liver consists of " << total << " samples.";
  
  int oberen = total * 0.15;
  
  for( int r = 32767 ; r >= -32768 ; r-- )
  {
    int histogrammwert = histogramm[32768+(int)r];
    
    oberen -= histogrammwert;
    
    if(oberen <= 0)
    {
      m_EstimatedThreshold = r;
      LOG_INFO << "Histogramm found treshold " << r;
      break;
    }
    
  }
  
  m_MaxThreshold=_max;
  m_MinThreshold=_min;
  

  delete histogramm;

  if(num>0)
    m_realSurfaceValue=sum/num;
  else
    m_realSurfaceValue=0;
    
  LOG_INFO << "real surface value: " << m_realSurfaceValue;

  work->DisconnectPipeline();
  return work;
}



}
