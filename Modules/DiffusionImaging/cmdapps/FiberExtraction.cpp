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
#include <boost/lexical_cast.hpp>
#include <mitkCoreObjectFactory.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkFiberBundle.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkStandaloneDataStorage.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

/*!
\brief Extract fibers from a tractogram using planar figure ROIs
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Fiber Extraction");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setContributor("MBI");
    parser.setDescription("Extract fibers from a tractogram using planar figure ROIs");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:", "input tractogram (.fib/.trk)", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false);
    parser.addArgument("planfirgure1", "pf1", mitkCommandLineParser::String, "Figure 1:", "first ROI", us::Any(), false);
    parser.addArgument("planfirgure2", "pf2", mitkCommandLineParser::String, "Figure 2:", "second ROI", us::Any());
    parser.addArgument("operation", "op", mitkCommandLineParser::String, "Operation:", "logical operation (AND, OR, NOT)", us::Any());


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
        mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::LoadDataNode(inFib)->GetData());
        mitk::FiberBundle::Pointer result;

        mitk::StandaloneDataStorage::Pointer storage = mitk::StandaloneDataStorage::New();
        mitk::DataNode::Pointer input1 = mitk::IOUtil::LoadDataNode(pf1_path);

        if (input1.IsNotNull())
        {
            mitk::PlanarFigureComposite::Pointer pfc = mitk::PlanarFigureComposite::New();
            mitk::DataNode::Pointer pfcNode = mitk::DataNode::New();
            pfcNode->SetData(pfc);
            mitk::DataStorage::SetOfObjects::Pointer set1 = mitk::DataStorage::SetOfObjects::New();
            set1->push_back(pfcNode);
            storage->Add(pfcNode);

            mitk::DataNode::Pointer input2;
            if (!pf2_path.empty())
            {
                input2 = mitk::IOUtil::LoadDataNode(pf2_path);
            }

            if (operation.empty())
            {
                result = inputTractogram->ExtractFiberSubset(input1, NULL);
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
