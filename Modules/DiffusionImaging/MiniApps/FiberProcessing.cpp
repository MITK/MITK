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

#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <mitkFiberBundleX.h>
#include "ctkCommandLineParser.h"
#include <boost/lexical_cast.hpp>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>


mitk::FiberBundleX::Pointer LoadFib(std::string filename)
{
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( filename, s1, s2, false );
    if( fibInfile.empty() )
        std::cout << "File " << filename << " could not be read!";

    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    return dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
}

int FiberProcessing(int argc, char* argv[])
{
    std::cout << "FiberProcessing";
    mitkCommandLineParser parser;

    parser.setTitle("Fiber Processing");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input fiber bundle (.fib)", us::Any(), false);
    parser.addArgument("outFile", "o", mitkCommandLineParser::OutputFile, "Output:", "output fiber bundle (.fib)", us::Any(), false);

    parser.addArgument("smooth", "s", mitkCommandLineParser::Float, "Spline resampling:", "Resample fiber using splines with the given point distance (in mm)");
    parser.addArgument("compress", "c", mitkCommandLineParser::Float, "Compress:", "Compress fiber using the given error threshold (in mm)");
    parser.addArgument("minLength", "l", mitkCommandLineParser::Float, "Minimum length:", "Minimum fiber length (in mm)");
    parser.addArgument("maxLength", "m", mitkCommandLineParser::Float, "Maximum length:", "Maximum fiber length (in mm)");
    parser.addArgument("minCurv", "a", mitkCommandLineParser::Float, "Minimum curvature radius:", "Minimum curvature radius (in mm)");
    parser.addArgument("mirror", "p", mitkCommandLineParser::Int, "Invert coordinates:", "Invert fiber coordinates XYZ (e.g. 010 to invert y-coordinate of each fiber point)");

    parser.addArgument("rotate-x", "rx", mitkCommandLineParser::Float, "Rotate x-axis:", "Rotate around x-axis (if copy is given the copy is rotated, in deg)");
    parser.addArgument("rotate-y", "ry", mitkCommandLineParser::Float, "Rotate y-axis:", "Rotate around y-axis (if copy is given the copy is rotated, in deg)");
    parser.addArgument("rotate-z", "rz", mitkCommandLineParser::Float, "Rotate z-axis:", "Rotate around z-axis (if copy is given the copy is rotated, in deg)");

    parser.addArgument("scale-x", "sx", mitkCommandLineParser::Float, "Scale x-axis:", "Scale in direction of x-axis (if copy is given the copy is scaled)");
    parser.addArgument("scale-y", "sy", mitkCommandLineParser::Float, "Scale y-axis:", "Scale in direction of y-axis (if copy is given the copy is scaled)");
    parser.addArgument("scale-z", "sz", mitkCommandLineParser::Float, "Scale z-axis", "Scale in direction of z-axis (if copy is given the copy is scaled)");

    parser.addArgument("translate-x", "tx", mitkCommandLineParser::Float, "Translate x-axis:", "Translate in direction of x-axis (if copy is given the copy is translated, in mm)");
    parser.addArgument("translate-y", "ty", mitkCommandLineParser::Float, "Translate y-axis:", "Translate in direction of y-axis (if copy is given the copy is translated, in mm)");
    parser.addArgument("translate-z", "tz", mitkCommandLineParser::Float, "Translate z-axis:", "Translate in direction of z-axis (if copy is given the copy is translated, in mm)");


    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    float smoothDist = -1;
    if (parsedArgs.count("smooth"))
        smoothDist = us::any_cast<float>(parsedArgs["smooth"]);

    float compress = -1;
    if (parsedArgs.count("compress"))
        compress = us::any_cast<float>(parsedArgs["compress"]);

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

    float rotateX = 0;
    if (parsedArgs.count("rotate-x"))
        rotateX = us::any_cast<float>(parsedArgs["rotate-x"]);

    float rotateY = 0;
    if (parsedArgs.count("rotate-y"))
        rotateY = us::any_cast<float>(parsedArgs["rotate-y"]);

    float rotateZ = 0;
    if (parsedArgs.count("rotate-z"))
        rotateZ = us::any_cast<float>(parsedArgs["rotate-z"]);

    float scaleX = 0;
    if (parsedArgs.count("scale-x"))
        scaleX = us::any_cast<float>(parsedArgs["scale-x"]);

    float scaleY = 0;
    if (parsedArgs.count("scale-y"))
        scaleY = us::any_cast<float>(parsedArgs["scale-y"]);

    float scaleZ = 0;
    if (parsedArgs.count("scale-z"))
        scaleZ = us::any_cast<float>(parsedArgs["scale-z"]);

    float translateX = 0;
    if (parsedArgs.count("translate-x"))
        translateX = us::any_cast<float>(parsedArgs["translate-x"]);

    float translateY = 0;
    if (parsedArgs.count("translate-y"))
        translateY = us::any_cast<float>(parsedArgs["translate-y"]);

    float translateZ = 0;
    if (parsedArgs.count("translate-z"))
        translateZ = us::any_cast<float>(parsedArgs["translate-z"]);


    string inFileName = us::any_cast<string>(parsedArgs["input"]);
    string outFileName = us::any_cast<string>(parsedArgs["outFile"]);

    try
    {
        mitk::FiberBundleX::Pointer fib = LoadFib(inFileName);

        if (minFiberLength>0)
            fib->RemoveShortFibers(minFiberLength);

        if (maxFiberLength>0)
            fib->RemoveLongFibers(maxFiberLength);

        if (curvThres>0)
            fib->ApplyCurvatureThreshold(curvThres, false);

        if (smoothDist>0)
            fib->ResampleSpline(smoothDist);

        if (compress>0)
            fib->Compress(compress);

        if (axis/100==1)
            fib->MirrorFibers(0);

        if ((axis%100)/10==1)
            fib->MirrorFibers(1);

        if (axis%10==1)
            fib->MirrorFibers(2);


        if (rotateX > 0 || rotateY > 0 || rotateZ > 0){
            std::cout << "Rotate " << rotateX << " " << rotateY << " " << rotateZ;
            fib->RotateAroundAxis(rotateX, rotateY, rotateZ);
        }
        if (translateX > 0 || translateY > 0 || translateZ > 0){
            fib->TranslateFibers(translateX, translateY, translateZ);
        }
        if (scaleX > 0 || scaleY > 0 || scaleZ > 0)
            fib->ScaleFibers(scaleX, scaleY, scaleZ);

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
RegisterDiffusionMiniApp(FiberProcessing);
