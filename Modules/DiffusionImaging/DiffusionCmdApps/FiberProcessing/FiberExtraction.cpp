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
#include <mitkCoreObjectFactory.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkStandaloneDataStorage.h>

#define _USE_MATH_DEFINES
#include <math.h>

/*!
\brief Extract fibers from a tractogram using planar figure ROIs
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fiber Extraction");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Extract fibers from a tractogram using planar figure ROIs");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input tractogram (.fib/.trk)", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);
  parser.addArgument("planfirgure1", "pf1", mitkCommandLineParser::String, "Figure 1:", "first planar figure ROI", us::Any(), false);
  parser.addArgument("planfirgure2", "pf2", mitkCommandLineParser::String, "Figure 2:", "second planar figure ROI", us::Any());
  parser.addArgument("operation", "op", mitkCommandLineParser::String, "Operation:", "logical operation (AND; OR; NOT)", us::Any());


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFib = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outFib = us::any_cast<std::string>(parsedArgs["o"]);
  std::string pf1_path = us::any_cast<std::string>(parsedArgs["planfirgure1"]);

  std::string operation("");
  std::string pf2_path("");
  if (parsedArgs.count("operation"))
  {
    operation = us::any_cast<std::string>(parsedArgs["operation"]);
    if (parsedArgs.count("planfirgure2") && (operation=="AND" || operation=="OR"))
      pf2_path = us::any_cast<std::string>(parsedArgs["planfirgure2"]);
  }

  try
  {
    // load fiber bundle
    mitk::FiberBundle::Pointer inputTractogram = mitk::IOUtil::Load<mitk::FiberBundle>(inFib);
    mitk::FiberBundle::Pointer result;

    mitk::StandaloneDataStorage::Pointer storage = mitk::StandaloneDataStorage::New();
    auto data = mitk::IOUtil::Load(pf1_path)[0];
    auto input1 = mitk::DataNode::New();
    input1->SetData(data);

    if (input1.IsNotNull())
    {
      mitk::PlanarFigureComposite::Pointer pfc = mitk::PlanarFigureComposite::New();
      mitk::DataNode::Pointer pfcNode = mitk::DataNode::New();
      pfcNode->SetData(pfc);
      mitk::DataStorage::SetOfObjects::Pointer set1 = mitk::DataStorage::SetOfObjects::New();
      set1->push_back(pfcNode);
      storage->Add(pfcNode);

      auto input2 = mitk::DataNode::New();
      if (!pf2_path.empty())
      {
        data = mitk::IOUtil::Load(pf2_path)[0];
        input2->SetData(data);
      }

      if (operation.empty())
      {
        result = inputTractogram->ExtractFiberSubset(input1, nullptr);
      }
      else if (operation=="NOT")
      {
        pfc->setOperationType(mitk::PlanarFigureComposite::NOT);
        storage->Add(input1, set1);
        result = inputTractogram->ExtractFiberSubset(pfcNode, storage);
      }
      else if (operation=="AND" && input2.IsNotNull())
      {
        pfc->setOperationType(mitk::PlanarFigureComposite::AND);
        storage->Add(input1, set1);
        storage->Add(input2, set1);
        result = inputTractogram->ExtractFiberSubset(pfcNode, storage);
      }
      else if (operation=="OR" && input2.IsNotNull())
      {
        pfc->setOperationType(mitk::PlanarFigureComposite::OR);
        storage->Add(input1, set1);
        storage->Add(input2, set1);
        result = inputTractogram->ExtractFiberSubset(pfcNode, storage);
      }
      else
      {
        std::cout << "Could not process input:";
        std::cout << pf1_path;
        std::cout << pf2_path;
        std::cout << operation;
      }
    }

    if (result.IsNotNull())
      mitk::IOUtil::Save(result, outFib);
    else
      std::cout << "No valid fiber bundle extracted.";
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
