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

#include "mitkTestingMacros.h"
#include "mitkITKImageImport.h"

#include "mitkImagePixelReadAccessor.h"

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkRandomImageSource.h>

#include <itkThresholdImageFilter.h>

#include "mitkImageAccessByItk.h"


/**
 * Create a test image with random pixel values. The image size is determined by the input parameter.
 *
 * @param size the number of voxels in each dimension
 */
template <typename TPixel, unsigned int VDimension>
typename itk::Image<TPixel, VDimension>::Pointer CreateTestImageRandom( short int size )
{
  typedef typename itk::Image< TPixel, VDimension> ImageType;
  typedef typename ImageType::Pointer ImagePointer;

  itk::Size<VDimension> regionSize;
  regionSize.Fill(size);

  typename itk::RandomImageSource<ImageType>::Pointer randomImageSource = itk::RandomImageSource<ImageType>::New();
  randomImageSource->SetNumberOfThreads(1); // to produce non-random results
  randomImageSource->SetSize(regionSize);
  randomImageSource->Update();

  return randomImageSource->GetOutput();
}

/**
 * Create a test image with a single pixel value. The image size is determined by the input parameter.
 *
 * @param value the pixel value the created image is filled with
 * @param size the number of voxels in each dimension
 */
template <typename TPixel, unsigned int VDimension>
typename itk::Image<TPixel, VDimension>::Pointer CreateTestImageFixedValue(size_t size, TPixel value)
{
  typedef typename itk::Image< TPixel, VDimension> ImageType;
  typedef typename ImageType::Pointer ImagePointer;

  typename ImageType::RegionType imageRegion;
  typename ImageType::RegionType::SizeType regionSize;
  regionSize.Fill(size);

  typename ImageType::RegionType::IndexType regionIndex;
  regionIndex.Fill(0);
  imageRegion.SetSize( regionSize );
  imageRegion.SetIndex( regionIndex );

  typename ImageType::SpacingType imageSpacing;
  imageSpacing.Fill(1.0f);

  typename ImageType::PointType imageOrigin;
  imageOrigin.Fill(0.0f);

  ImagePointer itkImage = ImageType::New();

  itkImage->SetRegions( imageRegion );
  itkImage->SetOrigin( imageOrigin );
  itkImage->SetSpacing( imageSpacing );
  itkImage->Allocate();

  itkImage->FillBuffer( value );

  return itkImage;
}

/**
 *  An ITK-based filter for thresholding.
 *
 *  The filter represents the typical usage of ITK-like filters inside MITK. It is to be called for an mitk::Image
 *  by using the AccessByItk macro. The filter executes the binary threshold filter and imports the result into the
 *  output by using the ImportItkImage method.
 *
 *  @param output mitk::Image to hold the result of the filter
 *  @param th[] two double values to set the lower/upper threshold
 */
template<typename TPixel, unsigned int VDimensions>
static void ItkThresholdFilter(
    const itk::Image<TPixel, VDimensions>* image,
    mitk::Image::Pointer& output,
    const double th[])
{
    typedef itk::Image<TPixel, VDimensions> InputImageType;
    typedef itk::Image<TPixel, VDimensions> OutputImageType;
    typedef itk::ThresholdImageFilter< InputImageType > ThresholdFilterType;

    typename ThresholdFilterType::Pointer thresholder =
      ThresholdFilterType::New();
    thresholder->SetInput(image);
    thresholder->ThresholdOutside(th[0], th[1]);
    thresholder->Update();

    try
    {
      output = mitk::GrabItkImageMemory(thresholder->GetOutput());
    }
    catch(itk::ExceptionObject&)
    {
      MITK_TEST_FAILED_MSG(<<"Thresholding computation failed");
    }
}

/**
 * Creates an mitk::Image, executes the binary threshold filter through AccessByItk and
 * checks whether the image data was correctly imported back to an mitk::Image.
 */
template< typename TPixel>
bool Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue()
{
  // data for 3x3x3 image
  const unsigned int dimensions[3] = {3,3,3};
  TPixel* image_data = new TPixel[27];

  // ground truth for result check
  TPixel* ground_truth = new TPixel[27];
  double threshold[2] = { 90.0, 180.0 };

  // fill image
  for( unsigned int i=0; i<27; i++)
  {
    image_data[i] = static_cast<TPixel>(i * 10);

    ground_truth[i] = 0;
    if( image_data[i] >= threshold[0] && image_data[i] <= threshold[1] )
      ground_truth[i] = static_cast<TPixel>(i * 10);

  }

  mitk::Image::Pointer input = mitk::Image::New();
  input->Initialize( mitk::MakeScalarPixelType<TPixel>(), 3, dimensions );
  input->SetImportVolume( image_data );

  mitk::Image::Pointer output = mitk::Image::New();
  //output->Initialize(input);
  AccessByItk_2(input, ItkThresholdFilter, output, threshold );

  mitk::ImagePixelReadAccessor< TPixel, 3 > readAccessor( output );
  const TPixel* output_data = readAccessor.GetConstData();

  bool equal = true;
  for( unsigned int i=0; i<27; i++)
  {

    equal &= (ground_truth[i] == output_data[i]);
    if(!equal)
    {
      MITK_INFO << "  :: At position " << i << " :  " <<ground_truth[i] << " ? " << output_data[i] << "\n";
      break;
    }
  }

  MITK_TEST_CONDITION(equal, " Imported output data equals the ground truth");

  return equal;

}


/**
 * Compares the meta information of both given images for equality.
 */
template <typename ImageType>
bool Assert_ImageMetaData_AreEqual( typename ImageType::Pointer itkImage, mitk::Image::Pointer mitkImage )
{
  bool return_value = true;

  typename ImageType::RegionType itkRegion = itkImage->GetLargestPossibleRegion();
  typename ImageType::SizeType itkImageSize = itkRegion.GetSize();

  // check dimension
  for( unsigned int idx=0; idx < mitkImage->GetDimension(); idx++)
  {
    return_value &= ( itkImageSize[idx] == mitkImage->GetDimension(idx) );
  }
  MITK_TEST_CONDITION( return_value, " - Dimensions equal!")

  // check pixel type
  bool ptype_compare = ( mitkImage->GetPixelType() == mitk::MakePixelType<ImageType>() );
  return_value &= ptype_compare;
  MITK_TEST_CONDITION( ptype_compare, " - Pixel types equal!")

  mitk::Geometry3D* imageGeometry = mitkImage->GetGeometry();
  const mitk::Point3D origin = imageGeometry->GetOrigin();

  bool origin_compare = true;
  for( unsigned int idx=0; idx < 3; idx++)
  {
      origin_compare &= ( itkImage->GetOrigin()[idx] == origin[idx] );
  }
  return_value &= origin_compare;
  MITK_TEST_CONDITION( origin_compare, " - Origin equals!")

  return return_value;
}

/**
 * Generates a random itk image and imports it to mitk image through ImportItkImage and compares the values
 * voxel-wise afterwards
 */
template <typename TPixel, unsigned int VDimension>
void Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue()
{
  std::stringstream msg;
  msg << "Current type: (Random Image, " << VDimension << "D):" << typeid(TPixel).name() << "\n";
  std::cout << msg.str();

  bool assert_value = true;

  typedef typename itk::Image< TPixel, VDimension> ImageType;
  typedef typename ImageType::Pointer ImagePointer;

  ImagePointer itkImage = CreateTestImageRandom<TPixel, VDimension>(5);

  mitk::Image::Pointer output_import = mitk::ImportItkImage( itkImage );

  itk::ImageRegionConstIteratorWithIndex< ImageType > iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  mitk::ImagePixelReadAccessor< TPixel, VDimension > readAccessor( output_import );

  bool difference = false;
  while( !iter.IsAtEnd() )
  {
    TPixel ref = iter.Get();
    TPixel val = readAccessor.GetPixelByIndex( iter.GetIndex() );

    difference |= ( ref != val );
    if( difference )
    {
      std::cout << iter.GetIndex() << ":" << ref << " ? " << val << "\n";
    }
    ++iter;
  }

  assert_value = Assert_ImageMetaData_AreEqual<ImageType>( itkImage, output_import );

  MITK_TEST_CONDITION( assert_value && (!difference), "Pixel values are same in voxel-wise comparison." );

}

/**
 * Generates an itk image with fixed pixel value and imports it to mitk image through ImportItkImage
 * and compares the values voxel-wise afterwards
 */
template <typename TPixel, unsigned int VDimension>
void Assert_ItkImageImportSucceded_ReturnsTrue()
{
  std::stringstream msg;
  msg << "Current type: " << VDimension << "D):" << typeid(TPixel).name() << "\n";
  std::cout << msg.str();

  bool assert_value = true;

  typedef typename itk::Image< TPixel, VDimension> ImageType;
  typedef typename ImageType::Pointer ImagePointer;

  ImagePointer itkImage = CreateTestImageFixedValue<TPixel, VDimension>(5, itk::NumericTraits<TPixel>::min());

  mitk::Image::Pointer output_import = mitk::ImportItkImage( itkImage );

  itk::ImageRegionConstIteratorWithIndex< ImageType > iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  mitk::ImagePixelReadAccessor< TPixel, VDimension > readAccessor( output_import );

  bool difference = false;
  while( !iter.IsAtEnd() )
  {
    TPixel ref = iter.Get();
    TPixel val = readAccessor.GetPixelByIndex( iter.GetIndex() );

    difference |= ( ref != val );
    if( difference )
    {
      std::cout << iter.GetIndex() << ":" << ref << " ? " << val << "\n";
    }

    ++iter;
  }

  assert_value = Assert_ImageMetaData_AreEqual<ImageType>( itkImage, output_import );

  MITK_TEST_CONDITION( assert_value && (!difference), "Pixel values are same in voxel-wise comparison." );

}

int mitkImportItkImageTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("ImportItkImageTest")

  Assert_ItkImageImportSucceded_ReturnsTrue<short, 3>();// "Import succesfull on 3D short");
  Assert_ItkImageImportSucceded_ReturnsTrue<float, 3>();// "Import succesfull on float");
  Assert_ItkImageImportSucceded_ReturnsTrue<unsigned char, 3>();// "Import succesfull on uchar");
  Assert_ItkImageImportSucceded_ReturnsTrue<int, 3>();// "Import succesfull on int");

  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<short, 3>();// "Import succesfull on 3D short");
  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<float, 3>();// "Import succesfull on float");
  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<unsigned char, 3>();// "Import succesfull on uchar");
  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<int, 3>();// "Import succesfull on int");

  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<short, 4>();// "Import succesfull on 3D short");
  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<float, 4>();// "Import succesfull on float");
  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<unsigned char, 4>();// "Import succesfull on uchar");
  Assert_ItkImageImportRandomValuesSucceded_ReturnsTrue<int, 4>();// "Import succesfull on int");

  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<short>();// "Import successful on 3D short");
  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<float>();// "Import succesfull on float");
  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<unsigned char>();// "Import succesfull on uchar");
  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<int>();// "Import succesfull on int");


  MITK_TEST_END()

}
