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

#include <mitkImageCast.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include "mitkCommandLineParser.h"

using namespace mitk;

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Format Converter");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input:", "input file", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output:", "output file", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    string inName = us::any_cast<string>(parsedArgs["in"]);
    string outName = us::any_cast<string>(parsedArgs["out"]);

    try
    {
        std::vector<mitk::BaseData::Pointer> baseData = mitk::IOUtil::Load(inName);

        if ( baseData.size()>0 && dynamic_cast<Image*>(baseData[0].GetPointer()) )
        {
            mitk::IOUtil::Save(dynamic_cast<Image*>(baseData[0].GetPointer()), outName.c_str());
        }
        else if ( baseData.size()>0 && dynamic_cast<FiberBundle*>(baseData[0].GetPointer()) )
        {
            mitk::IOUtil::Save(dynamic_cast<FiberBundle*>(baseData[0].GetPointer()) ,outName.c_str());
        }
        else
            std::cout << "File type currently not supported!";
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
