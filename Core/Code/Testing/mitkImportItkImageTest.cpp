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
#include "mitkImageCast.h"

#include "mitkImagePixelReadAccessor.h"

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkRandomImageSource.h>


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
 * Create a test vector image (with two components) with a single pixel value. The image size is determined by the input parameter.
 *
 * @param value the pixel value the created image is filled with
 * @param size the number of voxels in each dimension
 */
template <typename TPixel, unsigned int VDimension>
typename itk::VectorImage<TPixel, VDimension>::Pointer CreateTestVectorImageFixedValue(size_t size, const itk::VariableLengthVector<TPixel>& value)
{
  typedef typename itk::VectorImage< TPixel, VDimension> ImageType;
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

  itkImage->SetVectorLength(value.GetNumberOfElements());
  itkImage->SetRegions( imageRegion );
  itkImage->SetOrigin( imageOrigin );
  itkImage->SetSpacing( imageSpacing );
  itkImage->Allocate();

  itkImage->FillBuffer( value );

  return itkImage;
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

  mitk::BaseGeometry* imageGeometry = mitkImage->GetGeometry();
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

void Assert_ItkVectorImageImportAndCast_ReturnsTrue()
{
  typedef itk::VectorImage<short,3> ImageType;
  ImageType::PixelType value;
  value.SetSize(2);
  value.SetElement(0, 1);
  value.SetElement(0, 2);
  ImageType::Pointer itkImage = CreateTestVectorImageFixedValue<short, 3>(5, value);

  mitk::Image::Pointer mitkImage = mitk::ImportItkImage(itkImage);
  mitk::PixelType pixelType = mitkImage->GetPixelType();
  MITK_TEST_CONDITION(pixelType.GetPixelType() == itk::ImageIOBase::VECTOR, "Vector image pixel type")
  MITK_TEST_CONDITION(pixelType.GetComponentType() == itk::ImageIOBase::SHORT, "Vector image component type")

  mitk::Image::Pointer mitkImage2;
  mitk::CastToMitkImage(itkImage, mitkImage2);
  mitk::PixelType pixelType2 = mitkImage2->GetPixelType();
  MITK_TEST_CONDITION(pixelType == pixelType2, "ImportItkImage and CastToMitkImage produce same pixel types")

  ImageType::Pointer itkImageOut;
  mitk::CastToItkImage(mitkImage, itkImageOut);

  MITK_TEST_CONDITION(pixelType == mitk::MakePixelType<ImageType>(2), "MITK pixel type equals ITK pixel type")

  typedef itk::VectorImage<int,3> IntImageType;
  IntImageType::Pointer itkIntImageOut;
  mitk::CastToItkImage(mitkImage, itkIntImageOut);
  MITK_TEST_CONDITION(!(pixelType == mitk::MakePixelType<IntImageType>(2)), "MITK pixel type != ITK pixel type")

  mitk::Image::Pointer mitkImage3;
  mitk::CastToMitkImage(itkImageOut, mitkImage3);
  MITK_ASSERT_EQUAL(mitkImage, mitkImage3, "Equality for vector images");
}

int mitkImportItkImageTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("mitkImportItkImageTest")

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

  Assert_ItkVectorImageImportAndCast_ReturnsTrue();

  MITK_TEST_END()
}
