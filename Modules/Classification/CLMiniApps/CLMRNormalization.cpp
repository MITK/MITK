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
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input VTK polydata", us::Any(), false);
  parser.addArgument("mode", "mode", mitkCommandLineParser::InputImage, "Normalisation mode", "1,2,3: Single Area normalization to Mean, Median, Mode, 4,5,6: Mean, Median, Mode of two regions. ", us::Any(), false);
  parser.addArgument("mask0", "m0", mitkCommandLineParser::InputImage, "Input Mask", "The median of the area covered by this mask will be set to 0", us::Any(), false);
  parser.addArgument("mask1", "m1", mitkCommandLineParser::InputImage, "Input Mask", "The median of the area covered by this mask will be set to 1", us::Any(), true);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output Image", "Target file. The output statistic is appended to this file.", us::Any(), false);

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

  MITK_INFO << "Mode access";
  int mode = 5;//us::any_cast<int>(parsedArgs["mode"]);

  MITK_INFO << "Read images";
  mitk::Image::Pointer mask1;
  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  mitk::Image::Pointer mask0 = mitk::IOUtil::LoadImage(parsedArgs["mask0"].ToString());
  if (mode > 3)
  {
    mask1 = mitk::IOUtil::LoadImage(parsedArgs["mask1"].ToString());
  }
  mitk::MRNormLinearStatisticBasedFilter::Pointer oneRegion = mitk::MRNormLinearStatisticBasedFilter::New();
  mitk::MRNormTwoRegionsBasedFilter::Pointer twoRegion = mitk::MRNormTwoRegionsBasedFilter::New();
  mitk::Image::Pointer output;

  //oneRegion->SetInput(image);
  twoRegion->SetInput(image);
  //oneRegion->SetMask(mask0);
  twoRegion->SetMask1(mask0);
  twoRegion->SetMask2(mask1);

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

  mitk::IOUtil::SaveImage(output, parsedArgs["output"].ToString());

  return 0;
}

#endif