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

#include "mitkITKDICOMSeriesReaderHelper.h"

#include <itkImageSeriesReader.h>
#include <itkResampleImageFilter.h>
//#include <itkAffineTransform.h>
//#include <itkLinearInterpolateImageFunction.h>
//#include <itkTimeProbesCollectorBase.h>


template <typename PixelType>
mitk::Image::Pointer
mitk::ITKDICOMSeriesReaderHelper
::LoadDICOMByITK(
    const StringContainer& filenames,
    bool correctTilt,
    const GantryTiltInformation& tiltInfo,
    itk::GDCMImageIO::Pointer& io)
{
  /******** Normal Case, 3D (also for GDCM < 2 usable) ***************/
  mitk::Image::Pointer image = mitk::Image::New();

  typedef itk::Image<PixelType, 3> ImageType;
  typedef itk::ImageSeriesReader<ImageType> ReaderType;

  io = itk::GDCMImageIO::New();
  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetImageIO(io);
  reader->ReverseOrderOff(); // at this point we require an order of input images so that
                             // the direction between the origin of the first and the last slice
                             // is the same direction as the image normals! Otherwise we might
                             // see images upside down. Unclear whether this is a bug in MITK,
                             // see NormalDirectionConsistencySorter.

  reader->SetFileNames(filenames);
  reader->Update();
  typename ImageType::Pointer readVolume = reader->GetOutput();

  // if we detected that the images are from a tilted gantry acquisition, we need to push some pixels into the right position
  if (correctTilt)
  {
    readVolume = FixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
  }

  image->InitializeByItk(readVolume.GetPointer());
  image->SetImportVolume(readVolume->GetBufferPointer());

  MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", "
                                      << image->GetDimension(1) << ", "
                                      << image->GetDimension(2) << "]";

  MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", "
                                    << image->GetGeometry()->GetSpacing()[1] << ", "
                                    << image->GetGeometry()->GetSpacing()[2] << "]";

  return image;
}

#define MITK_DEBUG_OUTPUT_FILELIST(list)\
  MITK_DEBUG << "-------------------------------------------"; \
  for (StringContainer::const_iterator _iter = (list).begin(); _iter!=(list).end(); ++_iter) \
    { \
      MITK_DEBUG <<" file '" << *_iter<< "'"; \
    } \
  MITK_DEBUG << "-------------------------------------------";

template <typename PixelType>
mitk::Image::Pointer
mitk::ITKDICOMSeriesReaderHelper
::LoadDICOMByITK3DnT(
    const StringContainerList& filenamesForTimeSteps,
    bool correctTilt,
    const GantryTiltInformation& tiltInfo,
    itk::GDCMImageIO::Pointer& io)
{
  unsigned int numberOfTimeSteps = filenamesForTimeSteps.size();

  mitk::Image::Pointer image = mitk::Image::New();

  typedef itk::Image<PixelType, 4> ImageType;
  typedef itk::ImageSeriesReader<ImageType> ReaderType;

  io = itk::GDCMImageIO::New();
  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetImageIO(io);
  reader->ReverseOrderOff(); // at this point we require an order of input images so that
                             // the direction between the origin of the first and the last slice
                             // is the same direction as the image normals! Otherwise we might
                             // see images upside down. Unclear whether this is a bug in MITK,
                             // see NormalDirectionConsistencySorter.


  unsigned int currentTimeStep = 0;
  MITK_DEBUG << "Start loading timestep " << currentTimeStep;
  MITK_DEBUG_OUTPUT_FILELIST( filenamesForTimeSteps.front() )
    reader->SetFileNames(filenamesForTimeSteps.front());
  reader->Update();
  typename ImageType::Pointer readVolume = reader->GetOutput();

  // if we detected that the images are from a tilted gantry acquisition, we need to push some pixels into the right position
  if (correctTilt)
  {
    readVolume = FixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
  }

  image->InitializeByItk(readVolume.GetPointer(), 1, numberOfTimeSteps);
  image->SetImportVolume(readVolume->GetBufferPointer(), currentTimeStep++); // timestep 0

  // for other time-steps
  for (StringContainerList::const_iterator timestepsIter = ++(filenamesForTimeSteps.begin()); // start with SECOND entry
      timestepsIter != filenamesForTimeSteps.end();
      ++currentTimeStep, ++timestepsIter)
  {
    MITK_DEBUG << "Start loading timestep " << currentTimeStep;
    MITK_DEBUG_OUTPUT_FILELIST( *timestepsIter )
      reader->SetFileNames(*timestepsIter);
    reader->Update();
    readVolume = reader->GetOutput();

    if (correctTilt)
    {
      readVolume = FixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
    }

    image->SetImportVolume(readVolume->GetBufferPointer(), currentTimeStep);
  }

  MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", "
                                      << image->GetDimension(1) << ", "
                                      << image->GetDimension(2) << "]";

  MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", "
                                    << image->GetGeometry()->GetSpacing()[1] << ", "
                                    << image->GetGeometry()->GetSpacing()[2] << "]";

  return image;
}


template <typename ImageType>
typename ImageType::Pointer
mitk::ITKDICOMSeriesReaderHelper
::FixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo )
{
  typedef itk::ResampleImageFilter<ImageType,ImageType> ResampleFilterType;
  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( input );

  /*
     Transform for a point is
      - transform from actual position to index coordinates
      - apply a shear that undoes the gantry tilt
      - transform back into world coordinates

     Anybody who does this in a simpler way: don't forget to write up how and why your solution works
  */
  typedef itk::ScalableAffineTransform< double, ImageType::ImageDimension > TransformType;
  typename TransformType::Pointer transformShear = TransformType::New();

  /**
    - apply a shear and spacing correction to the image block that corrects the ITK reader's error
      - ITK ignores the shear and loads slices into an orthogonal volume
      - ITK calculates the spacing from the origin distance, which is more than the actual spacing with gantry tilt images
    - to undo the effect
      - we have calculated some information in tiltInfo:
        - the shift in Y direction that is added with each additional slice is the most important information
        - the Y-shift is calculated in mm world coordinates
      - we apply a shearing transformation to the ITK-read image volume
        - to do this locally,
          - we transform the image volume back to origin and "normal" orientation by applying the inverse of its transform
            (this brings us into the image's "index coordinate" system)
          - we apply a shear with the Y-shift factor put into a unit transform at row 1, col 2
          - we transform the image volume back to its actual position (from index to world coordinates)
      - we lastly apply modify the image spacing in z direction by replacing this number with the correctly calulcated inter-slice distance
  */

  ScalarType factor = tiltInfo.GetMatrixCoefficientForCorrectionInWorldCoordinates() / input->GetSpacing()[1];
  // row 1, column 2 corrects shear in parallel to Y axis, proportional to distance in Z direction
  transformShear->Shear( 1, 2, factor );

  typename TransformType::Pointer imageIndexToWorld = TransformType::New();
  imageIndexToWorld->SetOffset( input->GetOrigin().GetVectorFromOrigin() );

  typename TransformType::MatrixType indexToWorldMatrix;
  indexToWorldMatrix = input->GetDirection();

  typename ImageType::DirectionType scale;
  for ( unsigned int i = 0; i < ImageType::ImageDimension; i++ )
  {
    scale[i][i] = input->GetSpacing()[i];
  }
  indexToWorldMatrix *= scale;

  imageIndexToWorld->SetMatrix( indexToWorldMatrix );

  typename TransformType::Pointer imageWorldToIndex = TransformType::New();
  imageIndexToWorld->GetInverse( imageWorldToIndex );

  typename TransformType::Pointer gantryTiltCorrection = TransformType::New();
  gantryTiltCorrection->Compose( imageWorldToIndex );
  gantryTiltCorrection->Compose( transformShear );
  gantryTiltCorrection->Compose( imageIndexToWorld );

  resampler->SetTransform( gantryTiltCorrection );

  typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
  resampler->SetInterpolator( interpolator );
  /*
     This would be the right place to invent a meaningful value for positions outside of the image.
     For CT, HU -1000 might be meaningful, but a general solution seems not possible. Even for CT,
     -1000 would only look natural for many not all images.
  */

  // TODO use (0028,0120) Pixel Padding Value if present
  resampler->SetDefaultPixelValue( itk::NumericTraits< typename ImageType::PixelType >::min() );

  // adjust size in Y direction! (maybe just transform the outer last pixel to see how much space we would need

  resampler->SetOutputParametersFromImage( input ); // we basically need the same image again, just sheared


  // if tilt positive, then we need additional pixels BELOW origin, otherwise we need pixels behind the end of the block

  // in any case we need more size to accomodate shifted slices
  typename ImageType::SizeType largerSize = resampler->GetSize(); // now the resampler already holds the input image's size.
  double imageSizeZ = largerSize[2];
  MITK_DEBUG <<"Calculate lager size = " << largerSize[1] << " + " << tiltInfo.GetTiltCorrectedAdditionalSize(imageSizeZ) << " / " << input->GetSpacing()[1] << "+ 2.0";
  largerSize[1] += static_cast<typename ImageType::SizeType::SizeValueType>(tiltInfo.GetTiltCorrectedAdditionalSize(imageSizeZ) / input->GetSpacing()[1]+ 2.0);
  resampler->SetSize( largerSize );
  MITK_DEBUG << "Fix Y size of image w/ spacing " << input->GetSpacing()[1] << " from " << input->GetLargestPossibleRegion().GetSize()[1] << " to " << largerSize[1];

  // in SOME cases this additional size is below/behind origin
  if ( tiltInfo.GetMatrixCoefficientForCorrectionInWorldCoordinates() > 0.0 )
  {
    typename ImageType::DirectionType imageDirection = input->GetDirection();
    Vector3D yDirection;
    yDirection[0] = imageDirection[0][1];
    yDirection[1] = imageDirection[1][1];
    yDirection[2] = imageDirection[2][1];
    yDirection.Normalize();

    typename ImageType::PointType shiftedOrigin;
    shiftedOrigin = input->GetOrigin();

    // add some pixels to make everything fit
    shiftedOrigin[0] -= yDirection[0] * (tiltInfo.GetTiltCorrectedAdditionalSize(imageSizeZ) + 1.0 * input->GetSpacing()[1]);
    shiftedOrigin[1] -= yDirection[1] * (tiltInfo.GetTiltCorrectedAdditionalSize(imageSizeZ) + 1.0 * input->GetSpacing()[1]);
    shiftedOrigin[2] -= yDirection[2] * (tiltInfo.GetTiltCorrectedAdditionalSize(imageSizeZ) + 1.0 * input->GetSpacing()[1]);

    resampler->SetOutputOrigin( shiftedOrigin );
  }

  resampler->Update();
  typename ImageType::Pointer result = resampler->GetOutput();

  // ImageSeriesReader calculates z spacing as the distance between the first two origins.
  // This is not correct in case of gantry tilt, so we set our calculated spacing.
  typename ImageType::SpacingType correctedSpacing = result->GetSpacing();
  correctedSpacing[2] = tiltInfo.GetRealZSpacing();
  result->SetSpacing( correctedSpacing );

  return result;
}


