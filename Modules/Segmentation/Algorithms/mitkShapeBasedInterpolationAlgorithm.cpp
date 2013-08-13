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

#include <itkBinaryThresholdImageFilter.h>
#include "ipSegmentation.h"

void mitk::ShapeBasedInterpolationAlgorithm::Interpolate(
                               Image::ConstPointer lowerSlice, unsigned int lowerSliceIndex,
                               Image::ConstPointer upperSlice, unsigned int upperSliceIndex,
                               unsigned int requestedIndex,
                               unsigned int /*sliceDimension*/, // commented variables are not used
                               Image* resultImage,
                               int activeLabel,
                               unsigned int /*timeStep*/,
                               Image::ConstPointer /*referenceImage*/)
{
  typedef itk::Image< ipMITKSegmentationTYPE, 2 > InputSliceType;

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

  // binarize the slice according to current label
  typedef itk::BinaryThresholdImageFilter< InputSliceType, InputSliceType > InputThresholdType;

  InputThresholdType::Pointer thresholder1 = InputThresholdType::New();
  thresholder1->SetInput(correctPixelTypeLowerITKSlice);
  thresholder1->SetUpperThreshold( activeLabel );
  thresholder1->SetLowerThreshold( activeLabel );
  thresholder1->SetInsideValue( 1 );
  thresholder1->SetOutsideValue( 0 );
  thresholder1->ReleaseDataFlagOn();
  thresholder1->Update();

  // back-convert to MITK images to access a mitkIpPicDescriptor
  Image::Pointer correctPixelTypeLowerMITKSlice = Image::New();
  CastToMitkImage( thresholder1->GetOutput(), correctPixelTypeLowerMITKSlice );
  mitkIpPicDescriptor* lowerPICSlice = mitkIpPicNew();
  CastToIpPicDescriptor( correctPixelTypeLowerMITKSlice, lowerPICSlice);

  InputThresholdType::Pointer thresholder2 = InputThresholdType::New();
  thresholder2->SetInput(correctPixelTypeUpperITKSlice);
  thresholder2->SetUpperThreshold( activeLabel );
  thresholder2->SetLowerThreshold( activeLabel );
  thresholder2->SetInsideValue( 1 );
  thresholder2->SetOutsideValue( 0 );
  thresholder2->ReleaseDataFlagOn();
  thresholder2->Update();

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
  assert ( itkAuxImage.IsNotNull() );

  InputSliceType::Pointer itkResultImage;
  CastToItkImage( resultImage, itkResultImage );
  assert ( itkResultImage.IsNotNull() );

  typedef itk::ImageRegionConstIterator< InputSliceType > SourceIteratorType;
  typedef itk::ImageRegionIterator< InputSliceType >      TargetIteratorType;

  SourceIteratorType sourceIterator( itkAuxImage, itkAuxImage->GetLargestPossibleRegion() );
  TargetIteratorType targetIterator( itkResultImage, itkResultImage->GetLargestPossibleRegion() );

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  const bool overwrite = true;

    while (!sourceIterator.IsAtEnd())
    {

      if (overwrite)
      {
         if ( sourceIterator.Get() )
            targetIterator.Set( activeLabel );
      }
      else
      {
         if ( sourceIterator.Get() && !targetIterator.Get() )
           targetIterator.Set( activeLabel );
      }

      ++sourceIterator;
      ++targetIterator;
    }

    resultImage->Modified();
}

