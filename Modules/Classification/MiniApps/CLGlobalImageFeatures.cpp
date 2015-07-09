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

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

static vector<double> splitDouble(string str, char delimiter) {
  vector<double> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  double val;
  while(getline(ss, tok, delimiter)) {
    stringstream s2(tok);
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
  parser.addArgument("run-length","rl",mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());
  parser.addArgument("first-order","fo",mitkCommandLineParser::String, "Use First Order Features", "calculates First order based features",us::Any());
  parser.addArgument("header","head",mitkCommandLineParser::String,"Add Header (Labels) to output","",us::Any());

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
  ////////////////////////////////////////////////////////////////
  // CAlculate First Order Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("first-order"))
  {
    mitk::GIFFirstOrderStatistics firstOrderCalculator;
    auto localResults = firstOrderCalculator.CalculateFeatures(image, mask);
    stats.insert(stats.end(), localResults.begin(), localResults.end());
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Co-occurence Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("cooccurence"))
  {
    auto ranges = splitDouble(parsedArgs["cooccurence"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      mitk::GIFCooccurenceMatrix coocCalculator;
      coocCalculator.SetRange(ranges[i]);
      auto localResults = coocCalculator.CalculateFeatures(image, mask);
      stats.insert(stats.end(), localResults.begin(), localResults.end());
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
      mitk::GIFGrayLevelRunLength calculator;
      calculator.SetRange(ranges[i]);
      auto localResults = calculator.CalculateFeatures(image, mask);
      stats.insert(stats.end(), localResults.begin(), localResults.end());
    }
  }
  for (int i = 0; i < stats.size(); ++i)
  {
    std::cout << stats[i].first << " - " << stats[i].second <<std::endl;
  }

  std::ofstream output(parsedArgs["output"].ToString(),std::ios::app);
  if ( parsedArgs.count("header") )
  {
    for (int i = 0; i < stats.size(); ++i)
    {
      output << stats[i].first << ";";
    }
    output << std::endl;
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