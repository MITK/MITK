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

#include "mitkCommandLineParser.h"
#include "mitkImage.h"
#include "mitkIOUtil.h"
#include <iostream>
#include <usAny.h>
#include <fstream>
#include <mitkHotspotMaskGenerator.h>


int main( int argc, char* argv[] )
{
    mitkCommandLineParser parser;

    parser.setTitle("Test basic hotspot functionality");
    parser.setCategory("Preprocessing Tools");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("help", "h", mitkCommandLineParser::String, "Help:", "Show this help text");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(),false);
    parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask:", "mask image / roi image denotin area on which statistics are calculated", us::Any(),true);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output", "output file (default: hotspotMiniApp_output.nrrd)", us::Any());

    std::cout << "test...." << std::endl;

    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    std::cout << "parsedArgs.size()= " << parsedArgs.size() << std::endl;
    if (parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h"))
    {
//      std::cout << "\n\n MiniApp Description: \nCalculates and saves the hotspot of an image." << endl;
//      std::cout << "Output is written to the designated output file" << endl;
//      std::cout << parser.helpText();
//      return EXIT_SUCCESS;
    }

    // Parameters:
    std::string inputImageFile;
    if (!parsedArgs.count("i") && !parsedArgs.count("input"))
    {
        inputImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D.nrrd";
    }
    else
    {
        inputImageFile = us::any_cast<std::string>(parsedArgs["input"]);
    }

    mitk::Image::Pointer maskImage;
    if (parsedArgs.count("mask") || parsedArgs.count("m"))
    {
        std::string maskImageFile = us::any_cast<std::string>(parsedArgs["mask"]);
        maskImage = mitk::IOUtil::LoadImage(maskImageFile);
    }

    std::string outFile;
    if (parsedArgs.count("out") || parsedArgs.count("o") )
      outFile = us::any_cast<std::string>(parsedArgs["out"]);
    else
      outFile = "hotspotMiniApp_output.nrrd";

    // Load image and mask
    mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputImageFile);

    // Calculate statistics
    mitk::HotspotMaskGenerator::Pointer hotspotCalc = mitk::HotspotMaskGenerator::New();
    hotspotCalc->SetInputImage(inputImage);
    hotspotCalc->SetHotspotRadiusInMM(10);
    hotspotCalc->SetTimeStep(0);
    mitk::Image::Pointer outImage;
    try
    {
        outImage = hotspotCalc->GetMask();
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Failed - ITK Exception:" << e.what();
      return -1;
    }

    if (outImage != nullptr)
    {
        mitk::IOUtil::SaveImage(outImage, outFile);
    }


    return EXIT_SUCCESS;
}
