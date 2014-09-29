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

#include "MiniAppManager.h"
#include <mitkBaseDataIOFactory.h>
#include <metaCommand.h>
#include "ctkCommandLineParser.h"
#include <usAny.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <mitkCoreObjectFactory.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundleX.h>

#define _USE_MATH_DEFINES
#include <math.h>

int FiberJoin(int argc, char* argv[])
  {
    ctkCommandLineParser parser;

    parser.setTitle("Fiber Join");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setContributor("MBI");
    parser.setDescription("");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::StringList, "Input:", "input tractograms (.fib, vtk file format)", us::Any(), false);
    parser.addArgument("out", "o", ctkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    ctkCommandLineParser::StringContainerType inFibs = us::any_cast<ctkCommandLineParser::StringContainerType>(parsedArgs["input"]);
    string outFib = us::any_cast<string>(parsedArgs["out"]);

    if (inFibs.size()<=1)
    {
        MITK_INFO << "More than one input tractogram required!";
        return EXIT_FAILURE;
    }

    try
    {
        mitk::FiberBundleX::Pointer result = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(inFibs.at(0))->GetData());
        for (int i=1; i<inFibs.size(); i++)
        {
            try
            {
                mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(inFibs.at(i))->GetData());
                result = result->AddBundle(inputTractogram);
            }
            catch(...){ MITK_INFO << "could not load: " << inFibs.at(i); }
        }
        mitk::IOUtil::SaveBaseData(result, outFib);
    }
    catch (itk::ExceptionObject e)
    {
        MITK_INFO << e;
        return EXIT_FAILURE;
    }
    catch (std::exception e)
    {
        MITK_INFO << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        MITK_INFO << "ERROR!?!";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(FiberJoin);
