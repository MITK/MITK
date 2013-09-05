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
#include "ctkCommandLineParser.h"

using namespace mitk;
#include "ctkCommandLineParser.h"

int CopyGeometry(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("in", "i", ctkCommandLineParser::String, "input image", us::Any(), false);
    parser.addArgument("ref", "r", ctkCommandLineParser::String, "reference image", us::Any(), false);
    parser.addArgument("out", "o", ctkCommandLineParser::String, "output image", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    string imageName = us::any_cast<string>(parsedArgs["in"]);
    string refImage = us::any_cast<string>(parsedArgs["ref"]);
    string outImage = us::any_cast<string>(parsedArgs["out"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();

        MITK_INFO << "Loading image " << imageName;
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( refImage, s1, s2, false );
        Image::Pointer source = dynamic_cast<Image*>(infile.at(0).GetPointer());
        infile = BaseDataIO::LoadBaseDataFromFile( imageName, s1, s2, false );
        Image::Pointer target = dynamic_cast<Image*>(infile.at(0).GetPointer());

        mitk::Geometry3D* s_geom = source->GetGeometry();
        mitk::Geometry3D* t_geom = target->GetGeometry();

        t_geom->SetIndexToWorldTransform(s_geom->GetIndexToWorldTransform());
        target->SetGeometry(t_geom);

        if ( dynamic_cast<DiffusionImage<short>*>(target.GetPointer()) )
        {
            MITK_INFO << "Writing " << outImage;
            DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(target.GetPointer());
            NrrdDiffusionImageWriter<short>::Pointer writer = NrrdDiffusionImageWriter<short>::New();
            writer->SetFileName(outImage);
            writer->SetInput(dwi);
            writer->Update();
        }
        else
            mitk::IOUtil::SaveImage(target, outImage);
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
RegisterDiffusionMiniApp(CopyGeometry);
