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

#include "mitkShapeBasedInterpolationAlgorithm.h"
#include "mitkImageCast.h"
#include "mitkImageDataItem.h"
#include "mitkToolManagerProvider.h"
#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkBinaryThresholdImageFilter.h>
#include "ipSegmentation.h"

void mitk::ShapeBasedInterpolationAlgorithm::Interpolate(
                               Image::ConstPointer lowerSlice, unsigned int lowerSliceIndex,
                               Image::ConstPointer upperSlice, unsigned int upperSliceIndex,
                               unsigned int requestedIndex,
                               Image* resultImage)
{
  typedef itk::Image< ipMITKSegmentationTYPE, 2 > InputSliceType;

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(toolManager);
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(toolManager->GetWorkingData(0)->GetData());
  int activePixelValue = workingImage->GetActiveLabel()->GetIndex();

  // convert these slices to the ipSegmentation data type (into an ITK image)
  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeLowerITKSlice;
  CastToItkImage( lowerSlice, correctPixelTypeLowerITKSlice );
  assert ( correctPixelTypeLowerITKSlice.IsNotNull() );

  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeUpperITKSlice;
  CastToItkImage( upperSlice, correctPixelTypeUpperITKSlice );
  assert ( correctPixelTypeUpperITKSlice.IsNotNull() );

  // correct direction info
  itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
  imageDirection.SetIdentity();
  correctPixelTypeLowerITKSlice->SetDirection(imageDirection);
  correctPixelTypeUpperITKSlice->SetDirection(imageDirection);

  // binarize the slice according to the active label
  typedef itk::BinaryThresholdImageFilter< InputSliceType, InputSliceType > InputThresholdType;

  InputThresholdType::Pointer thresholder1 = InputThresholdType::New();
  thresholder1->SetInput(correctPixelTypeLowerITKSlice);
  thresholder1->SetUpperThreshold( activePixelValue );
  thresholder1->SetLowerThreshold( activePixelValue );
  thresholder1->SetInsideValue( 1 );
  thresholder1->SetOutsideValue( 0 );
  thresholder1->ReleaseDataFlagOn();

  try
  {
    thresholder1->Update();
  }
  catch(const std::exception &e)
  {
    MITK_ERROR << "Error in shape based interpolation: " << e.what();
    return;
  }

  // back-convert to MITK images to access a mitkIpPicDescriptor
  Image::Pointer correctPixelTypeLowerMITKSlice = Image::New();
  CastToMitkImage( thresholder1->GetOutput(), correctPixelTypeLowerMITKSlice );

  mitkIpPicDescriptor* lowerPICSlice = mitkIpPicNew();
  CastToIpPicDescriptor( correctPixelTypeLowerMITKSlice, lowerPICSlice);

  InputThresholdType::Pointer thresholder2 = InputThresholdType::New();
  thresholder2->SetInput(correctPixelTypeUpperITKSlice);
  thresholder2->SetUpperThreshold( activePixelValue );
  thresholder2->SetLowerThreshold( activePixelValue );
  thresholder2->SetInsideValue( 1 );
  thresholder2->SetOutsideValue( 0 );
  thresholder2->ReleaseDataFlagOn();

  try
  {
    thresholder2->Update();
  }
  catch(const std::exception &e)
  {
    MITK_ERROR << "Error in shape based interpolation: " << e.what();
    return;
  }

  Image::Pointer correctPixelTypeUpperMITKSlice = Image::New();
  CastToMitkImage( thresholder2->GetOutput(), correctPixelTypeUpperMITKSlice );

  mitkIpPicDescriptor* upperPICSlice = mitkIpPicNew();
  CastToIpPicDescriptor( correctPixelTypeUpperMITKSlice, upperPICSlice);

  // calculate where the current slice is in comparison to the lower and upper neighboring slices
  float ratio = (float)(requestedIndex - lowerSliceIndex) / (float)(upperSliceIndex - lowerSliceIndex);

  mitkIpPicDescriptor* ipPicResult = ipMITKSegmentationInterpolate( lowerPICSlice, upperPICSlice, ratio ); // magic
  if (!ipPicResult) return;

  mitk::Image::Pointer auxImage = mitk::Image::New();
  Geometry3D::Pointer originalGeometry = resultImage->GetGeometry();
  auxImage->Initialize( CastToImageDescriptor( ipPicResult ) );
  //resultImage->SetPicSlice( ipPicResult );
  auxImage->SetSlice( ipPicResult->data );
  auxImage->SetGeometry( originalGeometry );

  mitkIpPicFree( ipPicResult );

  // add the rest of data in the slice
  InputSliceType::Pointer itkAuxImage;
  CastToItkImage( auxImage, itkAuxImage );

  InputSliceType::Pointer itkResultImage;
  CastToItkImage( resultImage, itkResultImage );

  typedef itk::ImageRegionConstIterator< InputSliceType > SourceIteratorType;
  typedef itk::ImageRegionIterator< InputSliceType >      TargetIteratorType;

  SourceIteratorType sourceIterator( itkAuxImage, itkAuxImage->GetLargestPossibleRegion() );
  TargetIteratorType targetIterator( itkResultImage, itkResultImage->GetLargestPossibleRegion() );

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  while (!sourceIterator.IsAtEnd())
  {
    int targetValue = static_cast<int>(targetIterator.Get());
    if ( sourceIterator.Get() != 0 )
    {
      if (!workingImage->GetLabelLocked(targetValue))
        targetIterator.Set(1);
    }
    else
    {
      targetIterator.Set(0);
    }

    ++sourceIterator;
    ++targetIterator;
  }

  resultImage->Modified();
}

