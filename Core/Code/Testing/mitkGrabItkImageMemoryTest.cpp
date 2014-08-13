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

#include <itkThresholdImageFilter.h>

#include "mitkImageAccessByItk.h"

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
//! [ItkThresholdFilter]
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
//! [ItkThresholdFilter]

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

//! [OutOfScopeCall]
  mitk::Image::Pointer output = mitk::Image::New();
  AccessByItk_2(input, ItkThresholdFilter, output, threshold );
//! [OutOfScopeCall]

  mitk::ImagePixelReadAccessor< TPixel, 3 > readAccessor( output );
  const TPixel* output_data = readAccessor.GetData();

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

int mitkGrabItkImageMemoryTest( int /*argc*/, char* /*argv*/[] )
{
  MITK_TEST_BEGIN("mitkGrabItkImageMemoryTest")

  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<short>();// "Import successful on 3D short");
  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<float>();// "Import succesfull on float");
  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<unsigned char>();// "Import succesfull on uchar");
  Assert_ItkImportWithinAccessByItkSucceded_ReturnsTrue<int>();// "Import succesfull on int");

  MITK_TEST_END()
}
