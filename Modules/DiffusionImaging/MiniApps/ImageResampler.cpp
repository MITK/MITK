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
#include <mitkDiffusionImage.h>
#include <mitkImageTimeSelector.h>

// ITK
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include "itkLinearInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"
#include "itkResampleDwiImageFilter.h"

typedef itk::Image<double, 3> InputImageType;
typedef mitk::DiffusionImage<short> DiffusionImageType;


static mitk::Image::Pointer TransformToReference(mitk::Image *reference, mitk::Image *moving, bool sincInterpol = false)
{
  // Convert to itk Images
  InputImageType::Pointer itkReference = InputImageType::New();
  InputImageType::Pointer itkMoving = InputImageType::New();
  mitk::CastToItkImage(reference,itkReference);
  mitk::CastToItkImage(moving,itkMoving);

  // Identify Transform
  typedef itk::IdentityTransform<double, 3> T_Transform;
  T_Transform::Pointer _pTransform = T_Transform::New();
  _pTransform->SetIdentity();

  typedef itk::WindowedSincInterpolateImageFunction< InputImageType, 3> WindowedSincInterpolatorType;
  WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

  typedef itk::ResampleImageFilter<InputImageType, InputImageType>  ResampleFilterType;


  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(itkMoving);
  resampler->SetReferenceImage( itkReference );
  resampler->UseReferenceImageOn();
  resampler->SetTransform(_pTransform);
  resampler->SetInterpolator(sinc_interpolator);
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


static mitk::Image::Pointer ResampleBySpacing(mitk::Image *input, float *spacing, bool useLinInt = true)
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

/// Save images according to file type
static void SaveImage(std::string fileName, mitk::Image* image, std::string fileType )
{
  std::cout << "----Save to " << fileName;

  if (fileType == "dwi") // IOUtil does not handle dwi files properly Bug 15772
  {
    try
    {
      mitk::IOUtil::Save(dynamic_cast<mitk::DiffusionImage<short>*>(image), fileName.c_str());
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Caught exception: " << e.what();
      mitkThrow() << "Failed with exception from subprocess!";
    }
  }
  else
  {
    mitk::IOUtil::SaveImage(image, fileName);
  }
}

DiffusionImageType::Pointer ResampleDWIbySpacing(DiffusionImageType::Pointer input, float* spacing, bool useLinInt = true)
{

  itk::Vector<double, 3> spacingVector;
  spacingVector[0] = spacing[0];
  spacingVector[1] = spacing[1];
  spacingVector[2] = spacing[2];

  typedef itk::ResampleDwiImageFilter<short> ResampleFilterType;

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(input->GetVectorImage());
  resampler->SetInterpolation(ResampleFilterType::Interpolate_Linear);
  resampler->SetNewSpacing(spacingVector);
  resampler->Update();

  DiffusionImageType::Pointer output = DiffusionImageType::New();
  output->SetVectorImage(resampler->GetOutput());
  output->SetDirections(input->GetDirections());
  output->SetReferenceBValue(input->GetReferenceBValue());
  output->InitializeFromVectorImage();

  return output;
}

int main( int argc, char* argv[] )
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--","-");

  parser.setTitle("Image Resampler");
  parser.setCategory("Preprocessing Tools");
  parser.setContributor("MBI");
  parser.setDescription("Resample an image to eigther a specific spacing or to a reference image.");

  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::InputImage, "Input:", "Input file",us::Any(),false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputDirectory, "Output:", "Output folder (ending with /)",us::Any(),false);
  parser.addArgument("spacing", "s", mitkCommandLineParser::String, "Spacing:", "Resample provide x,y,z spacing in mm (e.g. -r 1,1,3), is not applied to tensor data",us::Any());
  parser.addArgument("reference", "r", mitkCommandLineParser::String, "Reference:", "Resample using supplied reference image. Also cuts image to same dimensions",us::Any());
  parser.addArgument("win-sinc", "w", mitkCommandLineParser::Bool, "Windowed-sinc interpolation:", "Use windowed-sinc interpolation (3) instead of linear interpolation ",us::Any());


  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Handle special arguments
  bool useSpacing = false;
  bool useLinearInterpol = true;
  {
    if (parsedArgs.size() == 0)
    {
      return EXIT_FAILURE;
    }

    if (parsedArgs.count("sinc-int"))
      useLinearInterpol = false;

    // Show a help message
    if ( parsedArgs.count("help") || parsedArgs.count("h"))
    {
      std::cout << parser.helpText();
      return EXIT_SUCCESS;
    }
   }

  std::string outputPath = us::any_cast<string>(parsedArgs["output"]);
  std::string inputFile = us::any_cast<string>(parsedArgs["input"]);

  std::vector<std::string> spacings;
  float spacing[3];
  if (parsedArgs.count("spacing"))
  {
    std::string arg =  us::any_cast<string>(parsedArgs["spacing"]);
    spacings = split(arg ,',');
    spacing[0] = atoi(spacings.at(0).c_str());
    spacing[1] = atoi(spacings.at(1).c_str());
    spacing[2] = atoi(spacings.at(2).c_str());
    useSpacing = true;
  }

  std::string refImageFile = "";
  if (parsedArgs.count("reference"))
  {
    refImageFile =  us::any_cast<string>(parsedArgs["reference"]);
  }

  if (refImageFile =="" && useSpacing == false)
  {
    MITK_ERROR << "No information how to resample is supplied. Use eigther --spacing or --reference !";
    return EXIT_FAILURE;
  }


  mitk::Image::Pointer refImage;
  if (!useSpacing)
      refImage = mitk::IOUtil::LoadImage(refImageFile);

  DiffusionImageType::Pointer inputDWI = dynamic_cast<DiffusionImageType*>(mitk::IOUtil::LoadBaseData(inputFile).GetPointer());
  if (inputDWI.IsNotNull())
  {
    DiffusionImageType::Pointer outputImage;

    if (useSpacing)
      outputImage = ResampleDWIbySpacing(inputDWI, spacing);
    else
    {
      MITK_WARN << "Not supported yet, to resample a DWI please set a new spacing.";
      return EXIT_FAILURE;
    }

    std::string fileStem = itksys::SystemTools::GetFilenameWithoutExtension(inputFile);

    std::string outName(outputPath + fileStem + "_res.dwi");
    mitk::IOUtil::Save(outputImage, outName.c_str());

    return EXIT_SUCCESS;
  }
  mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputFile);


  mitk::Image::Pointer resultImage;

  if (useSpacing)
    resultImage = ResampleBySpacing(inputImage,spacing);
  else
    resultImage = TransformToReference(refImage,inputImage);

  std::string fileStem = itksys::SystemTools::GetFilenameWithoutExtension(inputFile);

  mitk::IOUtil::SaveImage(resultImage, outputPath + fileStem + "_res.nrrd");

  return EXIT_SUCCESS;
}
