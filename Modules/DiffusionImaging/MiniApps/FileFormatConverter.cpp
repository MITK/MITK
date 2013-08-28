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
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkIOUtil.h>
#include <mitkNrrdDiffusionImageWriter.h>
#include <mitkFiberBundleX.h>
#include <mitkFiberBundleXWriter.h>
#include "ctkCommandLineParser.h"
#include "ctkCommandLineParser.cpp"

using namespace mitk;

int FileFormatConverter(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("in", "i", ctkCommandLineParser::String, "input file", us::Any(), false);
    parser.addArgument("out", "o", ctkCommandLineParser::String, "output file", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    string inName = us::any_cast<string>(parsedArgs["in"]);
    string outName = us::any_cast<string>(parsedArgs["out"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        MITK_INFO << "Loading " << inName;
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( inName, s1, s2, false );
        mitk::BaseData::Pointer baseData = infile.at(0);

        if ( dynamic_cast<DiffusionImage<short>*>(baseData.GetPointer()) )
        {
            MITK_INFO << "Writing " << outName;
            DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(baseData.GetPointer());
            NrrdDiffusionImageWriter<short>::Pointer writer = NrrdDiffusionImageWriter<short>::New();
            writer->SetFileName(outName);
            writer->SetInput(dwi);
            writer->Update();
        }
        else if ( dynamic_cast<Image*>(baseData.GetPointer()) )
        {
            Image::Pointer image = dynamic_cast<Image*>(baseData.GetPointer());
            mitk::IOUtil::SaveImage(image, outName);
        }
        else if ( dynamic_cast<FiberBundleX*>(baseData.GetPointer()) )
        {
            MITK_INFO << "Writing " << outName;
            FiberBundleXWriter::Pointer fibWriter = FiberBundleXWriter::New();
            fibWriter->SetFileName(outName.c_str());
            fibWriter->DoWrite( dynamic_cast<FiberBundleX*>(baseData.GetPointer()) );
        }
        else
            MITK_INFO << "File type currently not supported!";
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
RegisterDiffusionMiniApp(FileFormatConverter);
