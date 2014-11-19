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
#include <mitkDiffusionImage.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

using namespace mitk;


int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Copy Geometry");
    parser.setCategory("Preprocessing Tools");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(), false);
    parser.addArgument("ref", "r", mitkCommandLineParser::InputFile, "Reference:", "reference image", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output:", "output image", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    string imageName = us::any_cast<string>(parsedArgs["in"]);
    string refImage = us::any_cast<string>(parsedArgs["ref"]);
    string outImage = us::any_cast<string>(parsedArgs["out"]);

    try
    {
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( refImage, s1, s2, false );
        Image::Pointer source = dynamic_cast<Image*>(infile.at(0).GetPointer());
        infile = BaseDataIO::LoadBaseDataFromFile( imageName, s1, s2, false );
        Image::Pointer target = dynamic_cast<Image*>(infile.at(0).GetPointer());

        mitk::BaseGeometry* s_geom = source->GetGeometry();
        mitk::BaseGeometry* t_geom = target->GetGeometry();

        t_geom->SetIndexToWorldTransform(s_geom->GetIndexToWorldTransform());
        target->SetGeometry(t_geom);

        if ( dynamic_cast<DiffusionImage<short>*>(target.GetPointer()) )
        {
            mitk::IOUtil::Save(dynamic_cast<DiffusionImage<short>*>(target.GetPointer()), outImage.c_str());
        }
        else
            mitk::IOUtil::SaveImage(target, outImage);
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
