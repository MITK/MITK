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
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkPlanarFigure.h>
#include <mitkStandaloneDataStorage.h>


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
    parser.addArgument("planarfigure", "p", mitkCommandLineParser::InputFile, "PlanarFigure:", "mask image / roi image denotin area on which statistics are calculated", us::Any(),false);
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

    std::string inputPFFile;
    if (parsedArgs.count("planarfigure") || parsedArgs.count("p"))
    {
        inputPFFile = us::any_cast<std::string>(parsedArgs["planarfigure"]);
    }
    else
    {
        inputPFFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_rectangle.pf";
    }


    mitk::PlanarFigure::Pointer planarFigure;
    try
    {
        std::vector<mitk::BaseData::Pointer> loadedObjects;
        // try except
        loadedObjects = mitk::IOUtil::Load(inputPFFile);
        mitk::BaseData::Pointer pf = loadedObjects[0];
        planarFigure = dynamic_cast<mitk::PlanarFigure*>(pf.GetPointer());
        if (planarFigure.IsNull())
        {
            MITK_ERROR << "something went wrong";
            return -1;
        }
    }
    catch (const itk::ExceptionObject& e)
    {
        MITK_ERROR << "Failed: " << e.what();
        return -1;
    }

    std::string outFile;
    if (parsedArgs.count("out") || parsedArgs.count("o") )
      outFile = us::any_cast<std::string>(parsedArgs["out"]);
    else
      outFile = "planarFigureExtraction_output.nrrd";

    // Load image and mask
    mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputImageFile);

    // Calculate statistics
    mitk::PlanarFigureMaskGenerator::Pointer planarFigMaskExtr = mitk::PlanarFigureMaskGenerator::New();
    planarFigMaskExtr->SetPlanarFigure(planarFigure);
    planarFigMaskExtr->SetInputImage(inputImage);

    mitk::Image::Pointer outImage;
    try
    {
        outImage = planarFigMaskExtr->GetMask();
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Failed - ITK Exception:" << e.what();
      return -1;
    }

    mitk::BaseGeometry *imageGeo = outImage->GetGeometry();
    std::cout << "origin: " << imageGeo->GetOrigin()[0] << " " << imageGeo->GetOrigin()[1] << " " << imageGeo->GetOrigin()[2] << std::endl;
    if (outImage != nullptr)
    {
        mitk::IOUtil::SaveImage(outImage, outFile);
    }


    return EXIT_SUCCESS;
}
