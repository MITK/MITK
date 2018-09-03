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

#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <mitkIOUtil.h>
#include <mitkLexicalCast.h>
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <itksys/SystemTools.hxx>
#include <itkFiberExtractionFilter.h>
#include <itkTractDensityImageFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

typedef itksys::SystemTools ist;
typedef itk::Image<float, 3>    ItkFloatImgType;

/*!
\brief Extract fibers from a tractogram using binary image ROIs
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Filter Outliers by Tract Density");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input tractogram (.fib/.trk/.tck/.dcm)", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);

  parser.addArgument("threshold", "", mitkCommandLineParser::Float, "Threshold:", "positive means ROI image value threshold", 0.05);
  parser.addArgument("overlap", "", mitkCommandLineParser::Float, "Overlap:", "positive means ROI image value threshold", 0.5);
  parser.addArgument("min_fibers", "", mitkCommandLineParser::Int, "Min. num. fibers:", "discard positive tracts with less fibers", 0);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFib = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outFib = us::any_cast<std::string>(parsedArgs["o"]);

  int min_fibers = 0;
  if (parsedArgs.count("min_fibers"))
    min_fibers = us::any_cast<int>(parsedArgs["min_fibers"]);

  float overlap = 0.5;
  if (parsedArgs.count("overlap"))
    overlap = us::any_cast<float>(parsedArgs["overlap"]);

  float threshold = 0.05f;
  if (parsedArgs.count("threshold"))
    threshold = us::any_cast<float>(parsedArgs["threshold"]);

  try
  {
    mitk::FiberBundle::Pointer inputTractogram = mitk::IOUtil::Load<mitk::FiberBundle>(inFib);

    itk::TractDensityImageFilter< ItkFloatImgType >::Pointer generator = itk::TractDensityImageFilter< ItkFloatImgType >::New();
    generator->SetFiberBundle(inputTractogram);
    generator->SetBinaryOutput(false);
    generator->SetOutputAbsoluteValues(false);
    generator->Update();

    itk::FiberExtractionFilter<float>::Pointer extractor = itk::FiberExtractionFilter<float>::New();
    extractor->SetRoiImages({generator->GetOutput()});
    extractor->SetInputFiberBundle(inputTractogram);
    extractor->SetOverlapFraction(overlap);
    extractor->SetInterpolate(true);
    extractor->SetThreshold(threshold);
    extractor->SetNoNegatives(true);
    extractor->Update();
    if (extractor->GetPositives().at(0)->GetNumFibers() >= static_cast<unsigned int>(min_fibers))
      mitk::IOUtil::Save(extractor->GetPositives().at(0), outFib);
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
