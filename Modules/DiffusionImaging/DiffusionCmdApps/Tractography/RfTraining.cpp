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
#include <mitkIOUtil.h>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkFiberBundle.h>
#include <mitkTrackingHandlerRandomForest.h>
#include <mitkTractographyForest.h>

#define _USE_MATH_DEFINES
#include <math.h>

/*!
\brief Train random forest classifier for machine learning based streamline tractography
*/
int main(int argc, char* argv[])
{
    MITK_INFO << "RfTraining";
    mitkCommandLineParser parser;

    parser.setTitle("Trains Random Forests for Machine Learning Based Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Train random forest classifier for machine learning based streamline tractography");
    parser.setContributor("MIC");

    parser.setArgumentPrefix("--", "-");

    parser.beginGroup("1. Mandatory arguments:");
    parser.addArgument("", "i", mitkCommandLineParser::StringList, "DWIs:", "input diffusion-weighted images", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("", "t", mitkCommandLineParser::StringList, "Tractograms:", "input training tractograms", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("", "o", mitkCommandLineParser::String, "Forest:", "output random forest (HDF5)", us::Any(), false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("2. Additional input images:");
    parser.addArgument("masks", "", mitkCommandLineParser::StringList, "Masks:", "restrict training using a binary mask image", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument("wm_masks", "", mitkCommandLineParser::StringList, "WM-Masks:", "if no binary white matter mask is specified, the envelope of the input tractogram is used", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument("volume_modification_images", "", mitkCommandLineParser::StringList, "Volume modification images:", "specify a list of float images that modify the fiber density", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument("additional_feature_images", "", mitkCommandLineParser::StringList, "Additional feature images:", "specify a list of float images that hold additional features (float)", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.endGroup();

    parser.beginGroup("3. Forest parameters:");
    parser.addArgument("num_trees", "", mitkCommandLineParser::Int, "Number of trees:", "number of trees", 30);
    parser.addArgument("max_tree_depth", "", mitkCommandLineParser::Int, "Max. tree depth:", "maximum tree depth", 25);
    parser.addArgument("sample_fraction", "", mitkCommandLineParser::Float, "Sample fraction:", "fraction of samples used per tree", 0.7);
    parser.endGroup();

    parser.beginGroup("4. Feature parameters:");
    parser.addArgument("use_sh_features", "", mitkCommandLineParser::Bool, "Use SH features:", "use SH features", false);
    parser.addArgument("sampling_distance", "", mitkCommandLineParser::Float, "Sampling distance:", "resampling parameter for the input tractogram in mm (determines number of white-matter samples)", us::Any());
    parser.addArgument("max_wm_samples", "", mitkCommandLineParser::Int, "Max. num. WM samples:", "upper limit for the number of WM samples");
    parser.addArgument("num_gm_samples", "", mitkCommandLineParser::Int, "Number of gray matter samples per voxel:", "Number of gray matter samples per voxel", us::Any());
    parser.endGroup();

    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    bool shfeatures = false;
    if (parsedArgs.count("use_sh_features"))
        shfeatures = us::any_cast<bool>(parsedArgs["use_sh_features"]);

    mitkCommandLineParser::StringContainerType imageFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i"]);
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

    std::string forestFile = us::any_cast<std::string>(parsedArgs["o"]);

    mitkCommandLineParser::StringContainerType tractogramFiles;
    if (parsedArgs.count("t"))
        tractogramFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["t"]);

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
        auto dwi = mitk::IOUtil::Load<mitk::Image>(imgFile, &functor);
        rawData.push_back(dwi);
    }

    typedef itk::Image<float, 3> ItkFloatImgType;
    typedef itk::Image<unsigned char, 3> ItkUcharImgType;
    MITK_INFO << "loading mask images";
    std::vector< ItkUcharImgType::Pointer > maskImageVector;
    for (auto maskFile : maskFiles)
    {
        mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(maskFile);
        ItkUcharImgType::Pointer mask = ItkUcharImgType::New();
        mitk::CastToItkImage(img, mask);
        maskImageVector.push_back(mask);
    }

    MITK_INFO << "loading white matter mask images";
    std::vector< ItkUcharImgType::Pointer > wmMaskImageVector;
    for (auto wmFile : wmMaskFiles)
    {
        mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(wmFile);
        ItkUcharImgType::Pointer wmmask = ItkUcharImgType::New();
        mitk::CastToItkImage(img, wmmask);
        wmMaskImageVector.push_back(wmmask);
    }

    MITK_INFO << "loading tractograms";
    std::vector< mitk::FiberBundle::Pointer > tractograms;
    for (auto tractFile : tractogramFiles)
    {
        mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(tractFile);
        tractograms.push_back(fib);
    }

    MITK_INFO << "loading white volume modification images";
    std::vector< ItkFloatImgType::Pointer > volumeModImages;
    for (auto file : volModFiles)
    {
        mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(file);
        ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
        mitk::CastToItkImage(img, itkimg);
        volumeModImages.push_back(itkimg);
    }

    MITK_INFO << "loading additional feature images";
    std::vector< std::vector< ItkFloatImgType::Pointer > > addFeatImages;
    for (std::size_t i=0; i<rawData.size(); ++i)
        addFeatImages.push_back(std::vector< ItkFloatImgType::Pointer >());

    int c = 0;
    for (auto file : addFeatFiles)
    {
        mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(file);
        ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
        mitk::CastToItkImage(img, itkimg);
        addFeatImages.at(c%addFeatImages.size()).push_back(itkimg);
        c++;
    }

    mitk::TractographyForest::Pointer forest = nullptr;
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
        forestHandler.SetFiberSamplingStep(sampling_distance);
        forestHandler.SetMaxNumWmSamples(maxWmSamples);
        forestHandler.StartTraining();
        forest = forestHandler.GetForest();
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
        forestHandler.SetFiberSamplingStep(sampling_distance);
        forestHandler.SetMaxNumWmSamples(maxWmSamples);
        forestHandler.StartTraining();
        forest = forestHandler.GetForest();
    }

    mitk::IOUtil::Save(forest, forestFile);

    return EXIT_SUCCESS;
}
