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
#include "mitkIOUtil.h"
#include <mitkImageCast.h>
#include "mitkCommandLineParser.h"
#include <itkN4BiasFieldCorrectionImageFilter.h>

#include <itkSTAPLEImageFilter.h>

int main(int argc, char* argv[])
{
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef itk::Image<float, 3> ImageType;
  typedef itk::N4BiasFieldCorrectionImageFilter < ImageType, MaskImageType, ImageType > FilterType;

  mitkCommandLineParser parser;
  parser.setTitle("N4 Bias Field Correction");
  parser.setCategory("Classification Command Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::Directory, "Input file:", "Input file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::File, "Output file:", "Mask file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("number-of-controllpoints", "noc", mitkCommandLineParser::Int, "Parameter", "The noc for the point grid size defining the B-spline estimate (default 4)", us::Any(), true);
  parser.addArgument("number-of-fitting-levels", "nofl", mitkCommandLineParser::Int, "Parameter", "Number of fitting levels for the multi-scale approach (default 1)", us::Any(), true);
  parser.addArgument("number-of-histogram-bins", "nofl", mitkCommandLineParser::Int, "Parameter", "number of bins defining the log input intensity histogram (default 200)", us::Any(), true);
  parser.addArgument("spline-order", "so", mitkCommandLineParser::Int, "Parameter", "Define the spline order (default 3)", us::Any(), true);
  parser.addArgument("winer-filter-noise", "wfn", mitkCommandLineParser::Float, "Parameter", "Noise estimate defining the Wiener filter (default 0.01)", us::Any(), true);
  parser.addArgument("number-of-maximum-iterations", "nomi", mitkCommandLineParser::Int, "Parameter", "Spezifies the maximum number of iterations per run", us::Any(), true);
  // ToDo: Number Of Maximum Iterations durchschleifen

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

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
    int variable = us::any_cast<int>(parsedArgs["maximum-iterations"]);
    MITK_INFO << "Number of controll points: " << variable;
    filter->SetNumberOfControlPoints(variable);
  }
  if (parsedArgs.count("number-of-fitting-levels") > 0)
  {
    int variable = us::any_cast<int>(parsedArgs["number-of-fitting-levels"]);
    MITK_INFO << "Number of fitting levels: " << variable;
    filter->SetNumberOfFittingLevels(variable);
  }
  if (parsedArgs.count("number-of-histogram-bins") > 0)
  {
    int variable = us::any_cast<int>(parsedArgs["number-of-histogram-bins"]);
    MITK_INFO << "Number of histogram bins: " << variable;
    filter->SetNumberOfHistogramBins(variable);
  }
  if (parsedArgs.count("spline-order") > 0)
  {
    int variable = us::any_cast<int>(parsedArgs["spline-order"]);
    MITK_INFO << "Spline Order " << variable;
    filter->SetSplineOrder(variable);
  }
  if (parsedArgs.count("winer-filter-noise") > 0)
  {
    float variable = us::any_cast<float>(parsedArgs["winer-filter-noise"]);
    MITK_INFO << "Number of histogram bins: " << variable;
    filter->SetWienerFilterNoise(variable);
  }
  if (parsedArgs.count("number-of-maximum-iterations") > 0)
  {
    int variable = us::any_cast<int>(parsedArgs["number-of-maximum-iterations"]);
    MITK_INFO << "Number of Maximum Iterations: " << variable;
    auto list = filter->GetMaximumNumberOfIterations();
    list.Fill(variable);
    filter->SetMaximumNumberOfIterations(list);
  }

  filter->Update();
  auto out = filter->GetOutput();
  mitk::Image::Pointer outImg = mitk::Image::New();
  mitk::CastToMitkImage(out, outImg);
  mitk::IOUtil::Save(outImg, parsedArgs["output"].ToString());

  return EXIT_SUCCESS;
}
