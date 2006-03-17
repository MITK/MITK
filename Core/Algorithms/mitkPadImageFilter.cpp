/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPadImageFilter.h"
#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkConstantPadImageFilter.h"

mitk::PadImageFilter::PadImageFilter()
{
  this->SetNumberOfInputs(2);
  this->SetNumberOfRequiredInputs(2);
  m_BinaryFilter = false;
  m_PadConstante = -32766;
  m_LowerThreshold = -32766;
  m_UpperThreshold = -32765;
}

mitk::PadImageFilter::~PadImageFilter()
{
}

//void mitk::PadImageFilter::GenerateInputRequestedRegion()
//{
//}  
//
//void mitk::PadImageFilter::GenerateOutputInformation()
//{
//}

void mitk::PadImageFilter::GenerateData()
{
  mitk::Image::Pointer inputToPad = const_cast< mitk::Image * > ( this->GetInput(0) );
  mitk::Image::ConstPointer inputOrigImage = this->GetInput(1);
  mitk::Image::Pointer output = this->GetOutput();

  typedef itk::Image< short, 3 > ImageType;
  typedef itk::Image< unsigned char, 3 > OutputImageType;
  ImageType::Pointer itkToPadImage = ImageType::New();
  mitk::CastToItkImage( inputToPad, itkToPadImage );
  
  int origSize[3];
  for(int i=0; i<3; ++i)
    origSize[i] = inputOrigImage->GetDimension(i);

  int resultSize[3];
  for(int i=0; i<3; ++i)
    resultSize[i] = inputToPad->GetDimension(i);

  mitk::Point3D originPoint=inputToPad->GetGeometry()->GetOrigin();
  mitk::Vector3D spacing=inputToPad->GetGeometry()->GetSpacing();
  typedef itk::ConstantPadImageFilter< ImageType, ImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();

  unsigned long upperPad[3];
  unsigned long lowerPad[3];
   
  double origin[3];
  for(int i=0; i<3; ++i)
    origin[i] = 0;
  itkToPadImage->SetOrigin(origin);

  for(int i=0; i<3; ++i)
    lowerPad[i] = (int)(originPoint[i]/spacing[i]);

  for(int i=0; i<3; ++i)
    upperPad[i] = origSize[i]-((int)(originPoint[i]/spacing[i])+resultSize[i]);

  padFilter->SetInput(itkToPadImage);
  padFilter->SetConstant(m_PadConstante);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);

  if(m_BinaryFilter){
    typedef itk::BinaryThresholdImageFilter< ImageType, OutputImageType > BinaryFilterType;
    BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();

    binaryFilter->SetInput(padFilter->GetOutput());
    binaryFilter->SetLowerThreshold(m_LowerThreshold);
    binaryFilter->SetUpperThreshold(m_UpperThreshold);
    binaryFilter->SetInsideValue(0);
    binaryFilter->SetOutsideValue(1);
    binaryFilter->Update();

    mitk::CastToMitkImage(binaryFilter->GetOutput(), output);
  }
  else{
    padFilter->Update();
    mitk::CastToMitkImage(padFilter->GetOutput(), output);
  }
  output->SetRequestedRegionToLargestPossibleRegion();
}

// not useable because of variables to set
//template < typename TPixel, unsigned int VImageDimension >
//void mitk::PadIt(itk::Image< TPixel, VImageDimension >* padImage, mitk::Image::ConstPointer origImage)
//{  
//  int origSize[3];
//  for(int i=0; i<3; ++i)
//    origSize[i] = origImage->GetDimension(i);
//
//  typename itk::Image< TPixel, VImageDimension >::RegionType region;
//  typename itk::Size<VImageDimension> resultSize;
//  region = padImage->GetLargestPossibleRegion();
//  resultSize = region.GetSize();
//
//  const typename itk::Image< TPixel, VImageDimension >::SpacingType& spacing = padImage->GetSpacing();  
//  const typename itk::Image< TPixel, VImageDimension >::PointType& originPoint = padImage->GetOrigin();  
//  typedef itk::ConstantPadImageFilter< itk::Image < TPixel, VImageDimension > , itk::Image < TPixel, VImageDimension >  > PadFilterType;
//  typename PadFilterType::Pointer padFilter = PadFilterType::New();
//
//  unsigned long upperPad[3];
//  unsigned long lowerPad[3];
//   
//  double origin[3];
//  origin[0]=0;
//  origin[1]=0;
//  origin[2]=0;
//  padImage->SetOrigin(origin);
//
//  lowerPad[0]=(int)(originPoint[0]/spacing[0]);
//  lowerPad[1]=(int)(originPoint[1]/spacing[1]);
//  lowerPad[2]=(int)(originPoint[2]/spacing[2]);
//
//  upperPad[0]=origSize[0]-((int)(originPoint[0]/spacing[0])+resultSize[0]);
//  upperPad[1]=origSize[1]-((int)(originPoint[1]/spacing[1])+resultSize[1]);
//  upperPad[2]=origSize[2]-((int)(originPoint[2]/spacing[2])+resultSize[2]);
//
//  padFilter->SetInput(padImage);
//  padFilter->SetConstant(m_PadConstante);
//  padFilter->SetPadUpperBound(upperPad);
//  padFilter->SetPadLowerBound(lowerPad);
//  //padFilter->Update();
//
//  if(m_BinaryFilter){
//    typedef itk::BinaryThresholdImageFilter< itk::Image < TPixel, VImageDimension > , itk::Image < TPixel, VImageDimension >  > BinaryFilterType;
//    typename BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();
//
//    binaryFilter->SetInput(padFilter->GetOutput());
//    binaryFilter->SetLowerThreshold(m_LowerThreshold);
//    binaryFilter->SetUpperThreshold(m_UpperThreshold);
//    binaryFilter->SetInsideValue(0);
//    binaryFilter->SetOutsideValue(1);
//    binaryFilter->Update();
//
//    padImage = binaryFilter->GetOutput();
//  }
//  else{
//    padFilter->Update();
//    padImage = binaryFilter->GetOutput();
//  }
//}
