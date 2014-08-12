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
#include <mitkImageCast.h>
#include <mitkImageToItk.h>

#define _USE_MATH_DEFINES
#include <math.h>

int FiberExtraction(int argc, char* argv[])
{
    ctkCommandLineParser parser;

    parser.setTitle("Fiber Extraction");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setContributor("MBI");
    parser.setDescription("");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::String, "Input:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("out", "o", ctkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);
    parser.addArgument("planfirgure1", "pf1", ctkCommandLineParser::String, "Figure 1:", "first ROI", us::Any(), false);
    parser.addArgument("planfirgure2", "pf2", ctkCommandLineParser::String, "Figure 2:", "second ROI", us::Any());
    parser.addArgument("operation", "op", ctkCommandLineParser::String, "Operation:", "logical operation (AND, OR, NOT)", us::Any());


    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string inFib = us::any_cast<string>(parsedArgs["input"]);
    string outFib = us::any_cast<string>(parsedArgs["out"]);
    string pf1_path = us::any_cast<string>(parsedArgs["planfirgure1"]);

    string operation("");
    string pf2_path("");
    if (parsedArgs.count("operation"))
    {
        operation = us::any_cast<string>(parsedArgs["operation"]);
        if (parsedArgs.count("planfirgure2") && (operation=="AND" || operation=="OR"))
            pf2_path = us::any_cast<string>(parsedArgs["planfirgure2"]);
    }

    try
    {
        typedef itk::Image<unsigned char, 3>                                    ItkUcharImgType;

        // load fiber bundle
        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(inFib)->GetData());
        mitk::FiberBundleX::Pointer result;

        mitk::PlanarFigure::Pointer pf1 = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::LoadDataNode(pf1_path)->GetData());

        if (pf1.IsNotNull())
        {
            mitk::PlanarFigure::Pointer pf2;
            if (!pf2_path.empty())
                pf2 = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::LoadDataNode(pf2_path)->GetData());

            mitk::PlanarFigureComposite::Pointer pfc = mitk::PlanarFigureComposite::New();

            if (operation.empty())
            {
                result = inputTractogram->ExtractFiberSubset(pf1);
            }
            else if (operation=="NOT")
            {
                pfc->setOperationType(mitk::PFCOMPOSITION_NOT_OPERATION);
                pfc->addPlanarFigure(pf1);
                result = inputTractogram->ExtractFiberSubset(pfc);
            }
            else if (operation=="AND" && pf2.IsNotNull())
            {
                pfc->setOperationType(mitk::PFCOMPOSITION_AND_OPERATION);
                pfc->addPlanarFigure(pf1);
                pfc->addPlanarFigure(pf2);
                result = inputTractogram->ExtractFiberSubset(pfc);
            }
            else if (operation=="OR" && pf2.IsNotNull())
            {
                pfc->setOperationType(mitk::PFCOMPOSITION_OR_OPERATION);
                pfc->addPlanarFigure(pf1);
                pfc->addPlanarFigure(pf2);
                result = inputTractogram->ExtractFiberSubset(pfc);
            }
            else
            {
                MITK_INFO << "Could not process input:";
                MITK_INFO << pf1_path;
                MITK_INFO << pf2_path;
                MITK_INFO << operation;
            }
        }
        else
        {
            ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
            mitk::Image::Pointer mitkMaskImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(pf1_path)->GetData());
            mitk::CastToItkImage<ItkUcharImgType>(mitkMaskImage, itkMaskImage);

            if (operation=="NOT")
                result = inputTractogram->ExtractFiberSubset(itkMaskImage, true, true);
            else
                result = inputTractogram->ExtractFiberSubset(itkMaskImage, true, false);
        }

        if (result.IsNotNull())
            mitk::IOUtil::SaveBaseData(result, outFib);
        else
            MITK_INFO << "No valid fiber bundle extracted.";

        MITK_INFO << "DONE";
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
RegisterDiffusionMiniApp(FiberExtraction);
