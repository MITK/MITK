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

#include <mitkIOUtil.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <mitkCoreObjectFactory.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundle.h>

#define _USE_MATH_DEFINES
#include <math.h>

mitk::FiberBundle::Pointer LoadFib(std::string filename)
{
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(filename);
    if( fibInfile.empty() )
        std::cout << "File " << filename << " could not be read!";
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    return dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
}

/*!
\brief Join multiple tractograms
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fiber Join");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Join multiple tractograms");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::StringList, "Input:", "input tractograms", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType inFibs = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input"]);
  std::string outFib = us::any_cast<std::string>(parsedArgs["out"]);

  if (inFibs.size()<=1)
  {
      std::cout << "More than one input tractogram required!";
      return EXIT_FAILURE;
  }

  try
  {
      mitk::FiberBundle::Pointer result = LoadFib(inFibs.at(0));
      for (std::size_t i=1; i<inFibs.size(); ++i)
      {
          try
          {
              mitk::FiberBundle::Pointer inputTractogram = LoadFib(inFibs.at(i));
              result = result->AddBundle(inputTractogram);
          }
          catch(...){ std::cout << "could not load: " << inFibs.at(i); }
      }
      mitk::IOUtil::Save(result, outFib);
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
