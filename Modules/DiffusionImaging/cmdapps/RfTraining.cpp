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
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkFiberBundle.h>
#include <mitkTrackingHandlerRandomForest.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

/*!
\brief Train random forest classifier for machine learning based streamline tractography
*/
int main(int argc, char* argv[])
{
    MITK_INFO << "RfTraining";
    mitkCommandLineParser parser;

    parser.setTitle("Training for Machine Learning Based Streamline Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Train random forest classifier for machine learning based streamline tractography");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("images", "i", mitkCommandLineParser::StringList, "DWIs:", "input diffusion-weighted images", us::Any(), false);
    parser.addArgument("tractograms", "t", mitkCommandLineParser::StringList, "Tractograms:", "input training tractograms (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("forest", "f", mitkCommandLineParser::OutputFile, "Forest:", "output random forest (HDF5)", us::Any(), false);

    parser.addArgument("masks", "", mitkCommandLineParser::StringList, "Masks:", "restrict training using a binary mask image", us::Any());
    parser.addArgument("wm_masks", "", mitkCommandLineParser::StringList, "WM-Masks:", "if no binary white matter mask is specified, the envelope of the input tractogram is used", us::Any());
    parser.addArgument("volume_modification_images", "", mitkCommandLineParser::StringList, "Volume modification images:", "specify a list of float images that modify the fiber density", us::Any());
    parser.addArgument("additional_feature_images", "", mitkCommandLineParser::StringList, "Additional feature images:", "specify a list of float images that hold additional features (float)", us::Any());

    parser.addArgument("sampling_distance", "", mitkCommandLineParser::Float, "Sampling distance:", "resampling parameter for the input tractogram in mm (determines number of white-matter samples)", us::Any());
    parser.addArgument("num_gm_samples", "", mitkCommandLineParser::Int, "Number of gray matter samples per voxel:", "Number of gray matter samples per voxel", us::Any());
    parser.addArgument("num_trees", "", mitkCommandLineParser::Int, "Number of trees:", "number of trees", 30);
    parser.addArgument("max_tree_depth", "", mitkCommandLineParser::Int, "Max. tree depth:", "maximum tree depth", 25);
    parser.addArgument("sample_fraction", "", mitkCommandLineParser::Float, "Sample fraction:", "fraction of samples used per tree", 0.7);

    parser.addArgument("use_sh_features", "", mitkCommandLineParser::Bool, "Use SH features:", "use SH features", false);
    parser.addArgument("max_wm_samples", "", mitkCommandLineParser::Int, "Max. num. WM samples:", "upper limit for the number of WM samples");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    bool shfeatures = false;
    if (parsedArgs.count("use_sh_features"))
        shfeatures = us::any_cast<bool>(parsedArgs["use_sh_features"]);

    mitkCommandLineParser::StringContainerType imageFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["images"]);
    mitkCommandLineParser::StringContainerType wmMaskFiles;
    if (parsedArgs.count("wm_masks"))
        wmMaskFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["wm_masks"]);

    mitkCommandLineParser::StringContainerType volModFiles;
    if (parsedArgs.count("volume_modification_images"))
        volModFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["volume_modification_images"]);

    mitkCommandLineParser::StringContainerType addFeatFiles;
    if (parsedArgs.count("additional_feature_images"))
        addFeatFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["additional_feature_images"]);

    mitkCommandLineParser::StringContainerType maskFiles;
    if (parsedArgs.count("masks"))
        maskFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["masks"]);

    string forestFile = us::any_cast<string>(parsedArgs["forest"]);

    mitkCommandLineParser::StringContainerType tractogramFiles;
    if (parsedArgs.count("tractograms"))
        tractogramFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["tractograms"]);

    int num_trees = 30;
    if (parsedArgs.count("num_trees"))
        num_trees = us::any_cast<int>(parsedArgs["num_trees"]);

    int gm_samples = -1;
    if (parsedArgs.count("num_gm_samples"))
        gm_samples = us::any_cast<int>(parsedArgs["num_gm_samples"]);

    float sampling_distance = -1;
    if (parsedArgs.count("sampling_distance"))
        sampling_distance = us::any_cast<float>(parsedArgs["sampling_distance"]);

    int max_tree_depth = 25;
    if (parsedArgs.count("max_tree_depth"))
        max_tree_depth = us::any_cast<int>(parsedArgs["max_tree_depth"]);

    double sample_fraction = 0.7;
    if (parsedArgs.count("sample_fraction"))
        sample_fraction = us::any_cast<float>(parsedArgs["sample_fraction"]);

    int maxWmSamples = -1;
    if (parsedArgs.count("max_wm_samples"))
        maxWmSamples = us::any_cast<int>(parsedArgs["max_wm_samples"]);


    MITK_INFO << "loading diffusion-weighted images";
    std::vector< mitk::Image::Pointer > rawData;
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
    for (auto imgFile : imageFiles)
    {
        mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(imgFile, &functor).GetPointer());
        rawData.push_back(dwi);
    }

    typedef itk::Image<float, 3> ItkFloatImgType;
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

    MITK_INFO << "loading white volume modification images";
    std::vector< ItkFloatImgType::Pointer > volumeModImages;
    for (auto file : volModFiles)
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(file).GetPointer());
        ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
        mitk::CastToItkImage(img, itkimg);
        volumeModImages.push_back(itkimg);
    }

    MITK_INFO << "loading additional feature images";
    std::vector< std::vector< ItkFloatImgType::Pointer > > addFeatImages;
    for (int i=0; i<rawData.size(); i++)
        addFeatImages.push_back(std::vector< ItkFloatImgType::Pointer >());

    int c = 0;
    for (auto file : addFeatFiles)
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(file).GetPointer());
        ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
        mitk::CastToItkImage(img, itkimg);
        addFeatImages.at(c%addFeatImages.size()).push_back(itkimg);
        c++;
    }

    if (shfeatures)
    {
        mitk::TrackingHandlerRandomForest<6,28> forestHandler;
        forestHandler.SetDwis(rawData);
        forestHandler.SetMaskImages(maskImageVector);
        forestHandler.SetWhiteMatterImages(wmMaskImageVector);
        forestHandler.SetFiberVolumeModImages(volumeModImages);
        forestHandler.SetAdditionalFeatureImages(addFeatImages);
        forestHandler.SetTractograms(tractograms);
        forestHandler.SetNumTrees(num_trees);
        forestHandler.SetMaxTreeDepth(max_tree_depth);
        forestHandler.SetGrayMatterSamplesPerVoxel(gm_samples);
        forestHandler.SetSampleFraction(sample_fraction);
        forestHandler.SetStepSize(sampling_distance);
        forestHandler.SetMaxNumWmSamples(maxWmSamples);
        forestHandler.StartTraining();
        forestHandler.SaveForest(forestFile);
    }
    else
    {
        mitk::TrackingHandlerRandomForest<6,100> forestHandler;
        forestHandler.SetDwis(rawData);
        forestHandler.SetMaskImages(maskImageVector);
        forestHandler.SetWhiteMatterImages(wmMaskImageVector);
        forestHandler.SetFiberVolumeModImages(volumeModImages);
        forestHandler.SetAdditionalFeatureImages(addFeatImages);
        forestHandler.SetTractograms(tractograms);
        forestHandler.SetNumTrees(num_trees);
        forestHandler.SetMaxTreeDepth(max_tree_depth);
        forestHandler.SetGrayMatterSamplesPerVoxel(gm_samples);
        forestHandler.SetSampleFraction(sample_fraction);
        forestHandler.SetStepSize(sampling_distance);
        forestHandler.SetMaxNumWmSamples(maxWmSamples);
        forestHandler.StartTraining();
        forestHandler.SaveForest(forestFile);
    }

    return EXIT_SUCCESS;
}
