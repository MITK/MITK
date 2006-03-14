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
#include "itkBinaryThresholdImageFilter.h"
#include "itkConstantPadImageFilter.h"

mitk::PadImageFilter::PadImageFilter()
{
  this->SetNumberOfInputs(2);
  this->SetNumberOfRequiredInputs(2);
  //m_InputTimeSelector  = mitk::ImageTimeSelector::New();
  //m_OutputTimeSelector = mitk::ImageTimeSelector::New();
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
  //mitk::Image::Pointer output = const_cast< mitk::Image * > ( this->GetInput(0) );
  //mitk::Image::ConstPointer m_origImage = this->GetInput(1);

  ////AccessByItk_1(output.GetPointer(), PadImage, m_origImage);
  ////this->SetOutput(output);

  //this->SetOutput(TransformSegmentationCutResult(m_origImage, output));

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
  origin[0]=0;
  origin[1]=0;
  origin[2]=0;
  itkToPadImage->SetOrigin(origin);

  lowerPad[0]=(int)(originPoint[0]/spacing[0]);
  lowerPad[1]=(int)(originPoint[1]/spacing[1]);
  lowerPad[2]=(int)(originPoint[2]/spacing[2]);

  upperPad[0]=origSize[0]-((int)(originPoint[0]/spacing[0])+resultSize[0]);
  upperPad[1]=origSize[1]-((int)(originPoint[1]/spacing[1])+resultSize[1]);
  upperPad[2]=origSize[2]-((int)(originPoint[2]/spacing[2])+resultSize[2]);

  padFilter->SetInput(itkToPadImage);
  padFilter->SetConstant(-32766);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);
  //padFilter->Update();

  typedef itk::BinaryThresholdImageFilter< ImageType, OutputImageType > BinaryFilterType;
  BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();

  binaryFilter->SetInput(padFilter->GetOutput());
  binaryFilter->SetLowerThreshold(-32766);
  binaryFilter->SetUpperThreshold(-32765);
  binaryFilter->SetInsideValue(0);
  binaryFilter->SetOutsideValue(1);
  binaryFilter->Update();

  //mitk::Image::Pointer segmentationImage = mitk::Image::New();
  mitk::CastToMitkImage(binaryFilter->GetOutput(), output);
  output->SetRequestedRegionToLargestPossibleRegion();


}

//template < typename TPixel, unsigned int VImageDimension >
//void mitk::PadImage(itk::Image< TPixel, VImageDimension >* resultImage, mitk::Image::ConstPointer m_origImage)
//{
//  int origSize[3];
//  for(int i=0; i<3; ++i)
//    origSize[i] = m_origImage->GetDimension(i);
//
//  typename itk::Image< TPixel, VImageDimension >::RegionType region;
//  itk::Size<VImageDimension> resultSize;
//  region = resultImage->GetLargestPossibleRegion();
//  resultSize = region.GetSize();
//
//  const typename itk::Image< TPixel, VImageDimension >::SpacingType& spacing = resultImage->GetSpacing();  
//  const typename itk::Image< TPixel, VImageDimension >::PointType& originPoint = resultImage->GetOrigin();  
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
//  resultImage->SetOrigin(origin);
//
//  lowerPad[0]=(int)(originPoint[0]/spacing[0]);
//  lowerPad[1]=(int)(originPoint[1]/spacing[1]);
//  lowerPad[2]=(int)(originPoint[2]/spacing[2]);
//
//  upperPad[0]=origSize[0]-((int)(originPoint[0]/spacing[0])+resultSize[0]);
//  upperPad[1]=origSize[1]-((int)(originPoint[1]/spacing[1])+resultSize[1]);
//  upperPad[2]=origSize[2]-((int)(originPoint[2]/spacing[2])+resultSize[2]);
//
//  padFilter->SetInput(resultImage);
//  padFilter->SetConstant(-32766);
//  padFilter->SetPadUpperBound(upperPad);
//  padFilter->SetPadLowerBound(lowerPad);
//  
//  typedef itk::BinaryThresholdImageFilter< itk::Image < TPixel, VImageDimension > , itk::Image < TPixel, VImageDimension >  > BinaryFilterType;
//  typename BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();
//
//  binaryFilter->SetInput(padFilter->GetOutput());
//  binaryFilter->SetLowerThreshold(-32766);
//  binaryFilter->SetUpperThreshold(-32765);
//  binaryFilter->SetInsideValue(0);
//  binaryFilter->SetOutsideValue(1);
//  binaryFilter->Update();
//
//
//  resultImage = binaryFilter->GetOutput();  //???
//  resultImage->SetRequestedRegionToLargestPossibleRegion(); //???
//}

/*
mitk::Image::Pointer mitk::PadImageFilter::TransformSegmentationCutResult(mitk::Image::ConstPointer origImage, mitk::Image::Pointer resultImage)
{
  typedef itk::Image< short, 3 > ImageType;
  typedef itk::Image< unsigned char, 3 > OutputImageType;
  ImageType::Pointer itkTransformImage = ImageType::New();
  mitk::CastToItkImage( resultImage, itkTransformImage );
  
  int origSize[3];
  for(int i=0; i<3; ++i)
    origSize[i] = origImage->GetDimension(i);

  int resultSize[3];
  for(int i=0; i<3; ++i)
    resultSize[i] = resultImage->GetDimension(i);

  mitk::Point3D originPoint=resultImage->GetGeometry()->GetOrigin();
  mitk::Vector3D spacing=resultImage->GetGeometry()->GetSpacing();
  typedef itk::ConstantPadImageFilter< ImageType, ImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();

  unsigned long upperPad[3];
  unsigned long lowerPad[3];
   
  double origin[3];
  origin[0]=0;
  origin[1]=0;
  origin[2]=0;
  itkTransformImage->SetOrigin(origin);

  lowerPad[0]=(int)(originPoint[0]/spacing[0]);
  lowerPad[1]=(int)(originPoint[1]/spacing[1]);
  lowerPad[2]=(int)(originPoint[2]/spacing[2]);

  upperPad[0]=origSize[0]-((int)(originPoint[0]/spacing[0])+resultSize[0]);
  upperPad[1]=origSize[1]-((int)(originPoint[1]/spacing[1])+resultSize[1]);
  upperPad[2]=origSize[2]-((int)(originPoint[2]/spacing[2])+resultSize[2]);

  padFilter->SetInput(itkTransformImage);
  padFilter->SetConstant(-32766);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);

  typedef itk::BinaryThresholdImageFilter< ImageType, OutputImageType > BinaryFilterType;
  BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();

  binaryFilter->SetInput(padFilter->GetOutput());
  binaryFilter->SetLowerThreshold(-32766);
  binaryFilter->SetUpperThreshold(-32765);
  binaryFilter->SetInsideValue(0);
  binaryFilter->SetOutsideValue(1);
  binaryFilter->Update();

  mitk::Image::Pointer segmentationImage = mitk::Image::New();
  mitk::CastToMitkImage(binaryFilter->GetOutput(), segmentationImage);
  segmentationImage->SetRequestedRegionToLargestPossibleRegion();

  return segmentationImage;
}
*/
