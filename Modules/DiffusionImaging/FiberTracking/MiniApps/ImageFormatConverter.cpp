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
#include <mitkImageCast.h>
#include <mitkDiffusionImage.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <mitkNrrdDiffusionImageWriter.h>

using namespace mitk;
#include "ctkCommandLineParser.h"

int ImageFormatConverter(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("in", "i", ctkCommandLineParser::String, "input image", mitk::Any(), false);
    parser.addArgument("out", "o", ctkCommandLineParser::String, "output image", mitk::Any(), false);

    map<string, mitk::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    string imageName = mitk::any_cast<string>(parsedArgs["in"]);
    string outImage = mitk::any_cast<string>(parsedArgs["out"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();

        MITK_INFO << "Loading image ...";
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( imageName, s1, s2, false );

        MITK_INFO << "Writing " << outImage;
        if ( dynamic_cast<DiffusionImage<short>*>(infile.at(0).GetPointer()) )
        {
            DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(infile.at(0).GetPointer());
            NrrdDiffusionImageWriter<short>::Pointer writer = NrrdDiffusionImageWriter<short>::New();
            writer->SetFileName(outImage);
            writer->SetInput(dwi);
            writer->Update();
        }
        else if ( dynamic_cast<Image*>(infile.at(0).GetPointer()) )
        {
            Image::Pointer image = dynamic_cast<Image*>(infile.at(0).GetPointer());
            mitk::IOUtil::SaveImage(image, outImage);
        }
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
    MITK_INFO << "DONE";
    return EXIT_SUCCESS;
}
RegisterFiberTrackingMiniApp(ImageFormatConverter);
