/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommandLineParser.h>
#include <mitkExceptionMacro.h>
#include <mitkIOUtil.h>
#include <mitkImageCast.h>
#include <itkN4BiasFieldCorrectionImageFilter.h>

#include <algorithm>

namespace
{
  /** All N4 counts end up in unsigned ITK fields, where a negative value would
      wrap into an enormous bin or iteration count instead of being rejected. */
  unsigned int GetPositiveInt(const std::map<std::string, us::Any>& parsedArgs, const std::string& name)
  {
    const int value = us::any_cast<int>(parsedArgs.at(name));

    if (value < 1)
      mitkThrow() << "Argument --" << name << " must be greater than 0 but is " << value << '.';

    return static_cast<unsigned int>(value);
  }
}

int main(int argc, char* argv[])
{
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef itk::Image<float, 3> ImageType;
  typedef itk::N4BiasFieldCorrectionImageFilter < ImageType, MaskImageType, ImageType > FilterType;

  mitkCommandLineParser parser;
  parser.setTitle("N4 Bias Field Correction");
  parser.setCategory("Classification Tools");
  parser.setDescription("Corrects the low-frequency intensity inhomogeneity (bias field) of an MR image with the N4 algorithm. The mask marks the voxels used for estimating the bias field.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::File, "Mask file:", "Mask image; all voxels other than 0 are used for the bias field estimation", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Corrected output image", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("number-of-controllpoints", "noc", mitkCommandLineParser::Int, "Parameter", "The noc for the point grid size defining the B-spline estimate (default 4)", us::Any(), true);
  parser.addArgument("number-of-fitting-levels", "nofl", mitkCommandLineParser::Int, "Parameter", "Number of fitting levels for the multi-scale approach (default 1)", us::Any(), true);
  parser.addArgument("number-of-histogram-bins", "nohb", mitkCommandLineParser::Int, "Parameter", "number of bins defining the log input intensity histogram (default 200)", us::Any(), true);
  parser.addArgument("spline-order", "so", mitkCommandLineParser::Int, "Parameter", "Define the spline order (default 3)", us::Any(), true);
  parser.addArgument("winer-filter-noise", "wfn", mitkCommandLineParser::Float, "Parameter", "Noise estimate defining the Wiener filter (default 0.01)", us::Any(), true);
  parser.addArgument("number-of-maximum-iterations", "nomi", mitkCommandLineParser::Int, "Parameter", "Spezifies the maximum number of iterations per run", us::Any(), true);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
  {
    return EXIT_FAILURE;
  }

  try
  {
    MaskImageType::Pointer itkMsk = MaskImageType::New();
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(parsedArgs["mask"].ToString());
    mitk::CastToItkImage(img, itkMsk);

    ImageType::Pointer itkImage = ImageType::New();
    mitk::Image::Pointer img2 = mitk::IOUtil::Load<mitk::Image>(parsedArgs["input"].ToString());
    mitk::CastToItkImage(img2, itkImage);

    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(itkImage);
    filter->SetMaskImage(itkMsk);

    if (parsedArgs.count("number-of-controllpoints") > 0)
    {
      const unsigned int variable = GetPositiveInt(parsedArgs, "number-of-controllpoints");
      MITK_INFO << "Number of control points: " << variable;
      filter->SetNumberOfControlPoints(variable);
    }
    if (parsedArgs.count("number-of-fitting-levels") > 0)
    {
      const unsigned int variable = GetPositiveInt(parsedArgs, "number-of-fitting-levels");
      MITK_INFO << "Number of fitting levels: " << variable;
      filter->SetNumberOfFittingLevels(variable);
    }
    if (parsedArgs.count("number-of-histogram-bins") > 0)
    {
      const unsigned int variable = GetPositiveInt(parsedArgs, "number-of-histogram-bins");
      MITK_INFO << "Number of histogram bins: " << variable;
      filter->SetNumberOfHistogramBins(variable);
    }
    if (parsedArgs.count("spline-order") > 0)
    {
      const unsigned int variable = GetPositiveInt(parsedArgs, "spline-order");
      MITK_INFO << "Spline Order " << variable;
      filter->SetSplineOrder(variable);
    }
    if (parsedArgs.count("winer-filter-noise") > 0)
    {
      const float variable = us::any_cast<float>(parsedArgs["winer-filter-noise"]);
      MITK_INFO << "Wiener filter noise: " << variable;
      filter->SetWienerFilterNoise(variable);
    }
    // The filter expects one iteration count per fitting level but does not
    // resize the array when the number of levels changes.
    const auto fittingLevels = filter->GetNumberOfFittingLevels();
    const unsigned int maximumNumberOfLevels = *std::max_element(fittingLevels.Begin(), fittingLevels.End());
    auto iterations = filter->GetMaximumNumberOfIterations();
    unsigned int iterationsPerLevel = iterations.Size() > 0 ? iterations[0] : 50;

    if (parsedArgs.count("number-of-maximum-iterations") > 0)
    {
      iterationsPerLevel = GetPositiveInt(parsedArgs, "number-of-maximum-iterations");
      MITK_INFO << "Number of Maximum Iterations: " << iterationsPerLevel;
    }

    iterations.SetSize(maximumNumberOfLevels);
    iterations.Fill(iterationsPerLevel);
    filter->SetMaximumNumberOfIterations(iterations);

    filter->Update();
    auto out = filter->GetOutput();
    mitk::Image::Pointer outImg = mitk::Image::New();
    mitk::CastToMitkImage(out, outImg);
    mitk::IOUtil::Save(outImg, parsedArgs["output"].ToString());
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "MITK exception: " << e.what();
    return EXIT_FAILURE;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Exception: " << e.what();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
