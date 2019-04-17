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
#ifndef mitkCLPolyToNrrd_cpp
#define mitkCLPolyToNrrd_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

#include "itkImageRegionIterator.h"
// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

#include <mitkMRNormLinearStatisticBasedFilter.h>
#include <mitkMRNormTwoRegionBasedFilter.h>
// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

int main(int argc, char* argv[])
{
  MITK_INFO << "Start";
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::Image, "Input Image", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mode", "mode", mitkCommandLineParser::Image, "Normalisation mode", "1,2,3: Single Area normalization to Mean, Median, Mode, 4,5,6: Mean, Median, Mode of two regions. ", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask0", "m0", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 0", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask1", "m1", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 1", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output Image", "Target file. The output statistic is appended to this file.", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("ignore-outlier", "outlier", mitkCommandLineParser::Bool, "Ignore Outlier", "Ignores the highest and lowest 2% during calculation. Only on single mask normalization.", us::Any(), true);
  parser.addArgument("value", "v", mitkCommandLineParser::Float, "Target Value", "Target value, the target value (for example median) is set to this value.", us::Any(), true);
  parser.addArgument("width", "w", mitkCommandLineParser::Float, "Target Width", "Ignores the highest and lowest 2% during calculation. Only on single mask normalization.", us::Any(), true);
  parser.addArgument("float", "float", mitkCommandLineParser::Bool, "Target Width", "Ignores the highest and lowest 2% during calculation. Only on single mask normalization.", us::Any(), true);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("MR Normalization Tool");
  parser.setDescription("Normalizes a MR image. Sets the Median of the tissue covered by mask 0 to 0 and the median of the area covered by mask 1 to 1.");
  parser.setContributor("MBI");

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

  MITK_INFO << "Mode access";
  int mode =std::stoi(us::any_cast<std::string>(parsedArgs["mode"]));
  MITK_INFO << "Mode: " << mode;

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
    double target = us::any_cast<float>(parsedArgs["value"]);
    oneRegion->SetTargetValue(target);
  }
  if (parsedArgs.count("width"))
  {
    double width = us::any_cast<float>(parsedArgs["width"]);
    oneRegion->SetTargetValue(width);
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

  return 0;
}

#endif
