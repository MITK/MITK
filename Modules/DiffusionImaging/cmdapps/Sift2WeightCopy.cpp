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

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <istream>

#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <mitkImageCast.h>

#include <mitkBaseData.h>
#include <mitkFiberBundle.h>
#include "mitkCommandLineParser.h"
#include <boost/lexical_cast.hpp>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <itkTractDensityImageFilter.h>
#include <itkTractsToFiberEndingsImageFilter.h>


mitk::FiberBundle::Pointer LoadFib(std::string filename)
{
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(filename);
    if( fibInfile.empty() )
        std::cout << "File " << filename << " could not be read!";
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    return dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
}

/*!
\brief Import Sift2 Fiber Weights txt file.
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Sift2 Fiber Weight Import");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Import sift2 fiber weights.");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:", "input fiber bundle", us::Any(), false);
    parser.addArgument("weights", "w", mitkCommandLineParser::String, "Weights:", "input weights file (.txt)", us::Any(), false);
    parser.addArgument("output", "o", mitkCommandLineParser::String, "Output:", "output fiber bundle", us::Any(), false);


    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    bool binary = false;
    if (parsedArgs.count("binary"))
        binary = us::any_cast<int>(parsedArgs["binary"]);

    bool endpoints = false;
    if (parsedArgs.count("endpoints"))
        endpoints = us::any_cast<int>(parsedArgs["endpoints"]);

    std::string reference_image = "";
    if (parsedArgs.count("reference_image"))
        reference_image = us::any_cast<std::string>(parsedArgs["reference_image"]);

    std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
    std::string weightsFileName = us::any_cast<std::string>(parsedArgs["weights"]);
    std::string outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    try
    {
        mitk::FiberBundle::Pointer fib = LoadFib(inFileName);

        std::ifstream fin;
        fin.open(weightsFileName);
        if (!fin.good())
          return 1; // exit if file not found

        std::vector<float> weights;
        for (float d; fin >> d; ) { weights.push_back(d); }

        for(int i = 0; i != weights.size(); i++) {
          fib->SetFiberWeight(i, weights[i]);
        }

        mitk::IOUtil::SaveBaseData(fib.GetPointer(), outFileName );
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
