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
#include <mitkLog.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>
#include <omp.h>
#include <itksys/SystemTools.hxx>

#include <mitkFiberBundle.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingDataHandler.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerRandomForest.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerPeaks.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerOdf.h>
#include <itkTensorImageToOdfImageFilter.h>
#include <mitkTractographyForest.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

  parser.beginGroup("1. Mandatory arguments:");
  parser.addArgument("input", "i", mitkCommandLineParser::StringList, "Input:", "input image (multiple possible for 'DetTensor' algorithm)", us::Any(), false);
  parser.addArgument("algorithm", "a", mitkCommandLineParser::String, "Algorithm:", "which algorithm to use (Peaks, DetTensor, ProbTensor, DetODF, ProbODF, DetRF, ProbRF)", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output fiberbundle/probability map", us::Any(), false);
  parser.endGroup();

  parser.beginGroup("2. Seeding:");
  parser.addArgument("seeds", "", mitkCommandLineParser::Int, "Seeds per voxel:", "number of seed points per voxel", 1);
  parser.addArgument("seed_image", "", mitkCommandLineParser::String, "Seed image:", "mask image defining seed voxels", us::Any());
  parser.addArgument("trials_per_seed", "", mitkCommandLineParser::Int, "Max. trials per seed:", "try each seed N times until a valid streamline is obtained (only for probabilistic tractography)", 10);
  parser.addArgument("max_tracts", "", mitkCommandLineParser::Int, "Max. number of tracts:", "tractography is stopped if the reconstructed number of tracts is exceeded", -1);
  parser.endGroup();

  parser.beginGroup("3. Tractography constraints:");
  parser.addArgument("tracking_mask", "", mitkCommandLineParser::String, "Mask image:", "streamlines leaving the mask will stop immediately", us::Any());
  parser.addArgument("stop_image", "", mitkCommandLineParser::String, "Stop ROI image:", "streamlines entering the mask will stop immediately", us::Any());
  parser.addArgument("exclusion_image", "", mitkCommandLineParser::String, "Exclusion ROI image:", "streamlines entering the mask will be discarded", us::Any());
  parser.addArgument("ep_constraint", "", mitkCommandLineParser::String, "Endpoint constraint:", "determines which fibers are accepted based on their endpoint location - options are NONE, EPS_IN_TARGET, EPS_IN_TARGET_LABELDIFF, EPS_IN_SEED_AND_TARGET, MIN_ONE_EP_IN_TARGET, ONE_EP_IN_TARGET and NO_EP_IN_TARGET", us::Any());
  parser.addArgument("target_image", "", mitkCommandLineParser::String, "Target ROI image:", "effact depends on the chosen endpoint constraint (option ep_constraint)", us::Any());
  parser.endGroup();

  parser.beginGroup("4. Streamline integration parameters:");
  parser.addArgument("sharpen_odfs", "", mitkCommandLineParser::Bool, "SHarpen ODFs:", "if you are using dODF images as input, it is advisable to sharpen the ODFs (min-max normalize and raise to the power of 4). this is not necessary for CSD fODFs, since they are narurally much sharper.");
  parser.addArgument("cutoff", "", mitkCommandLineParser::Float, "Cutoff:", "set the FA, GFA or Peak amplitude cutoff for terminating tracks", 0.1);
  parser.addArgument("odf_cutoff", "", mitkCommandLineParser::Float, "ODF Cutoff:", "threshold on the ODF magnitude. this is useful in case of CSD fODF tractography.", 0.0);
  parser.addArgument("step_size", "", mitkCommandLineParser::Float, "Step size:", "step size (in voxels)", 0.5);
  parser.addArgument("min_tract_length", "", mitkCommandLineParser::Float, "Min. tract length:", "minimum fiber length (in mm)", 20);
  parser.addArgument("angular_threshold", "", mitkCommandLineParser::Float, "Angular threshold:", "angular threshold between two successive steps, (default: 90° * step_size, minimum 15°)");
  parser.addArgument("loop_check", "", mitkCommandLineParser::Float, "Check for loops:", "threshold on angular stdev over the last 4 voxel lengths");
  parser.endGroup();

  parser.beginGroup("5. Tractography prior:");
  parser.addArgument("prior_image", "", mitkCommandLineParser::String, "Peak prior:", "tractography prior in thr for of a peak image", us::Any());
  parser.addArgument("prior_weight", "", mitkCommandLineParser::Float, "Prior weight", "weighting factor between prior and data.", 0.5);
  parser.addArgument("restrict_to_prior", "", mitkCommandLineParser::Bool, "Restrict to prior:", "restrict tractography to regions where the prior is valid.");
  parser.addArgument("new_directions_from_prior", "", mitkCommandLineParser::Bool, "New directios from prior:", "the prior can create directions where there are none in the data.");
  parser.endGroup();

  parser.beginGroup("6. Neighborhood sampling:");
  parser.addArgument("num_samples", "", mitkCommandLineParser::Int, "Num. neighborhood samples:", "number of neighborhood samples that are use to determine the next progression direction", 0);
  parser.addArgument("sampling_distance", "", mitkCommandLineParser::Float, "Sampling distance:", "distance of neighborhood sampling points (in voxels)", 0.25);
  parser.addArgument("use_stop_votes", "", mitkCommandLineParser::Bool, "Use stop votes:", "use stop votes");
  parser.addArgument("use_only_forward_samples", "", mitkCommandLineParser::Bool, "Use only forward samples:", "use only forward samples");
  parser.endGroup();

  parser.beginGroup("7. Tensor tractography specific:");
  parser.addArgument("tend_f", "", mitkCommandLineParser::Float, "Weight f", "weighting factor between first eigenvector (f=1 equals FACT tracking) and input vector dependent direction (f=0).", 1.0);
  parser.addArgument("tend_g", "", mitkCommandLineParser::Float, "Weight g", "weighting factor between input vector (g=0) and tensor deflection (g=1 equals TEND tracking)", 0.0);
  parser.endGroup();

  parser.beginGroup("8. Random forest tractography specific:");
  parser.addArgument("forest", "", mitkCommandLineParser::String, "Forest:", "input random forest (HDF5 file)", us::Any());
  parser.addArgument("use_sh_features", "", mitkCommandLineParser::Bool, "Use SH features:", "use SH features");
  parser.endGroup();

  parser.beginGroup("9. Additional input:");
  parser.addArgument("additional_images", "", mitkCommandLineParser::StringList, "Additional images:", "specify a list of float images that hold additional information (FA, GFA, additional features for RF tractography)", us::Any());
  parser.endGroup();

  parser.beginGroup("10. Misc:");
  parser.addArgument("flip_x", "", mitkCommandLineParser::Bool, "Flip X:", "multiply x-coordinate of direction proposal by -1");
  parser.addArgument("flip_y", "", mitkCommandLineParser::Bool, "Flip Y:", "multiply y-coordinate of direction proposal by -1");
  parser.addArgument("flip_z", "", mitkCommandLineParser::Bool, "Flip Z:", "multiply z-coordinate of direction proposal by -1");
  parser.addArgument("no_data_interpolation", "", mitkCommandLineParser::Bool, "Don't interpolate input data:", "don't interpolate input image values");
  parser.addArgument("no_mask_interpolation", "", mitkCommandLineParser::Bool, "Don't interpolate masks:", "don't interpolate mask image values");
  parser.addArgument("compress", "", mitkCommandLineParser::Float, "Compress:", "compress output fibers using the given error threshold (in mm)");
  parser.endGroup();

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType input_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input"]);
  std::string outFile = us::any_cast<std::string>(parsedArgs["out"]);
  std::string algorithm = us::any_cast<std::string>(parsedArgs["algorithm"]);

  std::string prior_image = "";
  if (parsedArgs.count("prior_image"))
    prior_image = us::any_cast<std::string>(parsedArgs["prior_image"]);

  float prior_weight = 0.5;
  if (parsedArgs.count("prior_weight"))
    prior_weight = us::any_cast<float>(parsedArgs["prior_weight"]);

  bool restrict_to_prior = false;
  if (parsedArgs.count("restrict_to_prior"))
    restrict_to_prior = us::any_cast<bool>(parsedArgs["restrict_to_prior"]);

  bool new_directions_from_prior = false;
  if (parsedArgs.count("new_directions_from_prior"))
    new_directions_from_prior = us::any_cast<bool>(parsedArgs["new_directions_from_prior"]);

  bool sharpen_odfs = false;
  if (parsedArgs.count("sharpen_odfs"))
    sharpen_odfs = us::any_cast<bool>(parsedArgs["sharpen_odfs"]);

  bool interpolate = true;
  if (parsedArgs.count("no_data_interpolation"))
    interpolate = !us::any_cast<bool>(parsedArgs["no_data_interpolation"]);

  bool mask_interpolation = true;
  if (parsedArgs.count("no_mask_interpolation"))
    interpolate = !us::any_cast<bool>(parsedArgs["no_mask_interpolation"]);

  bool use_sh_features = false;
  if (parsedArgs.count("use_sh_features"))
    use_sh_features = us::any_cast<bool>(parsedArgs["use_sh_features"]);

  bool use_stop_votes = false;
  if (parsedArgs.count("use_stop_votes"))
    use_stop_votes = us::any_cast<bool>(parsedArgs["use_stop_votes"]);

  bool use_only_forward_samples = false;
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

  bool apply_image_rotation = false;
  if (parsedArgs.count("apply_image_rotation"))
    apply_image_rotation = us::any_cast<bool>(parsedArgs["apply_image_rotation"]);

  float compress = -1;
  if (parsedArgs.count("compress"))
    compress = us::any_cast<float>(parsedArgs["compress"]);

  float min_tract_length = 20;
  if (parsedArgs.count("min_tract_length"))
    min_tract_length = us::any_cast<float>(parsedArgs["min_tract_length"]);

  float loop_check = -1;
  if (parsedArgs.count("loop_check"))
    loop_check = us::any_cast<float>(parsedArgs["loop_check"]);

  std::string forestFile;
  if (parsedArgs.count("forest"))
    forestFile = us::any_cast<std::string>(parsedArgs["forest"]);

  std::string maskFile = "";
  if (parsedArgs.count("tracking_mask"))
    maskFile = us::any_cast<std::string>(parsedArgs["tracking_mask"]);

  std::string seedFile = "";
  if (parsedArgs.count("seed_image"))
    seedFile = us::any_cast<std::string>(parsedArgs["seed_image"]);

  std::string targetFile = "";
  if (parsedArgs.count("target_image"))
    targetFile = us::any_cast<std::string>(parsedArgs["target_image"]);

  std::string exclusionFile = "";
  if (parsedArgs.count("exclusion_image"))
    exclusionFile = us::any_cast<std::string>(parsedArgs["exclusion_image"]);

  std::string stopFile = "";
  if (parsedArgs.count("stop_image"))
    stopFile = us::any_cast<std::string>(parsedArgs["stop_image"]);

  std::string ep_constraint = "NONE";
  if (parsedArgs.count("ep_constraint"))
    ep_constraint = us::any_cast<std::string>(parsedArgs["ep_constraint"]);

  float cutoff = 0.1;
  if (parsedArgs.count("cutoff"))
    cutoff = us::any_cast<float>(parsedArgs["cutoff"]);

  float odf_cutoff = 0.0;
  if (parsedArgs.count("odf_cutoff"))
    odf_cutoff = us::any_cast<float>(parsedArgs["odf_cutoff"]);

  float stepsize = -1;
  if (parsedArgs.count("step_size"))
    stepsize = us::any_cast<float>(parsedArgs["step_size"]);

  float sampling_distance = -1;
  if (parsedArgs.count("sampling_distance"))
    sampling_distance = us::any_cast<float>(parsedArgs["sampling_distance"]);

  int num_samples = 0;
  if (parsedArgs.count("num_samples"))
    num_samples = us::any_cast<int>(parsedArgs["num_samples"]);

  int num_seeds = 1;
  if (parsedArgs.count("seeds"))
    num_seeds = us::any_cast<int>(parsedArgs["seeds"]);

  unsigned int trials_per_seed = 10;
  if (parsedArgs.count("trials_per_seed"))
    trials_per_seed = us::any_cast<int>(parsedArgs["trials_per_seed"]);

  float tend_f = 1;
  if (parsedArgs.count("tend_f"))
    tend_f = us::any_cast<float>(parsedArgs["tend_f"]);

  float tend_g = 0;
  if (parsedArgs.count("tend_g"))
    tend_g = us::any_cast<float>(parsedArgs["tend_g"]);

  float angular_threshold = -1;
  if (parsedArgs.count("angular_threshold"))
    angular_threshold = us::any_cast<float>(parsedArgs["angular_threshold"]);

  unsigned int max_tracts = -1;
  if (parsedArgs.count("max_tracts"))
    max_tracts = us::any_cast<int>(parsedArgs["max_tracts"]);


  std::string ext = itksys::SystemTools::GetFilenameExtension(outFile);
  if (ext != ".fib" && ext != ".trk")
  {
    MITK_INFO << "Output file format not supported. Use one of .fib, .trk, .nii, .nii.gz, .nrrd";
    return EXIT_FAILURE;
  }

  // LOAD DATASETS
  mitkCommandLineParser::StringContainerType addFiles;
  if (parsedArgs.count("additional_images"))
    addFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["additional_images"]);

  typedef itk::Image<float, 3> ItkFloatImgType;

  MITK_INFO << "loading input";
  std::vector< mitk::Image::Pointer > input_images;
  for (unsigned int i=0; i<input_files.size(); i++)
  {
    mitk::Image::Pointer mitkImage = mitk::IOUtil::Load<mitk::Image>(input_files.at(i));
    input_images.push_back(mitkImage);
  }

  ItkFloatImgType::Pointer mask = nullptr;
  if (!maskFile.empty())
  {
    MITK_INFO << "loading mask image";
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(maskFile);
    mask = ItkFloatImgType::New();
    mitk::CastToItkImage(img, mask);
  }

  ItkFloatImgType::Pointer seed = nullptr;
  if (!seedFile.empty())
  {
    MITK_INFO << "loading seed ROI image";
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(seedFile);
    seed = ItkFloatImgType::New();
    mitk::CastToItkImage(img, seed);
  }

  ItkFloatImgType::Pointer stop = nullptr;
  if (!stopFile.empty())
  {
    MITK_INFO << "loading stop ROI image";
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(stopFile);
    stop = ItkFloatImgType::New();
    mitk::CastToItkImage(img, stop);
  }

  ItkFloatImgType::Pointer target = nullptr;
  if (!targetFile.empty())
  {
    MITK_INFO << "loading target ROI image";
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(targetFile);
    target = ItkFloatImgType::New();
    mitk::CastToItkImage(img, target);
  }

  ItkFloatImgType::Pointer exclusion = nullptr;
  if (!exclusionFile.empty())
  {
    MITK_INFO << "loading exclusion ROI image";
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(exclusionFile);
    exclusion = ItkFloatImgType::New();
    mitk::CastToItkImage(img, exclusion);
  }

  MITK_INFO << "loading additional images";
  std::vector< std::vector< ItkFloatImgType::Pointer > > addImages;
  addImages.push_back(std::vector< ItkFloatImgType::Pointer >());
  for (auto file : addFiles)
  {
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(file);
    ItkFloatImgType::Pointer itkimg = ItkFloatImgType::New();
    mitk::CastToItkImage(img, itkimg);
    addImages.at(0).push_back(itkimg);
  }

  //    //////////////////////////////////////////////////////////////////
//      omp_set_num_threads(1);

  if (algorithm == "ProbTensor")
  {
    typedef mitk::ImageToItk< mitk::TrackingHandlerTensor::ItkTensorImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(input_images.at(0));
    caster->Update();
    mitk::TrackingHandlerTensor::ItkTensorImageType::Pointer itkTensorImg = caster->GetOutput();

    typedef itk::TensorImageToOdfImageFilter< float, float > FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkTensorImg );
    filter->Update();

    mitk::Image::Pointer image = mitk::Image::New();
    FilterType::OutputImageType::Pointer outimg = filter->GetOutput();
    image->InitializeByItk( outimg.GetPointer() );
    image->SetVolume( outimg->GetBufferPointer() );

    input_images.clear();
    input_images.push_back(image);

    sharpen_odfs = true;
    odf_cutoff = 0;
  }

  typedef itk::StreamlineTrackingFilter TrackerType;
  TrackerType::Pointer tracker = TrackerType::New();

  if (!prior_image.empty())
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image"}, {});
    mitk::PeakImage::Pointer priorImage = mitk::IOUtil::Load<mitk::PeakImage>(prior_image, &functor);

    if (priorImage.IsNull())
    {
      MITK_INFO << "Only peak images are supported as prior at the moment!";
      return EXIT_FAILURE;
    }

    mitk::TrackingDataHandler* priorhandler = new mitk::TrackingHandlerPeaks();

    typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(priorImage);
    caster->Update();
    mitk::TrackingHandlerPeaks::PeakImgType::Pointer itkImg = caster->GetOutput();

    dynamic_cast<mitk::TrackingHandlerPeaks*>(priorhandler)->SetPeakImage(itkImg);
    dynamic_cast<mitk::TrackingHandlerPeaks*>(priorhandler)->SetPeakThreshold(0.0);
    dynamic_cast<mitk::TrackingHandlerPeaks*>(priorhandler)->SetInterpolate(interpolate);
    dynamic_cast<mitk::TrackingHandlerPeaks*>(priorhandler)->SetMode(mitk::TrackingDataHandler::MODE::DETERMINISTIC);

    tracker->SetTrackingPriorHandler(priorhandler);
    tracker->SetTrackingPriorWeight(prior_weight);
    tracker->SetTrackingPriorAsMask(restrict_to_prior);
    tracker->SetIntroduceDirectionsFromPrior(new_directions_from_prior);
  }

  mitk::TrackingDataHandler* handler;
  if (algorithm == "DetRF" || algorithm == "ProbRF")
  {
    mitk::TractographyForest::Pointer forest = mitk::IOUtil::Load<mitk::TractographyForest>(forestFile);
    if (forest.IsNull())
      mitkThrow() << "Forest file " << forestFile << " could not be read.";

    if (use_sh_features)
    {
      handler = new mitk::TrackingHandlerRandomForest<6,28>();
      dynamic_cast<mitk::TrackingHandlerRandomForest<6,28>*>(handler)->SetForest(forest);
      dynamic_cast<mitk::TrackingHandlerRandomForest<6,28>*>(handler)->AddDwi(input_images.at(0));
      dynamic_cast<mitk::TrackingHandlerRandomForest<6,28>*>(handler)->SetAdditionalFeatureImages(addImages);
    }
    else
    {
      handler = new mitk::TrackingHandlerRandomForest<6,100>();
      dynamic_cast<mitk::TrackingHandlerRandomForest<6,100>*>(handler)->SetForest(forest);
      dynamic_cast<mitk::TrackingHandlerRandomForest<6,100>*>(handler)->AddDwi(input_images.at(0));
      dynamic_cast<mitk::TrackingHandlerRandomForest<6,100>*>(handler)->SetAdditionalFeatureImages(addImages);
    }

    if (algorithm == "ProbRF")
      handler->SetMode(mitk::TrackingDataHandler::MODE::PROBABILISTIC);
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
    dynamic_cast<mitk::TrackingHandlerPeaks*>(handler)->SetApplyDirectionMatrix(apply_image_rotation);
    dynamic_cast<mitk::TrackingHandlerPeaks*>(handler)->SetPeakThreshold(cutoff);
  }
  else if (algorithm == "DetTensor")
  {
    handler = new mitk::TrackingHandlerTensor();

    for (auto input_image : input_images)
    {
      typedef mitk::ImageToItk< mitk::TrackingHandlerTensor::ItkTensorImageType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(input_image);
      caster->Update();
      mitk::TrackingHandlerTensor::ItkTensorImageType::ConstPointer itkImg = caster->GetOutput();
      dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->AddTensorImage(itkImg);
    }

    dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetFaThreshold(cutoff);
    dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetF(tend_f);
    dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetG(tend_g);

    if (addImages.at(0).size()>0)
      dynamic_cast<mitk::TrackingHandlerTensor*>(handler)->SetFaImage(addImages.at(0).at(0));
  }
  else if (algorithm == "DetODF" || algorithm == "ProbODF" || algorithm == "ProbTensor")
  {
    handler = new mitk::TrackingHandlerOdf();

    typedef mitk::ImageToItk< mitk::TrackingHandlerOdf::ItkOdfImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(input_images.at(0));
    caster->Update();
    mitk::TrackingHandlerOdf::ItkOdfImageType::Pointer itkImg = caster->GetOutput();
    dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetOdfImage(itkImg);

    dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetGfaThreshold(cutoff);
    dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetOdfThreshold(odf_cutoff);
    dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetSharpenOdfs(sharpen_odfs);

    if (algorithm == "ProbODF" || algorithm == "ProbTensor")
      dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetMode(mitk::TrackingHandlerOdf::MODE::PROBABILISTIC);
    if (algorithm == "ProbTensor")
      dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetIsOdfFromTensor(true);

    if (addImages.at(0).size()>0)
      dynamic_cast<mitk::TrackingHandlerOdf*>(handler)->SetGfaImage(addImages.at(0).at(0));
  }
  else
  {
    MITK_INFO << "Unknown tractography algorithm (" + algorithm+"). Known types are Peaks, DetTensor, ProbTensor, DetODF, ProbODF, DetRF, ProbRF.";
    return EXIT_FAILURE;
  }
  handler->SetInterpolate(interpolate);
  handler->SetFlipX(flip_x);
  handler->SetFlipY(flip_y);
  handler->SetFlipZ(flip_z);

  if (ep_constraint=="NONE")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::NONE);
  else if (ep_constraint=="EPS_IN_TARGET")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET);
  else if (ep_constraint=="EPS_IN_TARGET_LABELDIFF")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_TARGET_LABELDIFF);
  else if (ep_constraint=="EPS_IN_SEED_AND_TARGET")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::EPS_IN_SEED_AND_TARGET);
  else if (ep_constraint=="MIN_ONE_EP_IN_TARGET")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::MIN_ONE_EP_IN_TARGET);
  else if (ep_constraint=="ONE_EP_IN_TARGET")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::ONE_EP_IN_TARGET);
  else if (ep_constraint=="NO_EP_IN_TARGET")
    tracker->SetEndpointConstraint(itk::StreamlineTrackingFilter::EndpointConstraints::NO_EP_IN_TARGET);

  MITK_INFO << "Tractography algorithm: " << algorithm;
  tracker->SetInterpolateMasks(mask_interpolation);
  tracker->SetNumberOfSamples(num_samples);
  tracker->SetAngularThreshold(angular_threshold);
  tracker->SetMaskImage(mask);
  tracker->SetSeedImage(seed);
  tracker->SetStoppingRegions(stop);
  tracker->SetTargetRegions(target);
  tracker->SetExclusionRegions(exclusion);
  tracker->SetSeedsPerVoxel(num_seeds);
  tracker->SetStepSize(stepsize);
  tracker->SetSamplingDistance(sampling_distance);
  tracker->SetUseStopVotes(use_stop_votes);
  tracker->SetOnlyForwardSamples(use_only_forward_samples);
  tracker->SetLoopCheck(loop_check);
  tracker->SetMaxNumTracts(max_tracts);
  tracker->SetTrialsPerSeed(trials_per_seed);
  tracker->SetTrackingHandler(handler);
  if (ext != ".fib" && ext != ".trk")
    tracker->SetUseOutputProbabilityMap(true);
  tracker->SetMinTractLength(min_tract_length);
  tracker->Update();

  if (ext == ".fib" || ext == ".trk")
  {
    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);
    if (compress > 0)
      outFib->Compress(compress);
    mitk::IOUtil::Save(outFib, outFile);
  }
  else
  {
    TrackerType::ItkDoubleImgType::Pointer outImg = tracker->GetOutputProbabilityMap();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    if (ext != ".nii" && ext != ".nii.gz" && ext != ".nrrd")
      outFile += ".nii.gz";

    mitk::IOUtil::Save(img, outFile);
  }

  delete handler;

  return EXIT_SUCCESS;
}
