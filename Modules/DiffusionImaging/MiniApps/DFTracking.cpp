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
#include <mitkCommandLineParser.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>

#include <mitkFiberBundle.h>
#include <itkMLBSTrackingFilter.h>
#include <mitkTrackingForestHandler.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

const int numOdfSamples = 200;
typedef itk::Image< itk::Vector< float, numOdfSamples > , 3 > SampledShImageType;

/*!
\brief Perform machine learning based streamline tractography
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Machine Learning Based Streamline Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Perform machine learning based streamline tractography");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("image", "i", mitkCommandLineParser::String, "DWI:", "input diffusion-weighted image", us::Any(), false);
    parser.addArgument("addfeatures", "a", mitkCommandLineParser::StringList, "Additional feature images:", "specify a list of float images that hold additional features (float)", us::Any());
    parser.addArgument("forest", "f", mitkCommandLineParser::String, "Forest:", "input random forest (HDF5 file)", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output fiberbundle", us::Any(), false);

    parser.addArgument("stop", "st", mitkCommandLineParser::String, "Stop image:", "streamlines entering the binary mask will stop immediately", us::Any());
    parser.addArgument("mask", "m", mitkCommandLineParser::String, "Mask image:", "restrict tractography with a binary mask image", us::Any());
    parser.addArgument("seed", "s", mitkCommandLineParser::String, "Seed image:", "binary mask image defining seed voxels", us::Any());

    parser.addArgument("stepsize", "se", mitkCommandLineParser::Float, "Stepsize:", "stepsize (in voxels)", us::Any());
    parser.addArgument("samplingdist", "sd", mitkCommandLineParser::Float, "Sampling distance:", "distance of neighborhood sampling points (in voxels)", us::Any());
    parser.addArgument("seeds", "nse", mitkCommandLineParser::Int, "Seeds per voxel:", "number of seed points per voxel", us::Any());

    parser.addArgument("stopvotes", "sv", mitkCommandLineParser::Int, "Use stop votes:", "use stop votes", us::Any());
    parser.addArgument("forward", "fs", mitkCommandLineParser::Int, "Use only forward samples:", "use only forward samples", us::Any());


    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string imageFile = us::any_cast<string>(parsedArgs["image"]);
    string forestFile = us::any_cast<string>(parsedArgs["forest"]);
    string outFile = us::any_cast<string>(parsedArgs["out"]);

    bool stopvotes = true;
    if (parsedArgs.count("stopvotes"))
        stopvotes = us::any_cast<int>(parsedArgs["stopvotes"]);

    bool forward = true;
    if (parsedArgs.count("forward"))
        forward = us::any_cast<int>(parsedArgs["forward"]);

    string maskFile = "";
    if (parsedArgs.count("mask"))
        maskFile = us::any_cast<string>(parsedArgs["mask"]);

    string seedFile = "";
    if (parsedArgs.count("seed"))
        seedFile = us::any_cast<string>(parsedArgs["seed"]);

    string stopFile = "";
    if (parsedArgs.count("stop"))
        stopFile = us::any_cast<string>(parsedArgs["stop"]);

    float stepsize = -1;
    if (parsedArgs.count("stepsize"))
        stepsize = us::any_cast<float>(parsedArgs["stepsize"]);

    float samplingdist = 0.25;
    if (parsedArgs.count("samplingdist"))
        samplingdist = us::any_cast<float>(parsedArgs["samplingdist"]);

    int seeds = 1;
    if (parsedArgs.count("seeds"))
        seeds = us::any_cast<int>(parsedArgs["seeds"]);

    mitkCommandLineParser::StringContainerType addFeatFiles;
    if (parsedArgs.count("addfeatures"))
        addFeatFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["addfeatures"]);

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;

    MITK_INFO << "loading diffusion-weighted image";
    mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(imageFile).GetPointer());

    ItkUcharImgType::Pointer mask;
    if (!maskFile.empty())
    {
        MITK_INFO << "loading mask image";
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(maskFile).GetPointer());
        mask = ItkUcharImgType::New();
        mitk::CastToItkImage(img, mask);
    }

    ItkUcharImgType::Pointer seed;
    if (!seedFile.empty())
    {
        MITK_INFO << "loading seed image";
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(seedFile).GetPointer());
        seed = ItkUcharImgType::New();
        mitk::CastToItkImage(img, seed);
    }

    ItkUcharImgType::Pointer stop;
    if (!stopFile.empty())
    {
        MITK_INFO << "loading stop image";
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(stopFile).GetPointer());
        stop = ItkUcharImgType::New();
        mitk::CastToItkImage(img, stop);
    }

    MITK_INFO << "loading additional feature images";
    typedef itk::Image<float, 3> ItkFloatImgType;
    std::vector< std::vector< ItkFloatImgType::Pointer > > addFeatImages;
    addFeatImages.push_back(std::vector< ItkFloatImgType::Pointer >());

    for (auto file : addFeatFiles)
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(file).GetPointer());
        ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
        mitk::CastToItkImage(img, itkimg);
        addFeatImages.at(0).push_back(itkimg);
    }

    mitk::TrackingForestHandler<6,100> tfh;
    tfh.LoadForest(forestFile);
    tfh.AddDwi(dwi);
    tfh.SetAdditionalFeatureImages(addFeatImages);

    typedef itk::MLBSTrackingFilter<6,100> TrackerType;
    TrackerType::Pointer tracker = TrackerType::New();
    tracker->SetInput(0, mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi));
    tracker->SetMaskImage(mask);
    tracker->SetSeedImage(seed);
    tracker->SetStoppingRegions(stop);
    tracker->SetSeedsPerVoxel(seeds);
    tracker->SetStepSize(stepsize);
    tracker->SetForestHandler(tfh);
    tracker->SetSamplingDistance(samplingdist);
    tracker->SetUseStopVotes(stopvotes);
    tracker->SetOnlyForwardSamples(forward);
    //tracker->SetDeflectionMod(deflection);
    //tracker->SetAvoidStop(false);
    tracker->SetAposterioriCurvCheck(false);
    tracker->SetRemoveWmEndFibers(false);
    tracker->Update();
    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    mitk::IOUtil::Save(outFib, outFile);

    return EXIT_SUCCESS;
}
