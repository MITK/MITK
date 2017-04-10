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
#include <omp.h>

#include <mitkFiberBundle.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingDataHandler.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerRandomForest.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerPeaks.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerOdf.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

const int numOdfSamples = 200;
typedef itk::Image< itk::Vector< float, numOdfSamples > , 3 > SampledShImageType;

/*!
\brief
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Streamline Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Perform streamline tractography");
    parser.setContributor("MIC");

    // parameters fo all methods
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::StringList, "Input:", "input image (multiple possible for 'Tensor' algorithm)", us::Any(), false);
    parser.addArgument("algorithm", "a", mitkCommandLineParser::String, "Algorithm:", "which algorithm to use (Peaks, Tensor, DetODF, ProbODF, RandomForest)", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output fiberbundle", us::Any(), false);

    parser.addArgument("stop_mask", "", mitkCommandLineParser::String, "Stop image:", "streamlines entering the binary mask will stop immediately", us::Any());
    parser.addArgument("tracking_mask", "", mitkCommandLineParser::String, "Mask image:", "restrict tractography with a binary mask image", us::Any());
    parser.addArgument("seed_mask", "", mitkCommandLineParser::String, "Seed image:", "binary mask image defining seed voxels", us::Any());
    parser.addArgument("5TT_image", "", mitkCommandLineParser::String, "Tissue type image:", "image with tissue type labels (WM=3, GM=1)", us::Any());

    parser.addArgument("cutoff", "", mitkCommandLineParser::Float, "Cutoff:", "set the FA, GFA or Peak amplitude cutoff for terminating tracks", 0.1);
    parser.addArgument("step_size", "", mitkCommandLineParser::Float, "Step size:", "step size (in voxels)", 0.5);
    parser.addArgument("angular_threshold", "", mitkCommandLineParser::Float, "Angular threshold:", "angular threshold between two successive steps, (default: 90° * step_size)");
    parser.addArgument("seeds", "", mitkCommandLineParser::Int, "Seeds per voxel:", "number of seed points per voxel", 1);
    parser.addArgument("seed_gm", "", mitkCommandLineParser::Bool, "Seed only GM:", "Seed only in gray matter (requires tissue type image -t)", true);
    parser.addArgument("max_tracts", "", mitkCommandLineParser::Int, "Max. number of tracts:", "tractography is stopped if the reconstructed number of tracts is exceeded.", -1);

    parser.addArgument("num_samples", "", mitkCommandLineParser::Int, "Num. neighborhood samples:", "number of neighborhood samples that are use to determine the next progression direction", 0);
    parser.addArgument("sampling_distance", "", mitkCommandLineParser::Float, "Sampling distance:", "distance of neighborhood sampling points (in voxels)", 0.25);
    parser.addArgument("use_stop_votes", "", mitkCommandLineParser::Bool, "Use stop votes:", "use stop votes", true);
    parser.addArgument("use_only_forward_samples", "", mitkCommandLineParser::Bool, "Use only forward samples:", "use only forward samples");

    parser.addArgument("use_trilinear_interpolation", "", mitkCommandLineParser::Bool, "Use trilinear interpolation:", "use trilinear interpolation", true);
    parser.addArgument("flip_x", "", mitkCommandLineParser::Bool, "Flip X:", "multiply x-coordinate of direction proposal by -1", false);
    parser.addArgument("flip_y", "", mitkCommandLineParser::Bool, "Flip Y:", "multiply y-coordinate of direction proposal by -1", false);
    parser.addArgument("flip_z", "", mitkCommandLineParser::Bool, "Flip Z:", "multiply z-coordinate of direction proposal by -1", false);

    parser.addArgument("compress", "", mitkCommandLineParser::Float, "Compress:", "Compress output fibers using the given error threshold (in mm)");
    parser.addArgument("additional_images", "", mitkCommandLineParser::StringList, "Additional images:", "specify a list of float images that hold additional information (FA, GFA, additional Features)", us::Any());

    // parameters for ODF based tractography
    parser.addArgument("normalize_odfs", "", mitkCommandLineParser::Bool, "Min-max normalize ODFs:", "Voxel-wise min-max normalization of ODFs", true);

    // parameters for random forest based tractography
    parser.addArgument("forest", "", mitkCommandLineParser::String, "Forest:", "input random forest (HDF5 file)", us::Any());
    parser.addArgument("use_sh_features", "", mitkCommandLineParser::Bool, "Use SH features:", "use SH features", false);

    // parameters for tensor tractography
    parser.addArgument("tend_f", "", mitkCommandLineParser::Float, "Weight f", "Weighting factor between first eigenvector (f=1 equals FACT tracking) and input vector dependent direction (f=0).", 1.0);
    parser.addArgument("tend_g", "", mitkCommandLineParser::Float, "Weight g", "Weighting factor between input vector (g=0) and tensor deflection (g=1 equals TEND tracking)", 0.0);


    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    mitkCommandLineParser::StringContainerType input_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input"]);
    string outFile = us::any_cast<string>(parsedArgs["out"]);
    string algorithm = us::any_cast<string>(parsedArgs["algorithm"]);

    bool interpolate = true;
    if (parsedArgs.count("use_trilinear_interpolation"))
        interpolate = us::any_cast<bool>(parsedArgs["use_trilinear_interpolation"]);

    bool use_sh_features = false;
    if (parsedArgs.count("use_sh_features"))
        use_sh_features = us::any_cast<bool>(parsedArgs["use_sh_features"]);

    bool seed_gm = false;
    if (parsedArgs.count("seed_gm"))
        seed_gm = us::any_cast<bool>(parsedArgs["seed_gm"]);

    bool use_stop_votes = true;
    if (parsedArgs.count("use_stop_votes"))
        use_stop_votes = us::any_cast<bool>(parsedArgs["use_stop_votes"]);

    bool use_only_forward_samples = true;
    if (parsedArgs.count("use_only_forward_samples"))
        use_only_forward_samples = us::any_cast<bool>(parsedArgs["use_only_forward_samples"]);

    bool flip_x = false;
    if (parsedArgs.count("flip_x"))
        flip_x = us::any_cast<bool>(parsedArgs["flip_x"]);
    bool flip_y = false;
    if (parsedArgs.count("flip_y"))
        flip_y = us::any_cast<bool>(parsedArgs["flip_y"]);
    bool flip_z = false;
    if (parsedArgs.count("flip_z"))
        flip_z = us::any_cast<bool>(parsedArgs["flip_z"]);

    float compress = -1;
    if (parsedArgs.count("compress"))
        compress = us::any_cast<float>(parsedArgs["compress"]);

    string forestFile;
    if (parsedArgs.count("forest"))
        forestFile = us::any_cast<string>(parsedArgs["forest"]);

    string maskFile = "";
    if (parsedArgs.count("tracking_mask"))
        maskFile = us::any_cast<string>(parsedArgs["tracking_mask"]);

    string seedFile = "";
    if (parsedArgs.count("seed_mask"))
        seedFile = us::any_cast<string>(parsedArgs["seed_mask"]);

    string stopFile = "";
    if (parsedArgs.count("stop_mask"))
        stopFile = us::any_cast<string>(parsedArgs["stop_mask"]);

    string tissueFile = "";
    if (parsedArgs.count("5TT_image"))
        tissueFile = us::any_cast<string>(parsedArgs["5TT_image"]);

    float cutoff = 0.1;
    if (parsedArgs.count("cutoff"))
        cutoff = us::any_cast<float>(parsedArgs["cutoff"]);

    float stepsize = -1;
    if (parsedArgs.count("step_size"))
        stepsize = us::any_cast<float>(parsedArgs["step_size"]);

    float sampling_distance = -1;
    if (parsedArgs.count("sampling_distance"))
        sampling_distance = us::any_cast<float>(parsedArgs["sampling_distance"]);

    int num_samples = 0;
    if (parsedArgs.count("num_samples"))
        num_samples = us::any_cast<int>(parsedArgs["num_samples"]);

    int seeds = 1;
    if (parsedArgs.count("seeds"))
        seeds = us::any_cast<int>(parsedArgs["seeds"]);

    float tend_f = 1;
    if (parsedArgs.count("tend_f"))
        tend_f = us::any_cast<float>(parsedArgs["tend_f"]);

    float tend_g = 0;
    if (parsedArgs.count("tend_g"))
        tend_g = us::any_cast<float>(parsedArgs["tend_g"]);

    float angular_threshold = 0;
    if (parsedArgs.count("angular_threshold"))
        angular_threshold = us::any_cast<float>(parsedArgs["angular_threshold"]);

    bool normalize_odfs = true;
    if (parsedArgs.count("normalize_odfs"))
        normalize_odfs = us::any_cast<bool>(parsedArgs["normalize_odfs"]);

    unsigned int max_tracts = -1;
    if (parsedArgs.count("max_tracts"))
        max_tracts = us::any_cast<int>(parsedArgs["max_tracts"]);


    // LOAD DATASETS

    mitkCommandLineParser::StringContainerType addFiles;
    if (parsedArgs.count("additional_images"))
        addFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["additional_images"]);

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;

    MITK_INFO << "loading input";
    std::vector< mitk::Image::Pointer > input_images;
    for (unsigned int i=0; i<input_files.size(); i++)
    {
        mitk::Image::Pointer mitkImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(input_files.at(i))->GetData());
        input_images.push_back(mitkImage);
    }

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


    ItkUcharImgType::Pointer tissue;
    if (!tissueFile.empty())
    {
        MITK_INFO << "loading tissue image";
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(tissueFile).GetPointer());
        tissue = ItkUcharImgType::New();
        mitk::CastToItkImage(img, tissue);
    }

    MITK_INFO << "loading additional images";
    typedef itk::Image<float, 3> ItkFloatImgType;
    std::vector< std::vector< ItkFloatImgType::Pointer > > addImages;
    addImages.push_back(std::vector< ItkFloatImgType::Pointer >());
    for (auto file : addFiles)
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(file).GetPointer());
        ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
        mitk::CastToItkImage(img, itkimg);
        addImages.at(0).push_back(itkimg);
    }

//    //////////////////////////////////////////////////////////////////
//    omp_set_num_threads(1);

    typedef itk::StreamlineTrackingFilter TrackerType;
    TrackerType::Pointer tracker = TrackerType::New();

    mitk::TrackingDataHandler* handler;
    if (algorithm == "RandomForest")
    {
        if (use_sh_features)
        {
            handler = new mitk::TrackingHandlerRandomForest<6,28>();
            dynamic_cast<mitk::TrackingHandlerRandomForest<6,28>*>(handler)->LoadForest(forestFile);
            dynamic_cast<mitk::TrackingHandlerRandomForest<6,28>*>(handler)->AddDwi(input_images.at(0));
            dynamic_cast<mitk::TrackingHandlerRandomForest<6,28>*>(handler)->SetAdditionalFeatureImages(addImages);
        }
        else
        {
            handler = new mitk::TrackingHandlerRandomForest<6,100>();
            dynamic_cast<mitk::TrackingHandlerRandomForest<6,100>*>(handler)->LoadForest(forestFile);
            dynamic_cast<mitk::TrackingHandlerRandomForest<6,100>*>(handler)->AddDwi(input_images.at(0));
            dynamic_cast<mitk::TrackingHandlerRandomForest<6,100>*>(handler)->SetAdditionalFeatureImages(addImages);
        }
    }
    else if (algorithm == "Peaks")
    {
        handler = new mitk::TrackingHandlerPeaks();

        typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(input_images.at(0));
        caster->Update();
        mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();

        dynamic_cast<mitk::TrackingHandlerPeaks*>(handler)->SetPeakImage(itkImg);
        dynamic_cast<mitk::TrackingHandlerPeaks*>(handler)->SetPeakThreshold(cutoff);
    }
    else if (algorithm == "Tensor")
    {
        handler = new mitk::TrackingHandlerTensor();

        for (auto input_image : input_images)
        {
            typedef mitk::ImageToItk< mitk::TrackingHandlerTensor::ItkTensorImageType > CasterType;
            CasterType::Pointer caster = CasterType::New();
            caster->SetInput(input_image);
            caster->Update();
            mitk::TrackingHandlerTensor::ItkTensorImageType::Pointer itkImg = caster->GetOutput();
            dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->AddTensorImage(itkImg);
        }

        dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetFaThreshold(cutoff);
        dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetF(tend_f);
        dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetG(tend_g);

        if (addImages.size()>0)
            dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetFaImage(addImages.at(0).at(0));
    }
    else if (algorithm == "DetODF" || algorithm == "ProbODF")
    {
        handler = new mitk::TrackingHandlerOdf();

        for (auto input_image : input_images)
        {
            typedef mitk::ImageToItk< mitk::TrackingHandlerOdf::ItkOdfImageType > CasterType;
            CasterType::Pointer caster = CasterType::New();
            caster->SetInput(input_image);
            caster->Update();
            mitk::TrackingHandlerOdf::ItkOdfImageType::Pointer itkImg = caster->GetOutput();
            dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetOdfImage(itkImg);
        }

        dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetGfaThreshold(cutoff);
        if (algorithm == "ProbODF")
            dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetMode(mitk::TrackingHandlerOdf::MODE::PROBABILISTIC);

        if (addImages.size()>0)
            dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetGfaImage(addImages.at(0).at(0));

        dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->setMinMaxNormalize(normalize_odfs);
    }
    handler->SetInterpolate(interpolate);
    handler->SetFlipX(flip_x);
    handler->SetFlipY(flip_y);
    handler->SetFlipZ(flip_z);

    MITK_INFO << "Tractography algorithm: " << algorithm;

    tracker->SetNumberOfSamples(num_samples);
    tracker->SetAngularThreshold(angular_threshold);
    tracker->SetMaskImage(mask);
    tracker->SetSeedImage(seed);
    tracker->SetStoppingRegions(stop);
    tracker->SetSeedsPerVoxel(seeds);
    tracker->SetStepSize(stepsize);
    tracker->SetSamplingDistance(sampling_distance);
    tracker->SetUseStopVotes(use_stop_votes);
    tracker->SetOnlyForwardSamples(use_only_forward_samples);
    tracker->SetAposterioriCurvCheck(false);
    tracker->SetFourTTImage(tissue);
    tracker->SetSeedOnlyGm(seed_gm);
    tracker->SetMaxNumTracts(max_tracts);
    tracker->SetTrackingHandler(handler);
    tracker->Update();
    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    if (compress>0)
        outFib->Compress(compress);

    mitk::IOUtil::Save(outFib, outFile);

    delete handler;

    return EXIT_SUCCESS;
}
