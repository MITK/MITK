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
#include "mitkCommandLineParser.h"
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkFiberBundle.h>

/*!
\brief Load image and save as specified file type.
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Format Converter");
    parser.setCategory("Preprocessing Tools");
    parser.setDescription("Load image and save as specified file type.");
    parser.setContributor("MIC");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input file", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    std::string inName = us::any_cast<std::string>(parsedArgs["i"]);
    std::string outName = us::any_cast<std::string>(parsedArgs["o"]);

    try
    {
        mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
        std::vector<mitk::BaseData::Pointer> baseData = mitk::IOUtil::Load(inName, &functor);

        if ( baseData.size()>0 && dynamic_cast<mitk::Image*>(baseData[0].GetPointer()) )
        {
            mitk::IOUtil::Save(dynamic_cast<mitk::Image*>(baseData[0].GetPointer()), outName.c_str());
        }
        else if ( baseData.size()>0 && dynamic_cast<mitk::FiberBundle*>(baseData[0].GetPointer()) )
        {
            mitk::IOUtil::Save(dynamic_cast<mitk::FiberBundle*>(baseData[0].GetPointer()) ,outName.c_str());
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
