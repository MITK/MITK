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

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

#include <mitkGIFCooccurenceMatrix.h>


typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input VTK polydata", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputImage, "Input Mask", "Mask Image that specifies the area over for the statistic, (Values = 1)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false);

  parser.addArgument("cooccurence","cooc",mitkCommandLineParser::Int, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Global Image Feature calculator");
  parser.setDescription("Calculates different global statistics for a given segmentation / image combination");
  parser.setContributor("MBI");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }
  bool useCooc = parsedArgs.count("cooccurence");

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  mitk::Image::Pointer mask = mitk::IOUtil::LoadImage(parsedArgs["mask"].ToString());

  mitk::AbstractGlobalImageFeature::FeatureListType stats;
  if (useCooc)
  {
    MITK_INFO << "Calculating co-occurence matrix features";
    mitk::GIFCooccurenceMatrix coocCalculator;
    stats = coocCalculator.CalculateFeatures(image, mask);
  }

  for (int i = 0; i < stats.size(); ++i)
  {
    std::cout << stats[i].first << " - " << stats[i].second <<std::endl;
  }

  return 0;
}

#endif