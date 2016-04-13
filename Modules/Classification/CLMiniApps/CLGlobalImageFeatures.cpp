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

#include <mitkGIFCooccurenceMatrix.h>
#include <mitkGIFGrayLevelRunLength.h>
#include <mitkGIFFirstOrderStatistics.h>
#include <mitkGIFVolumetricStatistics.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

static std::vector<double> splitDouble(std::string str, char delimiter) {
  std::vector<double> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  double val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input VTK polydata", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputImage, "Input Mask", "Mask Image that specifies the area over for the statistic, (Values = 1)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false);

  parser.addArgument("cooccurence","cooc",mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());
  parser.addArgument("volume","vol",mitkCommandLineParser::String, "Use Volume-Statistic", "calculates volume based features",us::Any());
  parser.addArgument("run-length","rl",mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());
  parser.addArgument("first-order","fo",mitkCommandLineParser::String, "Use First Order Features", "calculates First order based features",us::Any());
  parser.addArgument("header","head",mitkCommandLineParser::String,"Add Header (Labels) to output","",us::Any());
  parser.addArgument("description","d",mitkCommandLineParser::String,"Text","Description that is added to the output",us::Any());
  parser.addArgument("same-space", "sp", mitkCommandLineParser::String, "Bool", "Set the spacing of all images to equal. Otherwise an error will be thrown. ", us::Any());
  parser.addArgument("direction", "dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... Without dimension 0,1,2... ", us::Any());

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Global Image Feature calculator");
  parser.setDescription("Calculates different global statistics for a given segmentation / image combination");
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

  MITK_INFO << "Version: "<< 1.3;

  bool useCooc = parsedArgs.count("cooccurence");

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  mitk::Image::Pointer mask = mitk::IOUtil::LoadImage(parsedArgs["mask"].ToString());

  bool fixDifferentSpaces = parsedArgs.count("same-space");
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetOrigin(), image->GetGeometry(0)->GetOrigin()))
  {
    MITK_INFO << "Not equal Origins";
    if (fixDifferentSpaces)
    {
      image->GetGeometry(0)->SetOrigin(mask->GetGeometry(0)->GetOrigin());
    } else
    {
      return -1;
    }
  }
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetSpacing(), image->GetGeometry(0)->GetSpacing()))
  {
    MITK_INFO << "Not equal Sapcings";
    if (fixDifferentSpaces)
    {
      image->GetGeometry(0)->SetSpacing(mask->GetGeometry(0)->GetSpacing());
    } else
    {
      return -1;
    }
  }

  int direction = 0;
  if (parsedArgs.count("direction"))
  {
    direction = splitDouble(parsedArgs["direction"].ToString(), ';')[0];
  }

  mitk::AbstractGlobalImageFeature::FeatureListType stats;
  ////////////////////////////////////////////////////////////////
  // CAlculate First Order Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("first-order"))
  {
    MITK_INFO << "Start calculating first order statistics....";
    mitk::GIFFirstOrderStatistics::Pointer firstOrderCalculator = mitk::GIFFirstOrderStatistics::New();
    auto localResults = firstOrderCalculator->CalculateFeatures(image, mask);
    stats.insert(stats.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating first order statistics....";
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Volume based Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("volume"))
  {
    MITK_INFO << "Start calculating volumetric ....";
    mitk::GIFVolumetricStatistics::Pointer volCalculator = mitk::GIFVolumetricStatistics::New();
    auto localResults = volCalculator->CalculateFeatures(image, mask);
    stats.insert(stats.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating volumetric....";
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Co-occurence Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("cooccurence"))
  {
    auto ranges = splitDouble(parsedArgs["cooccurence"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating coocurence with range " << ranges[i] << "....";
      mitk::GIFCooccurenceMatrix::Pointer coocCalculator = mitk::GIFCooccurenceMatrix::New();
      coocCalculator->SetRange(ranges[i]);
      coocCalculator->SetDirection(direction);
      auto localResults = coocCalculator->CalculateFeatures(image, mask);
      stats.insert(stats.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating coocurence with range " << ranges[i] << "....";
    }
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Run-Length Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("run-length"))
  {
    auto ranges = splitDouble(parsedArgs["run-length"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating run-length with number of bins " << ranges[i] << "....";
      mitk::GIFGrayLevelRunLength::Pointer calculator = mitk::GIFGrayLevelRunLength::New();
      calculator->SetRange(ranges[i]);

      auto localResults = calculator->CalculateFeatures(image, mask);
      stats.insert(stats.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating run-length with number of bins " << ranges[i] << "....";
    }
  }
  for (int i = 0; i < stats.size(); ++i)
  {
    std::cout << stats[i].first << " - " << stats[i].second <<std::endl;
  }

  std::ofstream output(parsedArgs["output"].ToString(),std::ios::app);
  if ( parsedArgs.count("header") )
  {
    if ( parsedArgs.count("description") )
    {
      output << "Description" << ";";
    }
    for (int i = 0; i < stats.size(); ++i)
    {
      output << stats[i].first << ";";
    }
    output << std::endl;
  }
  if ( parsedArgs.count("description") )
  {
    output << parsedArgs["description"].ToString() << ";";
  }
  for (int i = 0; i < stats.size(); ++i)
  {
    output << stats[i].second << ";";
  }
  output << std::endl;
  output.close();

  return 0;
}

#endif