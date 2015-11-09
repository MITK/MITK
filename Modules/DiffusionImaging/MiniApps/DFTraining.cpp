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

#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>

#include <mitkFiberBundle.h>
#include <mitkTrackingForestHandler.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

/*!
\brief Train random forest classifier for machine learning based streamline tractography
*/
int main(int argc, char* argv[])
{
    MITK_INFO << "DFTraining";
    mitkCommandLineParser parser;

    parser.setTitle("Training for Machine Learning Based Streamline Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Train random forest classifier for machine learning based streamline tractography");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("images", "i", mitkCommandLineParser::StringList, "DWIs:", "input diffusion-weighted images", us::Any(), false);
    parser.addArgument("tractograms", "t", mitkCommandLineParser::StringList, "Tractograms:", "input training tractograms (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("forest", "f", mitkCommandLineParser::OutputFile, "Forest:", "output random forest (HDF5)", us::Any(), false);

    parser.addArgument("masks", "m", mitkCommandLineParser::StringList, "Masks:", "restrict trining using a binary mask image", us::Any());
    parser.addArgument("wmmasks", "w", mitkCommandLineParser::StringList, "WM-Masks:", "if no binary white matter mask is specified, the envelope of the input tractogram is used", us::Any());

    parser.addArgument("stepsize", "s", mitkCommandLineParser::Float, "Stepsize:", "resampling parameter for the input tractogram in mm (determines number of white-matter samples)", us::Any());
    parser.addArgument("gmsamples", "g", mitkCommandLineParser::Int, "Number of gray matter samples per voxel:", "Number of gray matter samples per voxel", us::Any());
    parser.addArgument("numtrees", "n", mitkCommandLineParser::Int, "Number of trees:", "number of trees", us::Any());
    parser.addArgument("max_tree_depth", "d", mitkCommandLineParser::Int, "Max. tree depth:", "maximum tree depth", us::Any());
    parser.addArgument("sample_fraction", "sf", mitkCommandLineParser::Float, "Sample fraction:", "fraction of samples used per tree", us::Any());

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    mitkCommandLineParser::StringContainerType imageFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["images"]);
    mitkCommandLineParser::StringContainerType wmMaskFiles;
    if (parsedArgs.count("wmmasks"))
        wmMaskFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["wmmasks"]);

    mitkCommandLineParser::StringContainerType maskFiles;
    if (parsedArgs.count("masks"))
        maskFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["masks"]);

    string forestFile = us::any_cast<string>(parsedArgs["forest"]);

    mitkCommandLineParser::StringContainerType tractogramFiles;
    if (parsedArgs.count("tractograms"))
        tractogramFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["tractograms"]);

    int numTrees = 50;
    if (parsedArgs.count("numtrees"))
        numTrees = us::any_cast<int>(parsedArgs["numtrees"]);

    int gmsamples = -1;
    if (parsedArgs.count("gmsamples"))
        gmsamples = us::any_cast<int>(parsedArgs["gmsamples"]);

    float stepsize = -1;
    if (parsedArgs.count("stepsize"))
        stepsize = us::any_cast<float>(parsedArgs["stepsize"]);

    int max_tree_depth = 25;
    if (parsedArgs.count("max_tree_depth"))
        max_tree_depth = us::any_cast<int>(parsedArgs["max_tree_depth"]);

    double sample_fraction = 0.6;
    if (parsedArgs.count("sample_fraction"))
        sample_fraction = us::any_cast<float>(parsedArgs["sample_fraction"]);


    MITK_INFO << "loading diffusion-weighted images";
    std::vector< mitk::Image::Pointer > rawData;
    for (auto imgFile : imageFiles)
    {
        mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(imgFile).GetPointer());
        rawData.push_back(dwi);
    }

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;
    MITK_INFO << "loading mask images";
    std::vector< ItkUcharImgType::Pointer > maskImageVector;
    for (auto maskFile : maskFiles)
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(maskFile).GetPointer());
        ItkUcharImgType::Pointer mask = ItkUcharImgType::New();
        mitk::CastToItkImage(img, mask);
        maskImageVector.push_back(mask);
    }

    MITK_INFO << "loading white matter mask images";
    std::vector< ItkUcharImgType::Pointer > wmMaskImageVector;
    for (auto wmFile : wmMaskFiles)
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(wmFile).GetPointer());
        ItkUcharImgType::Pointer wmmask = ItkUcharImgType::New();
        mitk::CastToItkImage(img, wmmask);
        wmMaskImageVector.push_back(wmmask);
    }

    MITK_INFO << "loading tractograms";
    std::vector< mitk::FiberBundle::Pointer > tractograms;
    for (auto tractFile : tractogramFiles)
    {
        mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(tractFile).at(0).GetPointer());
        tractograms.push_back(fib);
    }

    mitk::TrackingForestHandler<> forestHandler;
    forestHandler.SetRawData(rawData);
    forestHandler.SetMaskImages(maskImageVector);
    forestHandler.SetWhiteMatterImages(wmMaskImageVector);
    forestHandler.SetTractograms(tractograms);
    forestHandler.SetNumTrees(numTrees);
    forestHandler.SetMaxTreeDepth(max_tree_depth);
    forestHandler.SetGrayMatterSamplesPerVoxel(gmsamples);
    forestHandler.SetSampleFraction(sample_fraction);
    forestHandler.SetStepSize(stepsize);
    forestHandler.StartTraining();
    forestHandler.SaveForest(forestFile);

    return EXIT_SUCCESS;
}
