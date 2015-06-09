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
//#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>

#include <mitkFiberBundle.h>
#include <itkDwiNormilzationFilter.h>
//#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <itkMLBSTrackingFilter.h>
#include <vtkCell.h>
#include <itkOrientationDistributionFunction.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

#define _USE_MATH_DEFINES
#include <math.h>

const int numOdfSamples = 200;
typedef itk::Image< itk::Vector< float, numOdfSamples > , 3 > SampledShImageType;

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Machine Learning Based Streamline Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("image", "i", mitkCommandLineParser::String, "DWIs:", "input diffusion-weighted image", us::Any(), false);
    parser.addArgument("forest", "f", mitkCommandLineParser::String, "Forest:", "input forest", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output fiberbundle", us::Any(), false);

    parser.addArgument("stop", "st", mitkCommandLineParser::String, "Stop image:", "stop image", us::Any());
    parser.addArgument("mask", "m", mitkCommandLineParser::String, "Mask image:", "mask image", us::Any());
    parser.addArgument("seed", "s", mitkCommandLineParser::String, "Seed image:", "seed image", us::Any());

    parser.addArgument("athres", "a", mitkCommandLineParser::Float, "Angular threshold:", "angular threshold (in radians)", us::Any());
    parser.addArgument("stepsize", "se", mitkCommandLineParser::Float, "Stepsize:", "stepsize", us::Any());
    parser.addArgument("samples", "ns", mitkCommandLineParser::Int, "Samples:", "samples", us::Any());
    parser.addArgument("samplingdist", "sd", mitkCommandLineParser::Float, "Sampling distance:", "sampling distance (in voxels)", us::Any());
    parser.addArgument("seeds", "nse", mitkCommandLineParser::Int, "Seeds per voxel:", "seeds per voxel", us::Any());

    parser.addArgument("usedirection", "ud", mitkCommandLineParser::Bool, "Use previous direction:", "use previous direction as feature", us::Any());
    parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Verbose:", "output additional images", us::Any());

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string imageFile = us::any_cast<string>(parsedArgs["image"]);
    string forestFile = us::any_cast<string>(parsedArgs["forest"]);
    string outFile = us::any_cast<string>(parsedArgs["out"]);

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

    float athres = 0.7;
    if (parsedArgs.count("athres"))
        athres = us::any_cast<float>(parsedArgs["athres"]);

    float samplingdist = 0.25;
    if (parsedArgs.count("samplingdist"))
        samplingdist = us::any_cast<float>(parsedArgs["samplingdist"]);

    bool useDirection = false;
    if (parsedArgs.count("usedirection"))
        useDirection = true;

    bool verbose = false;
    if (parsedArgs.count("verbose"))
        verbose = true;

    int samples = 10;
    if (parsedArgs.count("samples"))
        samples = us::any_cast<int>(parsedArgs["samples"]);

    int seeds = 1;
    if (parsedArgs.count("seeds"))
        seeds = us::any_cast<int>(parsedArgs["seeds"]);

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

    MITK_INFO << "loading forest";
    vigra::RandomForest<int> rf;
    vigra::rf_import_HDF5(rf, forestFile);

    typedef itk::MLBSTrackingFilter<100> TrackerType;
    TrackerType::Pointer tracker = TrackerType::New();
    tracker->SetInput(0, mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi));
    tracker->SetGradientDirections( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi) );
    tracker->SetB_Value( mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi) );
    tracker->SetMaskImage(mask);
    tracker->SetSeedImage(seed);
    tracker->SetStoppingRegions(stop);
    tracker->SetSeedsPerVoxel(seeds);
    tracker->SetUseDirection(useDirection);
    tracker->SetStepSize(stepsize);
    tracker->SetAngularThreshold(athres);
    tracker->SetDecisionForest(&rf);
    tracker->SetSamplingDistance(samplingdist);
    tracker->SetNumberOfSamples(samples);
    //tracker->SetAvoidStop(false);
    tracker->SetAposterioriCurvCheck(false);
    tracker->SetRemoveWmEndFibers(false);
    tracker->Update();
    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    mitk::IOUtil::SaveBaseData(outFib, outFile);

    if (verbose)
    {
        MITK_INFO << "Writing images...";
        string outName = itksys::SystemTools::GetFilenamePath(outFile)+"/"+itksys::SystemTools::GetFilenameWithoutLastExtension(outFile);
        itk::ImageFileWriter< TrackerType::ItkDoubleImgType >::Pointer writer = itk::ImageFileWriter< TrackerType::ItkDoubleImgType >::New();
        writer->SetFileName(outName+"_WhiteMatter.nrrd");
        writer->SetInput(tracker->GetWmImage());
        writer->Update();

        writer->SetFileName(outName+"_NotWhiteMatter.nrrd");
        writer->SetInput(tracker->GetNotWmImage());
        writer->Update();

        writer->SetFileName(outName+"_AvoidStop.nrrd");
        writer->SetInput(tracker->GetAvoidStopImage());
        writer->Update();
    }

    return EXIT_SUCCESS;
}
