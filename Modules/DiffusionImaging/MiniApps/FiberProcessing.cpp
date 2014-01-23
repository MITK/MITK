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

    parser.addArgument("copyAndJoin", "c", ctkCommandLineParser::Bool, "Create a copy of the input fiber bundle (applied after resample/smooth/minLength/maxLength/minCurv/mirror) and join copy with original (applied after rotate/scale/translate)");
    //parser.addArgument("join", "j", ctkCommandLineParser::Bool, "Join the original and copied fiber bundle (applied after rotate/scale/translate)");

    parser.addArgument("rotate-x", "rx", ctkCommandLineParser::Float, "Rotate around x-axis (if copy is given the copy is rotated, in deg)");
    parser.addArgument("rotate-y", "ry", ctkCommandLineParser::Float, "Rotate around y-axis (if copy is given the copy is rotated, in deg)");
    parser.addArgument("rotate-z", "rz", ctkCommandLineParser::Float, "Rotate around z-axis (if copy is given the copy is rotated, in deg)");

    parser.addArgument("scale-x", "sx", ctkCommandLineParser::Float, "Scale in direction of x-axis (if copy is given the copy is scaled)");
    parser.addArgument("scale-y", "sy", ctkCommandLineParser::Float, "Scale in direction of y-axis (if copy is given the copy is scaled)");
    parser.addArgument("scale-z", "sz", ctkCommandLineParser::Float, "Scale in direction of z-axis (if copy is given the copy is scaled)");

    parser.addArgument("translate-x", "tx", ctkCommandLineParser::Float, "Translate in direction of x-axis (if copy is given the copy is translated, in mm)");
    parser.addArgument("translate-y", "ty", ctkCommandLineParser::Float, "Translate in direction of y-axis (if copy is given the copy is translated, in mm)");
    parser.addArgument("translate-z", "tz", ctkCommandLineParser::Float, "Translate in direction of z-axis (if copy is given the copy is translated, in mm)");


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

    bool copyAndJoin = false;
    if(parsedArgs.count("copyAndJoin"))
      copyAndJoin = us::any_cast<bool>(parsedArgs["copyAndJoin"]);

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

        if (copyAndJoin == true)
        {
          MITK_INFO << "Create copy";
          mitk::FiberBundleX::Pointer fibCopy = fib->GetDeepCopy();

          if (rotateX > 0 || rotateY > 0 || rotateZ > 0){
            MITK_INFO << "Rotate " << rotateX << " " << rotateY << " " << rotateZ;
            fibCopy->RotateAroundAxis(rotateX, rotateY, rotateZ);
          }
          if (translateX > 0 || translateY > 0 || translateZ > 0)
            fibCopy->TranslateFibers(translateX, translateY, translateZ);
          if (scaleX > 0 || scaleY > 0 || scaleZ > 0)
            fibCopy->ScaleFibers(scaleX, scaleY, scaleZ);

          MITK_INFO << "Join copy with original";
          fib = fib->AddBundle(fibCopy.GetPointer());

        } else {
          if (rotateX > 0 || rotateY > 0 || rotateZ > 0){
            MITK_INFO << "Rotate " << rotateX << " " << rotateY << " " << rotateZ;
            fib->RotateAroundAxis(rotateX, rotateY, rotateZ);
          }
          if (translateX > 0 || translateY > 0 || translateZ > 0){
            fib->TranslateFibers(translateX, translateY, translateZ);
          }
          if (scaleX > 0 || scaleY > 0 || scaleZ > 0)
            fib->ScaleFibers(scaleX, scaleY, scaleZ);
        }

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
