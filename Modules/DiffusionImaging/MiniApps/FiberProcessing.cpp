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

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <itkResampleImageFilter.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionImage.h>
#include <mitkQBallImage.h>
#include <mitkBaseData.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include "ctkCommandLineParser.h"
#include <boost/lexical_cast.hpp>

/**
 * Short program to average redundant gradients in dwi-files
 */

mitk::FiberBundleX::Pointer LoadFib(std::string filename)
{
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( filename, s1, s2, false );
    if( fibInfile.empty() )
        MITK_INFO << "File " << filename << " could not be read!";

    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    return dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
}

int FiberProcessing(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::String, "input fiber bundle (.fib)", us::Any(), false);
    parser.addArgument("outFile", "o", ctkCommandLineParser::String, "output fiber bundle (.fib)", us::Any(), false);

    parser.addArgument("resample", "r", ctkCommandLineParser::Float, "Resample fiber with the given point distance (in mm)");
    parser.addArgument("smooth", "s", ctkCommandLineParser::Float, "Smooth fiber with the given point distance (in mm)");
    parser.addArgument("minLength", "l", ctkCommandLineParser::Float, "Minimum fiber length (in mm)");
    parser.addArgument("maxLength", "m", ctkCommandLineParser::Float, "Maximum fiber length (in mm)");
    parser.addArgument("minCurv", "a", ctkCommandLineParser::Float, "Minimum curvature radius (in mm)");
    parser.addArgument("mirror", "p", ctkCommandLineParser::Int, "Invert fiber coordinates XYZ (e.g. 010 to invert y-coordinate of each fiber point)");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    float pointDist = -1;
    if (parsedArgs.count("resample"))
        pointDist = us::any_cast<float>(parsedArgs["resample"]);

    float smoothDist = -1;
    if (parsedArgs.count("smooth"))
        smoothDist = us::any_cast<float>(parsedArgs["smooth"]);

    float minFiberLength = -1;
    if (parsedArgs.count("minLength"))
        minFiberLength = us::any_cast<float>(parsedArgs["minLength"]);

    float maxFiberLength = -1;
    if (parsedArgs.count("maxLength"))
        maxFiberLength = us::any_cast<float>(parsedArgs["maxLength"]);

    float curvThres = -1;
    if (parsedArgs.count("minCurv"))
        curvThres = us::any_cast<float>(parsedArgs["minCurv"]);

    int axis = 0;
    if (parsedArgs.count("mirror"))
        axis = us::any_cast<int>(parsedArgs["mirror"]);

    string inFileName = us::any_cast<string>(parsedArgs["input"]);
    string outFileName = us::any_cast<string>(parsedArgs["outFile"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        mitk::FiberBundleX::Pointer fib = LoadFib(inFileName);

        if (minFiberLength>0)
            fib->RemoveShortFibers(minFiberLength);

        if (maxFiberLength>0)
            fib->RemoveLongFibers(maxFiberLength);

        if (curvThres>0)
            fib->ApplyCurvatureThreshold(curvThres, false);

        if (pointDist>0)
            fib->ResampleFibers(pointDist);

        if (smoothDist>0)
            fib->DoFiberSmoothing(smoothDist);

        if (axis/100==1)
            fib->MirrorFibers(0);

        if ((axis%100)/10==1)
            fib->MirrorFibers(1);

        if (axis%10==1)
            fib->MirrorFibers(2);

        mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
        {
            if ( (*it)->CanWriteBaseDataType(fib.GetPointer()) ) {
                MITK_INFO << "writing " << outFileName;
                (*it)->SetFileName( outFileName.c_str() );
                (*it)->DoWrite( fib.GetPointer() );
            }
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
}
RegisterDiffusionMiniApp(FiberProcessing);
