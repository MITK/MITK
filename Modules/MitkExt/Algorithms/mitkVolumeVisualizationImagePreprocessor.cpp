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

#include <itkOrImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>

#include <mitkMemoryUtilities.h>

#define VVP_INFO LOG_INFO << "Mem Usage: " << mitk::MemoryUtilities::GetProcessMemoryUsage() << " "

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



TransferFunction::Pointer
VolumeVisualizationImagePreprocessor::GetInitialTransferFunction(  )
{
  int treshold = m_EstimatedThreshold;

  double opacity = 0.005;

  double maskValue = m_OutOfLiverValue;
  double surfaceValue = m_surfaceValue;
  double realSurfaceValue = m_realSurfaceValue;

  double surfaceSteepness = 0.0;

  VVP_INFO << "using threshold of " << treshold << " and opacity of " << opacity;

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
    f->AddPoint(m_MaxThreshold+1,0.8); 
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
    ctf->AddRGBPoint( m_MaxThreshold+1, 251/255.0, 1.0, 0.0 );

    ctf->ClampingOn();
    ctf->Modified();
  }

  m_LastUsedTreshold = treshold;

  return tf;
}


void VolumeVisualizationImagePreprocessor::UpdateTransferFunction( TransferFunction::Pointer tf, int treshold  )
{
  double opacity = 0.005;

  double maskValue = m_OutOfLiverValue;
  double surfaceValue = m_surfaceValue;
  double realSurfaceValue = m_realSurfaceValue;

  double surfaceSteepness = 0.0;

  //VVP_INFO << "changing to threshold of " << treshold << " and opacity of " << opacity;

  // grayvalue->opacity
  {   
    vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();
    
    f->RemovePoint( m_LastUsedTreshold-1 );
    f->AddPoint(treshold-1,opacity); 
   
    f->RemovePoint( m_LastUsedTreshold+4 );
    f->AddPoint(treshold+4,0.8); 
  }  

  // grayvalue->color
  {  
    vtkColorTransferFunction *ctf=tf->GetColorTransferFunction();

    ctf->RemovePoint( m_LastUsedTreshold-32 );
    ctf->AddRGBPoint( treshold-32, 0.2, 0.0, 0.0 );
    
    ctf->RemovePoint( m_LastUsedTreshold );
    ctf->AddRGBPoint( treshold, 251/255.0, 1.0, 0.0 );
  }

  m_LastUsedTreshold = treshold;
}

VolumeVisualizationImagePreprocessor::LabelImage::Pointer 
VolumeVisualizationImagePreprocessor::ConnectComponents(BinImage::Pointer src)
{
  VVP_INFO << "Connect Components...";

  LabelImage::Pointer dst = LabelImage::New();

  typedef itk::ConnectedComponentImageFilter< BinImage, LabelImage >  myFilterType;
  
  myFilterType::Pointer myFilter = myFilterType::New();
  
  myFilter->SetInput(src);
  myFilter->Update();
  dst = myFilter->GetOutput(); 
  dst->DisconnectPipeline();
  
  return dst;
}

VolumeVisualizationImagePreprocessor::BinImage::Pointer 
VolumeVisualizationImagePreprocessor::Threshold(CTImage::Pointer src, int threshold)
{
  VVP_INFO << "thresholding...";

  BinImage::Pointer dst = BinImage::New();

  typedef itk::ThresholdLabelerImageFilter< CTImage, BinImage >  myFilterType;
  
  myFilterType::Pointer myFilter = myFilterType::New();
  
  myFilter->SetInput(src);
  
  myFilterType::ThresholdVector tv;
  tv.push_back(threshold);
  myFilter->SetThresholds(tv);
  
  myFilter->Update();
  dst = myFilter->GetOutput(); 
  dst->DisconnectPipeline();
  
  return dst;
}

VolumeVisualizationImagePreprocessor::LabelImage::Pointer 
VolumeVisualizationImagePreprocessor::RelabelComponents(LabelImage::Pointer src)
{
  VVP_INFO << "Relabeling Components...";

  LabelImage::Pointer dst = LabelImage::New();

  typedef itk::RelabelComponentImageFilter< LabelImage, LabelImage >  myFilterType;
  
  myFilterType::Pointer myFilter = myFilterType::New();
  
  myFilter->SetInput(src);
  myFilter->Update();
  dst = myFilter->GetOutput(); 
  dst->DisconnectPipeline();
  
  return dst;
}

VolumeVisualizationImagePreprocessor::BinImage::Pointer 
VolumeVisualizationImagePreprocessor::Dilate(BinImage::Pointer src)
{
  VVP_INFO << "Dilating...";

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
  VVP_INFO << "Eroding...";

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
  VVP_INFO << "Gaussian...";

  typedef itk::DiscreteGaussianImageFilter< CTImage, CTImage>					GaussianFilterType;

  GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
  gaussianFilter->SetInput( src );
  gaussianFilter->SetVariance( 1 );
  //   gaussianFilter->SetMaximumError( 0.1 );

  gaussianFilter->SetMaximumKernelWidth ( 8 ); 
  gaussianFilter->UpdateLargestPossibleRegion();
  
  CTImage::Pointer dst = gaussianFilter->GetOutput(); 
  dst->DisconnectPipeline();

  return dst;

}

VolumeVisualizationImagePreprocessor::CTImage::Pointer VolumeVisualizationImagePreprocessor::Crop(VolumeVisualizationImagePreprocessor::CTImage::Pointer src )
{
  VVP_INFO << "Cropping 16bit...";

  typedef itk::RegionOfInterestImageFilter<CTImage,CTImage> FilterType;
  
  FilterType::Pointer cropFilter = FilterType::New();

  cropFilter->SetInput( src );
  
  CTImage::RegionType region;
  CTImage::SizeType        size;
  CTImage::IndexType    index;

  index.SetElement(0,m_MinX);
  index.SetElement(1,m_MinY);
  index.SetElement(2,m_MinZ);
     
  size.SetElement(0,m_MaxX-m_MinX+1);
  size.SetElement(1,m_MaxY-m_MinY+1);
  size.SetElement(2,m_MaxZ-m_MinZ+1);
  
  region.SetIndex(index);
  region.SetSize(size);
  
  cropFilter->SetRegionOfInterest(region);
    cropFilter->Update();

  CTImage::Pointer dst = cropFilter->GetOutput(); 
  dst->DisconnectPipeline();

  return dst;
}



VolumeVisualizationImagePreprocessor::BinImage::Pointer VolumeVisualizationImagePreprocessor::Crop(VolumeVisualizationImagePreprocessor::BinImage::Pointer src )
{
  VVP_INFO << "Cropping 8bit...";

  typedef itk::RegionOfInterestImageFilter<BinImage,BinImage> FilterType;
  
  FilterType::Pointer cropFilter = FilterType::New();

  cropFilter->SetInput( src );
  
  BinImage::RegionType region;
  BinImage::SizeType        size;
  BinImage::IndexType    index;

  index.SetElement(0,m_MinX);
  index.SetElement(1,m_MinY);
  index.SetElement(2,m_MinZ);
     
  size.SetElement(0,m_MaxX-m_MinX+1);
  size.SetElement(1,m_MaxY-m_MinY+1);
  size.SetElement(2,m_MaxZ-m_MinZ+1);
  
  region.SetIndex(index);
  region.SetSize(size);
  
  cropFilter->SetRegionOfInterest(region);
    cropFilter->Update();

  BinImage::Pointer dst = cropFilter->GetOutput(); 
  dst->DisconnectPipeline();

  return dst;
}


/*
  CTImage::Pointer CTImageWork = CTImage::New();
CastToItkImage( imageOrg, CTImageWork );

BinImage::Pointer BinImageMask = BinImage::New();
CastToItkImage( imageMask, BinImageMask );

BinImage::Pointer BinImageMaskDilate = BinImage::New();
BinImage::Pointer BinImageMaskErode = BinImage::New();
*/

int mitk::VolumeVisualizationImagePreprocessor::GetHistogrammValueFromBottom( double part )
{
  int unteren = total * part;
  for( int r = -32768 ; r <= 32767 ; r++ )
    if( (unteren -= histogramm[32768+(int)r]) <= 0 )
      return r;
}


int mitk::VolumeVisualizationImagePreprocessor::GetHistogrammValueFromTop( double part )
{
  int oberen = total * part;
  for( int r = 32767 ; r >= -32768 ; r-- )
    if( (oberen -= histogramm[32768+(int)r]) <= 0 )
      return r;
}


void VolumeVisualizationImagePreprocessor::DetermineBoundingBox( BinImage::Pointer mask )
{
  VVP_INFO << "determining Bounding Box...";
  
  BinIteratorIndexType maskIt( mask, mask->GetRequestedRegion() );

  maskIt.GoToBegin();

  int totalMinX;
  int totalMinY;
  int totalMinZ;

  int totalMaxX;
  int totalMaxY;
  int totalMaxZ;

  // Initialize Bounding Box
  {
    m_MinX=m_MinY=m_MinZ =  1000000;
    m_MaxX=m_MaxY=m_MaxZ = -1000000;    

    totalMinX=totalMinY=totalMinZ =  1000000;
    totalMaxX=totalMaxY=totalMaxZ = -1000000;    
  }

  while ( ! maskIt.IsAtEnd() )
  {
    BinIteratorIndexType::IndexType idx = maskIt.GetIndex();
    int x=idx.GetElement(0);
    int y=idx.GetElement(1);
    int z=idx.GetElement(2);  

    if(x<totalMinX) totalMinX=x;
    if(y<totalMinY) totalMinY=y;
    if(z<totalMinZ) totalMinZ=z;
    
    if(x>totalMaxX) totalMaxX=x;
    if(y>totalMaxY) totalMaxY=y;
    if(z>totalMaxZ) totalMaxZ=z;

    if(maskIt.Get())
    {
      if(x<m_MinX) m_MinX=x;
      if(y<m_MinY) m_MinY=y;
      if(z<m_MinZ) m_MinZ=z;
      
      if(x>m_MaxX) m_MaxX=x;
      if(y>m_MaxY) m_MaxY=y;
      if(z>m_MaxZ) m_MaxZ=z;
    }
    ++maskIt;
  } 
  
  int border = 3;
  
  m_MinX -= border; if(m_MinX < totalMinX ) m_MinX = totalMinX;
  m_MinY -= border; if(m_MinY < totalMinY ) m_MinY = totalMinY;
  m_MinZ -= border; if(m_MinZ < totalMinZ ) m_MinZ = totalMinZ;
  
  m_MaxX += border; if(m_MaxX > totalMaxX ) m_MaxX = totalMaxX;
  m_MaxY += border; if(m_MaxY > totalMaxY ) m_MaxY = totalMaxY;
  m_MaxZ += border; if(m_MaxZ > totalMaxZ ) m_MaxZ = totalMaxZ;
  
  VVP_INFO << "Bounding box" << " m_MinX: " << m_MinX << " m_MaxX: " << m_MaxX
                           << "\n m_MinY: " << m_MinY << " m_MaxY: " << m_MaxY
                           << "\n m_MinZ: " << m_MinZ << " m_MaxZ: " << m_MaxZ;


}

mitk::VolumeVisualizationImagePreprocessor::CTImage::Pointer VolumeVisualizationImagePreprocessor::Composite( CTImage::Pointer work,
                       BinImage::Pointer mask,
                       BinImage::Pointer dilated,
                       BinImage::Pointer eroded)
{
  VVP_INFO << "Compositing...";
  
  /*
  itk::OrImageFilter<CTImage, CTImage, CTImage>::Pointer nullFilter= itk::OrImageFilter<CTImage, CTImage, CTImage>::New();
  nullFilter->SetInput1( input );
  nullFilter->SetInput2( input );
  nullFilter->UpdateLargestPossibleRegion();
  CTImage::Pointer work = nullFilter->GetOutput();
  */

  CTIteratorIndexType workIt( work, work->GetRequestedRegion() );
  BinIteratorType maskIt( mask, mask->GetRequestedRegion() );
  BinIteratorType dilateIt( dilated, dilated->GetRequestedRegion() );
  BinIteratorType erodeIt( eroded, eroded->GetRequestedRegion() );
  
  workIt.GoToBegin();
  maskIt.GoToBegin();
  dilateIt.GoToBegin();
  erodeIt.GoToBegin();

  double sum=0;
  int num=0;
  
  double sumIn=0;
  int numIn=0;

  int _min=32767,_max=-32768;
  
  total=0;
  memset(histogramm,0,sizeof(int)*65536);
  
  while ( ! ( workIt.IsAtEnd() || maskIt.IsAtEnd() || dilateIt.IsAtEnd() || erodeIt.IsAtEnd() ) )
  {
    int value = workIt.Get();
    unsigned char mask = maskIt.Get();
    unsigned char dilate = dilateIt.Get();
    unsigned char erode = erodeIt.Get();
//baut Histogramm auf vom Leberinneren
    if(mask != 0)
    {
      sumIn+=value;
      numIn++;
      histogramm[32768+(int)value]++;
      total++;
    }
//Mittelwert der äußeren Schicht
    if(erode != 0 && mask != 0 )
    {
      sum+=value;
      num++;
      if(value>_max) _max=value;
      if(value<_min) _min=value;
    }
    
    //markiere Leberoberfläche mit -1024 und update bounding box
    if(erode == 0 && dilate != 0 )
    {
      value = -1024;
   
    }
    else if( erode != 0 && mask != 0 )//Leberinneres, behalte Grauwert bei
    {
    
    }
    else//markiere äußeres mit -2048
    {
      value = -2048;
    }
  
    workIt.Set(value);
        
    ++workIt;
    ++maskIt;
    ++dilateIt;
    ++erodeIt;
  }

  VVP_INFO << "liver consists of " << total << " samples.";

  m_GreatestStructureThreshold = GetHistogrammValueFromTop(0.20);
  m_EstimatedThreshold = GetHistogrammValueFromTop(0.10);
  m_MaxThreshold=GetHistogrammValueFromTop(0.001);
  m_MinThreshold=GetHistogrammValueFromBottom(0.20);
  
  VVP_INFO << "threshold range: (" << m_MinThreshold  << ";" << m_MaxThreshold << ") estimated vessel threshold: " << m_EstimatedThreshold ;
  VVP_INFO << "m_GreatestStructureThreshold: " << m_GreatestStructureThreshold;

//  BinImage::Pointer binImageThreshold= Threshold(work,m_GreatestStructureThreshold );
//  LabelImage::Pointer LabelImageunsorted=ConnectComponents(binImageThreshold);
//  LabelImage::Pointer LabelImageSorted= RelabelComponents(LabelImageunsorted);
  

  if(num>0)
    m_realSurfaceValue=sum/num;
  else
    m_realSurfaceValue=0;
    
  if(numIn>0)
    m_realInLiverValue=sumIn/numIn;
  else
    m_realInLiverValue=0;

  m_surfaceValue = _min - 40;
  m_OutOfLiverValue = m_surfaceValue - 40;
  
//  LabelIteratorType labelIt( LabelImageSorted, LabelImageSorted->GetRequestedRegion() );

  workIt.GoToBegin();
//  labelIt.GoToBegin();

  int numGesetzt=0;
  int numGelassen=0;
  
  
  while ( ! workIt.IsAtEnd() )
  {
    int value = workIt.Get();
//    int label = labelIt.Get();
    
    if(value == -1024 )
    {
      value = m_surfaceValue;
    }
    else if( value == -2048 )
    {
      value = m_OutOfLiverValue;
    }
    else
    {//innerhalb der Leber
    //Label ungleich 1 -->value auf min setzen
/*
      if (label != 1){
        numGesetzt++;
        value=m_realInLiverValue;
      }
      else
      {
        //value=m_EstimatedThreshold;
        numGelassen++;
      }
  */
    }
  
    workIt.Set(value);
        
    ++workIt;
//    ++labelIt;

  }

  //VVP_INFO << "gesetzt: " << numGesetzt << " --- gelassen: " << numGelassen;
      
  VVP_INFO << "OutOfLiver value: " << m_OutOfLiverValue;
  VVP_INFO << "surface value: " << m_surfaceValue;
  VVP_INFO << "real surface value: " << m_realSurfaceValue;
  VVP_INFO << "real inLiver value:" << m_realInLiverValue;

  work->DisconnectPipeline();
  
  return work;
}



Image::Pointer
VolumeVisualizationImagePreprocessor::Process(
  Image::Pointer m_originalCT, Image::Pointer m_originalLiverMask)
{
  VVP_INFO << "Processing...";

  // converting mitk image -> itk image
  CTImage::Pointer CTImageWork = CTImage::New();
  CastToItkImage( m_originalCT, CTImageWork );

  // converting mitk image -> itk image
  BinImage::Pointer BinImageMask = BinImage::New();
  CastToItkImage( m_originalLiverMask, BinImageMask );
  
  DetermineBoundingBox( BinImageMask );
  
  if( m_MaxX < m_MinX
   || m_MaxY < m_MinY
   || m_MaxZ < m_MinZ )
    return 0;
  
  CTImageWork = Gaussian(Crop( CTImageWork ));
  BinImageMask = Crop( BinImageMask );

  CTImage::Pointer itkResult =Composite(CTImageWork,BinImageMask,Dilate(BinImageMask),Erode(BinImageMask));

  mitk::Image::Pointer mitkResult= mitk::Image::New();
               mitk::CastToMitkImage( itkResult, mitkResult ); //TODO here we can perhaps save memory

  VVP_INFO << "Finished...";

  return mitkResult;
}

}
