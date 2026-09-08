/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommandLineParser.h>
#include <mitkException.h>
#include <mitkIOUtil.h>
#include <mitkImageCast.h>

#include <mitkMRNormLinearStatisticBasedFilter.h>
#include <mitkMRNormTwoRegionBasedFilter.h>

int main(int argc, char* argv[])
{
  MITK_INFO << "Start";
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::Image, "Input Image", "Path to the input MR image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mode", "mode", mitkCommandLineParser::Int, "Normalisation mode", "1,2,3: Single Area normalization to Mean, Median, Mode, 4,5,6: Mean, Median, Mode of two regions. ", us::Any(), false);
  parser.addArgument("mask0", "m0", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 0", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask1", "m1", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 1. Required for modes 4 to 6.", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output Image", "Path of the normalized output image.", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("ignore-outlier", "outlier", mitkCommandLineParser::Bool, "Ignore Outlier", "Ignores the highest and lowest 2% during calculation. Only on single mask normalization.", us::Any(), true);
  parser.addArgument("value", "v", mitkCommandLineParser::Float, "Target Value", "Offset added to the center statistic (mean, median or mode) of mask 0 before it is subtracted from the image. The center is mapped to -value / (stddev * width). Only on single mask normalization.", us::Any(), true);
  parser.addArgument("width", "w", mitkCommandLineParser::Float, "Target Width", "Factor applied to the standard deviation of mask 0 before the image is divided by it (default 1). Only on single mask normalization.", us::Any(), true);
  parser.addArgument("float", "float", mitkCommandLineParser::Bool, "Convert to Float", "Casts the input image to float before processing so that the output is a float image.", us::Any(), true);

  // General information about the app
  parser.setCategory("Classification Tools");
  parser.setTitle("MR Normalization Tool");
  parser.setDescription("Normalizes a MR image. Sets the Median of the tissue covered by mask 0 to 0 and the median of the area covered by mask 1 to 1.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  bool ignore_outlier = false;
  if (parsedArgs.count("ignore-outlier"))
  {
    ignore_outlier = us::any_cast<bool>(parsedArgs["ignore-outlier"]);
  }

  const int mode = us::any_cast<int>(parsedArgs["mode"]);
  MITK_INFO << "Mode: " << mode;

  if (mode < 1 || mode > 6)
  {
    MITK_ERROR << "Invalid normalization mode " << mode << ". Valid modes are 1 to 6.";
    return EXIT_FAILURE;
  }
  if (mode > 3 && parsedArgs.count("mask1") == 0)
  {
    MITK_ERROR << "Modes 4 to 6 require a second mask (--mask1).";
    return EXIT_FAILURE;
  }

  try
  {
    MITK_INFO << "Read images";
    mitk::Image::Pointer mask1;
    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["image"].ToString());

    if (parsedArgs.count("float"))
    {
      typedef itk::Image<float, 3> ImageType;
      ImageType::Pointer img = ImageType::New();
      mitk::CastToItkImage(image, img);
      mitk::CastToMitkImage(img, image);
    }

    mitk::Image::Pointer mask0 = mitk::IOUtil::Load<mitk::Image>(parsedArgs["mask0"].ToString());
    if (mode > 3)
    {
      mask1 = mitk::IOUtil::Load<mitk::Image>(parsedArgs["mask1"].ToString());
    }
    mitk::MRNormLinearStatisticBasedFilter::Pointer oneRegion = mitk::MRNormLinearStatisticBasedFilter::New();
    mitk::MRNormTwoRegionsBasedFilter::Pointer twoRegion = mitk::MRNormTwoRegionsBasedFilter::New();
    mitk::Image::Pointer output;

    oneRegion->SetInput(image);
    oneRegion->SetMask(mask0);
    oneRegion->SetIgnoreOutlier(ignore_outlier);
    twoRegion->SetInput(image);
    twoRegion->SetMask1(mask0);
    twoRegion->SetMask2(mask1);

    if (parsedArgs.count("value"))
    {
      const double target = us::any_cast<float>(parsedArgs["value"]);
      oneRegion->SetTargetValue(target);
    }
    if (parsedArgs.count("width"))
    {
      const double width = us::any_cast<float>(parsedArgs["width"]);
      oneRegion->SetTargetWidth(width);
    }

    switch (mode)
    {
    case 1:
      oneRegion->SetCenterMode(mitk::MRNormLinearStatisticBasedFilter::MEAN);
      oneRegion->Update();
      output=oneRegion->GetOutput();
      break;
    case 2:
      oneRegion->SetCenterMode(mitk::MRNormLinearStatisticBasedFilter::MEDIAN);
      oneRegion->Update();
      output=oneRegion->GetOutput();
      break;
    case 3:
      oneRegion->SetCenterMode(mitk::MRNormLinearStatisticBasedFilter::MODE);
      oneRegion->Update();
      output=oneRegion->GetOutput();
      break;
    case 4:
      twoRegion->SetArea1(mitk::MRNormTwoRegionsBasedFilter::MEAN);
      twoRegion->SetArea2(mitk::MRNormTwoRegionsBasedFilter::MEAN);
      twoRegion->Update();
      output=twoRegion->GetOutput();
      break;
    case 5:
      twoRegion->SetArea1(mitk::MRNormTwoRegionsBasedFilter::MEDIAN);
      twoRegion->SetArea2(mitk::MRNormTwoRegionsBasedFilter::MEDIAN);
      twoRegion->Update();
      output=twoRegion->GetOutput();
      break;
    case 6:
      twoRegion->SetArea1(mitk::MRNormTwoRegionsBasedFilter::MODE);
      twoRegion->SetArea2(mitk::MRNormTwoRegionsBasedFilter::MODE);
      twoRegion->Update();
      output=twoRegion->GetOutput();
      break;
    }

    mitk::IOUtil::Save(output, parsedArgs["output"].ToString());
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
