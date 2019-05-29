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


#include "mitkCommandLineParser.h"

#include <mitkIOUtil.h>
#include <mitkRegistrationWrapper.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageTimeSelector.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkProperties.h>

// ITK
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include "itkLinearInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"
#include "itkResampleDwiImageFilter.h"

typedef itk::Image<double, 3> InputImageType;
typedef itk::Image<unsigned char, 3> BinaryImageType;

static mitk::Image::Pointer TransformToReference(mitk::Image *reference, mitk::Image *moving, bool sincInterpol = false, bool nn = false)
{
  // Convert to itk Images

  // Identify Transform
  typedef itk::IdentityTransform<double, 3> T_Transform;
  T_Transform::Pointer _pTransform = T_Transform::New();
  _pTransform->SetIdentity();

  typedef itk::WindowedSincInterpolateImageFunction< InputImageType, 3> WindowedSincInterpolatorType;
  WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

  typedef itk::LinearInterpolateImageFunction< InputImageType> LinearInterpolateImageFunctionType;
  LinearInterpolateImageFunctionType::Pointer lin_interpolator = LinearInterpolateImageFunctionType::New();

  typedef itk::NearestNeighborInterpolateImageFunction< BinaryImageType> NearestNeighborInterpolateImageFunctionType;
  NearestNeighborInterpolateImageFunctionType::Pointer nn_interpolator = NearestNeighborInterpolateImageFunctionType::New();


  if (!nn)
  {
    InputImageType::Pointer itkReference = InputImageType::New();
    InputImageType::Pointer itkMoving = InputImageType::New();
    mitk::CastToItkImage(reference,itkReference);
    mitk::CastToItkImage(moving,itkMoving);
    typedef itk::ResampleImageFilter<InputImageType, InputImageType>  ResampleFilterType;


    ResampleFilterType::Pointer resampler = ResampleFilterType::New();
    resampler->SetInput(itkMoving);
    resampler->SetReferenceImage( itkReference );
    resampler->UseReferenceImageOn();
    resampler->SetTransform(_pTransform);
    if ( sincInterpol)
      resampler->SetInterpolator(sinc_interpolator);
    else
      resampler->SetInterpolator(lin_interpolator);

    resampler->Update();

    // Convert back to mitk
    mitk::Image::Pointer result = mitk::Image::New();
    result->InitializeByItk(resampler->GetOutput());
    GrabItkImageMemory( resampler->GetOutput() , result );
    return result;
  }


  BinaryImageType::Pointer itkReference = BinaryImageType::New();
  BinaryImageType::Pointer itkMoving = BinaryImageType::New();
  mitk::CastToItkImage(reference,itkReference);
  mitk::CastToItkImage(moving,itkMoving);


  typedef itk::ResampleImageFilter<BinaryImageType, BinaryImageType>  ResampleFilterType;


  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(itkMoving);
  resampler->SetReferenceImage( itkReference );
  resampler->UseReferenceImageOn();
  resampler->SetTransform(_pTransform);
  resampler->SetInterpolator(nn_interpolator);

  resampler->Update();
  // Convert back to mitk
  mitk::Image::Pointer result = mitk::Image::New();
  result->InitializeByItk(resampler->GetOutput());
  GrabItkImageMemory( resampler->GetOutput() , result );
  return result;

}


static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim))
  {
    elems.push_back(item);
  }
  return elems;
}

static std::vector<std::string> split(const std::string &s, char delim)
{
  std::vector < std::string > elems;
  return split(s, delim, elems);
}


static mitk::Image::Pointer ResampleBySpacing(mitk::Image *input, float *spacing, bool useLinInt = true, bool useNN = false)
{
  if (!useNN)
  {
    InputImageType::Pointer itkImage = InputImageType::New();
    CastToItkImage(input,itkImage);

    /**
   * 1) Resampling
   *
   */
    // Identity transform.
    // We don't want any transform on our image except rescaling which is not
    // specified by a transform but by the input/output spacing as we will see
    // later.
    // So no transform will be specified.
    typedef itk::IdentityTransform<double, 3> T_Transform;

    // The resampler type itself.
    typedef itk::ResampleImageFilter<InputImageType, InputImageType>  T_ResampleFilter;

    // Prepare the resampler.
    // Instantiate the transform and specify it should be the id transform.
    T_Transform::Pointer _pTransform = T_Transform::New();
    _pTransform->SetIdentity();

    // Instantiate the resampler. Wire in the transform and the interpolator.
    T_ResampleFilter::Pointer _pResizeFilter = T_ResampleFilter::New();

    // Specify the input.
    _pResizeFilter->SetInput(itkImage);

    _pResizeFilter->SetTransform(_pTransform);

    // Set the output origin.
    _pResizeFilter->SetOutputOrigin(itkImage->GetOrigin());

    // Compute the size of the output.
    // The size (# of pixels) in the output is recomputed using
    // the ratio of the input and output sizes.
    InputImageType::SpacingType inputSpacing = itkImage->GetSpacing();
    InputImageType::SpacingType outputSpacing;
    const InputImageType::RegionType& inputSize = itkImage->GetLargestPossibleRegion();

    InputImageType::SizeType outputSize;
    typedef InputImageType::SizeType::SizeValueType SizeValueType;

    // Set the output spacing.
    outputSpacing[0] = spacing[0];
    outputSpacing[1] = spacing[1];
    outputSpacing[2] = spacing[2];

    outputSize[0] = static_cast<SizeValueType>(inputSize.GetSize()[0] * inputSpacing[0] / outputSpacing[0] + .5);
    outputSize[1] = static_cast<SizeValueType>(inputSize.GetSize()[1] * inputSpacing[1] / outputSpacing[1] + .5);
    outputSize[2] = static_cast<SizeValueType>(inputSize.GetSize()[2] * inputSpacing[2] / outputSpacing[2] + .5);

    _pResizeFilter->SetOutputSpacing(outputSpacing);
    _pResizeFilter->SetSize(outputSize);

    typedef itk::LinearInterpolateImageFunction< InputImageType > LinearInterpolatorType;
    LinearInterpolatorType::Pointer lin_interpolator = LinearInterpolatorType::New();


    typedef itk::WindowedSincInterpolateImageFunction< InputImageType, 4> WindowedSincInterpolatorType;
    WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

    if (useLinInt)
      _pResizeFilter->SetInterpolator(lin_interpolator);
    else
      _pResizeFilter->SetInterpolator(sinc_interpolator);

    _pResizeFilter->Update();

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk(_pResizeFilter->GetOutput());
    mitk::GrabItkImageMemory( _pResizeFilter->GetOutput(), image);
    return image;
  }

  BinaryImageType::Pointer itkImage = BinaryImageType::New();
  CastToItkImage(input,itkImage);

  /**
 * 1) Resampling
 *
 */
  // Identity transform.
  // We don't want any transform on our image except rescaling which is not
  // specified by a transform but by the input/output spacing as we will see
  // later.
  // So no transform will be specified.
  typedef itk::IdentityTransform<double, 3> T_Transform;

  // The resampler type itself.
  typedef itk::ResampleImageFilter<BinaryImageType, BinaryImageType>  T_ResampleFilter;

  // Prepare the resampler.
  // Instantiate the transform and specify it should be the id transform.
  T_Transform::Pointer _pTransform = T_Transform::New();
  _pTransform->SetIdentity();

  // Instantiate the resampler. Wire in the transform and the interpolator.
  T_ResampleFilter::Pointer _pResizeFilter = T_ResampleFilter::New();

  // Specify the input.
  _pResizeFilter->SetInput(itkImage);

  _pResizeFilter->SetTransform(_pTransform);

  // Set the output origin.
  _pResizeFilter->SetOutputOrigin(itkImage->GetOrigin());

  // Compute the size of the output.
  // The size (# of pixels) in the output is recomputed using
  // the ratio of the input and output sizes.
  BinaryImageType::SpacingType inputSpacing = itkImage->GetSpacing();
  BinaryImageType::SpacingType outputSpacing;
  const BinaryImageType::RegionType& inputSize = itkImage->GetLargestPossibleRegion();

  BinaryImageType::SizeType outputSize;
  typedef BinaryImageType::SizeType::SizeValueType SizeValueType;

  // Set the output spacing.
  outputSpacing[0] = spacing[0];
  outputSpacing[1] = spacing[1];
  outputSpacing[2] = spacing[2];

  outputSize[0] = static_cast<SizeValueType>(inputSize.GetSize()[0] * inputSpacing[0] / outputSpacing[0] + .5);
  outputSize[1] = static_cast<SizeValueType>(inputSize.GetSize()[1] * inputSpacing[1] / outputSpacing[1] + .5);
  outputSize[2] = static_cast<SizeValueType>(inputSize.GetSize()[2] * inputSpacing[2] / outputSpacing[2] + .5);

  _pResizeFilter->SetOutputSpacing(outputSpacing);
  _pResizeFilter->SetSize(outputSize);

  typedef itk::NearestNeighborInterpolateImageFunction< BinaryImageType> NearestNeighborInterpolateImageType;
  NearestNeighborInterpolateImageType::Pointer nn_interpolator = NearestNeighborInterpolateImageType::New();
    _pResizeFilter->SetInterpolator(nn_interpolator);

  _pResizeFilter->Update();

  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk(_pResizeFilter->GetOutput());
  mitk::GrabItkImageMemory( _pResizeFilter->GetOutput(), image);
  return image;
}

static mitk::Image::Pointer ResampleDWIbySpacing(mitk::Image::Pointer input, float* spacing)
{
  itk::Vector<double, 3> spacingVector;
  spacingVector[0] = spacing[0];
  spacingVector[1] = spacing[1];
  spacingVector[2] = spacing[2];

  typedef itk::ResampleDwiImageFilter<short> ResampleFilterType;

  mitk::DiffusionPropertyHelper::ImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::ImageType::New();
  mitk::CastToItkImage(input, itkVectorImagePointer);

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( itkVectorImagePointer );
  resampler->SetInterpolation(ResampleFilterType::Interpolate_Linear);
  resampler->SetNewSpacing(spacingVector);
  resampler->Update();

  mitk::Image::Pointer output = mitk::GrabItkImageMemory( resampler->GetOutput() );

  mitk::DiffusionPropertyHelper::SetGradientContainer(output, mitk::DiffusionPropertyHelper::GetGradientContainer(input));
  mitk::DiffusionPropertyHelper::SetReferenceBValue(output, mitk::DiffusionPropertyHelper::GetReferenceBValue(input));
  mitk::DiffusionPropertyHelper::InitializeImage( output );

  return output;
}

int main( int argc, char* argv[] )
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--","-");

  parser.setTitle("Image Resampler");
  parser.setCategory("Preprocessing Tools");
  parser.setContributor("MIC");
  parser.setDescription("Resample an image to eigther a specific spacing or to a reference image.");

  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Show this help text");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "Input file",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "Output file",us::Any(),false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("spacing", "s", mitkCommandLineParser::String, "Spacing:", "Resample provide x,y,z spacing in mm (e.g. -r 1,1,3), is not applied to tensor data",us::Any());
  parser.addArgument("reference", "r", mitkCommandLineParser::String, "Reference:", "Resample using supplied reference image. Also cuts image to same dimensions",us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("win-sinc", "w", mitkCommandLineParser::Bool, "Windowed-sinc interpolation:", "Use windowed-sinc interpolation (3) instead of linear interpolation ",us::Any());
  parser.addArgument("nearest-neigh", "n", mitkCommandLineParser::Bool, "Nearest Neighbor:", "Use Nearest Neighbor interpolation instead of linear interpolation ",us::Any());


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Handle special arguments
  bool useSpacing = false;
  bool useLinearInterpol = true;
  bool useNN= false;
  {
    if (parsedArgs.size() == 0)
    {
      return EXIT_FAILURE;
    }

    if (parsedArgs.count("sinc-int"))
      useLinearInterpol = false;

    if (parsedArgs.count("nearest-neigh"))
      useNN = true;

    // Show a help message
    if ( parsedArgs.count("help") || parsedArgs.count("h"))
    {
      std::cout << parser.helpText();
      return EXIT_SUCCESS;
    }
  }

  std::string outputFile = us::any_cast<std::string>(parsedArgs["i"]);
  std::string inputFile = us::any_cast<std::string>(parsedArgs["o"]);

  std::vector<std::string> spacings;
  float spacing[] = { 0.0f, 0.0f, 0.0f };
  if (parsedArgs.count("spacing"))
  {

    std::string arg =  us::any_cast<std::string>(parsedArgs["spacing"]);
    if (arg != "")
    {
      spacings = split(arg ,',');
      spacing[0] = atoi(spacings.at(0).c_str());
      spacing[1] = atoi(spacings.at(1).c_str());
      spacing[2] = atoi(spacings.at(2).c_str());
      useSpacing = true;
    }
  }

  std::string refImageFile = "";
  if (parsedArgs.count("reference"))
  {
    refImageFile =  us::any_cast<std::string>(parsedArgs["reference"]);
  }

  if (refImageFile =="" && useSpacing == false)
  {
    MITK_ERROR << "No information how to resample is supplied. Use eigther --spacing or --reference !";
    return EXIT_FAILURE;
  }


  mitk::Image::Pointer refImage;
  if (!useSpacing)
    refImage = mitk::IOUtil::Load<mitk::Image>(refImageFile);

  mitk::Image::Pointer inputDWI = mitk::IOUtil::Load<mitk::Image>(inputFile);
  if ( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(inputDWI.GetPointer()))
  {
    mitk::Image::Pointer outputImage;

    if (useSpacing)
      outputImage = ResampleDWIbySpacing(inputDWI, spacing);
    else
    {
      MITK_WARN << "Not supported yet, to resample a DWI please set a new spacing.";
      return EXIT_FAILURE;
    }

    mitk::IOUtil::Save(outputImage, outputFile.c_str());

    return EXIT_SUCCESS;
  }
  mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>(inputFile);


  mitk::Image::Pointer resultImage;

  if (useSpacing)
    resultImage = ResampleBySpacing(inputImage,spacing,useLinearInterpol,useNN);
  else
    resultImage = TransformToReference(refImage,inputImage,useLinearInterpol,useNN);


  mitk::IOUtil::Save(resultImage, outputFile);

  return EXIT_SUCCESS;
}
