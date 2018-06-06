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
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include <mitkFiberBundle.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itkImageFileWriter.h>
#include <mitkPeakImage.h>
#include <itkFitFibersToImageFilter.h>
#include <mitkLexicalCast.h>
#include <itkTractDensityImageFilter.h>

typedef itksys::SystemTools ist;
typedef itk::Point<float, 4> PointType4;
typedef itk::Image< float, 4 >  PeakImgType;
typedef itk::Image< unsigned char, 3 >  ItkUcharImageType;

std::vector< mitk::FiberBundle::Pointer > CombineTractograms(std::vector< mitk::FiberBundle::Pointer > reference, std::vector< mitk::FiberBundle::Pointer > candidates, int skip=-1)
{
  std::vector< mitk::FiberBundle::Pointer > fib;
  for (auto f : reference)
    fib.push_back(f);

  int c = 0;
  for (auto f : candidates)
  {
    if (c!=skip)
      fib.push_back(f);
    ++c;
  }

  return fib;
}

std::vector< std::string > get_file_list(const std::string& path, std::vector< std::string > extensions={".fib", ".trk"})
{
  std::vector< std::string > file_list;
  itk::Directory::Pointer dir = itk::Directory::New();

  if (dir->Load(path.c_str()))
  {
    int n = dir->GetNumberOfFiles();
    for (int r = 0; r < n; r++)
    {
      const char *filename = dir->GetFile(r);
      std::string ext = ist::GetFilenameExtension(filename);
      for (auto e : extensions)
      {
        if (ext==e)
        {
          file_list.push_back(path + '/' + filename);
          break;
        }
      }
    }
  }
  return file_list;
}

/*!
\brief Score input candidate tracts using ACP analysis
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Anchor Constrained Plausibility");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("Score input candidate tracts using ACP analysis");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "a", mitkCommandLineParser::InputFile, "Anchor tractogram:", "anchor tracts in one tractogram file", us::Any(), false);
  parser.addArgument("", "p", mitkCommandLineParser::InputFile, "Input peaks:", "input peak image", us::Any(), false);
  parser.addArgument("", "c", mitkCommandLineParser::InputDirectory, "Candidates folder:", "folder containing candidate tracts", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output folder:", "output folder", us::Any(), false);

  parser.addArgument("anchor_masks", "", mitkCommandLineParser::StringList, "Reference Masks:", "reference tract masks for accuracy evaluation");
  parser.addArgument("mask", "", mitkCommandLineParser::InputFile, "Mask image:", "scoring is only performed inside the mask image");
  parser.addArgument("greedy_add", "", mitkCommandLineParser::Bool, "Greedy:", "if enabled, the candidate tracts are not jointly fitted to the residual image but one after the other employing a greedy scheme", false);
  parser.addArgument("lambda", "", mitkCommandLineParser::Float, "Lambda:", "modifier for regularization", 0.1);
  parser.addArgument("filter_outliers", "", mitkCommandLineParser::Bool, "Filter outliers:", "perform second optimization run with an upper weight bound based on the first weight estimation (99% quantile)", false);
  parser.addArgument("regu", "", mitkCommandLineParser::String, "Regularization:", "MSM, Variance, VoxelVariance, Lasso, GroupLasso, GroupVariance, NONE (default)");
  parser.addArgument("use_num_streamlines", "", mitkCommandLineParser::Bool, "Use number of streamlines as score:", "Don't fit candidates, simply use number of streamlines per candidate as score", false);
  parser.addArgument("use_weights", "", mitkCommandLineParser::Bool, "Use input weights as score:", "Don't fit candidates, simply use first input streamline weight per candidate as score", false);
  parser.addArgument("filter_zero_weights", "", mitkCommandLineParser::Bool, "Filter zero-weights", "Remove streamlines with weight 0 from candidates", false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string anchors_file = us::any_cast<std::string>(parsedArgs["a"]);
  std::string peak_file_name = us::any_cast<std::string>(parsedArgs["p"]);
  std::string candidate_tract_folder = us::any_cast<std::string>(parsedArgs["c"]);
  std::string out_folder = us::any_cast<std::string>(parsedArgs["o"]);

  bool greedy_add = false;
  if (parsedArgs.count("greedy_add"))
    greedy_add = us::any_cast<bool>(parsedArgs["greedy_add"]);

  float lambda = 0.1;
  if (parsedArgs.count("lambda"))
    lambda = us::any_cast<float>(parsedArgs["lambda"]);

  bool filter_outliers = false;
  if (parsedArgs.count("filter_outliers"))
    filter_outliers = us::any_cast<bool>(parsedArgs["filter_outliers"]);

  bool filter_zero_weights = false;
  if (parsedArgs.count("filter_zero_weights"))
    filter_zero_weights = us::any_cast<bool>(parsedArgs["filter_zero_weights"]);

  std::string mask_file = "";
  if (parsedArgs.count("mask"))
    mask_file = us::any_cast<std::string>(parsedArgs["mask"]);

  mitkCommandLineParser::StringContainerType anchor_mask_files_folders;
  if (parsedArgs.count("anchor_masks"))
    anchor_mask_files_folders = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["anchor_masks"]);

  std::string regu = "NONE";
  if (parsedArgs.count("regu"))
    regu = us::any_cast<std::string>(parsedArgs["regu"]);

  bool use_weights = false;
  if (parsedArgs.count("use_weights"))
    use_weights = us::any_cast<bool>(parsedArgs["use_weights"]);

  bool use_num_streamlines = false;
  if (parsedArgs.count("use_num_streamlines"))
    use_num_streamlines = us::any_cast<bool>(parsedArgs["use_num_streamlines"]);

  try
  {
    itk::TimeProbe clock;
    clock.Start();

    if (!ist::PathExists(out_folder))
    {
      MITK_INFO << "Creating output directory";
      ist::MakeDirectory(out_folder);
    }

    MITK_INFO << "Loading data";
    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect

    ofstream logfile;
    logfile.open (out_folder + "log.txt");

    itk::ImageFileWriter< PeakImgType >::Pointer peak_image_writer = itk::ImageFileWriter< PeakImgType >::New();
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::PeakImage*>(mitk::IOUtil::Load(peak_file_name, &functor)[0].GetPointer());

    float minSpacing = 1;
    if(inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[1] && inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[0];
    else if (inputImage->GetGeometry()->GetSpacing()[1] < inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[1];
    else
      minSpacing = inputImage->GetGeometry()->GetSpacing()[2];

    // Load mask file. Fit is only performed inside the mask
    itk::FitFibersToImageFilter::UcharImgType::Pointer mask = nullptr;
    if (mask_file.compare("")!=0)
    {
      mitk::Image::Pointer mitk_mask = mitk::IOUtil::Load<mitk::Image>(mask_file);
      mitk::CastToItkImage(mitk_mask, mask);
    }

    // Load masks covering the true positives for evaluation purposes
    std::vector< itk::FitFibersToImageFilter::UcharImgType::Pointer > reference_masks;
    std::vector< std::string > anchor_mask_files;
    for (auto filename : anchor_mask_files_folders)
    {
      if (itksys::SystemTools::PathExists(filename))
      {
        auto list = get_file_list(filename, {".nrrd",".nii.gz",".nii"});
        for (auto f : list)
        {
          MITK_INFO << f;
          itk::FitFibersToImageFilter::UcharImgType::Pointer ref_mask = nullptr;
          mitk::Image::Pointer ref_mitk_mask = mitk::IOUtil::Load<mitk::Image>(f);
          mitk::CastToItkImage(ref_mitk_mask, ref_mask);
          reference_masks.push_back(ref_mask);
          anchor_mask_files.push_back(f);
        }
      }
      else if (itksys::SystemTools::FileExists(filename))
      {
        anchor_mask_files.push_back(filename);
        itk::FitFibersToImageFilter::UcharImgType::Pointer ref_mask = nullptr;
        mitk::Image::Pointer ref_mitk_mask = mitk::IOUtil::Load<mitk::Image>(filename);
        mitk::CastToItkImage(ref_mitk_mask, ref_mask);
        reference_masks.push_back(ref_mask);
      }
    }

    // Load peak image
    typedef mitk::ImageToItk< PeakImgType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(inputImage);
    caster->Update();
    PeakImgType::Pointer peak_image = caster->GetOutput();

    // Load all candidate tracts
    std::vector< std::string > candidate_tract_files = get_file_list(candidate_tract_folder);
    std::vector< mitk::FiberBundle::Pointer > input_candidates;
    for (std::string f : candidate_tract_files)
    {
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      if (fib.IsNull())
        continue;
      if (fib->GetNumFibers()<=0)
        continue;
      fib->ResampleLinear(minSpacing/10.0);
      input_candidates.push_back(fib);
    }
    std::cout.rdbuf (old);              // <-- restore
    MITK_INFO << "Loaded " << candidate_tract_files.size() << " candidate tracts.";
    MITK_INFO << "Loaded " << reference_masks.size() << " reference masks.";

    double rmse = 0.0;
    int iteration = 0;
    std::string name = "NOANCHOR";
    // Load reference tractogram consisting of all known tracts
    std::vector< mitk::FiberBundle::Pointer > input_reference;
    mitk::FiberBundle::Pointer anchor_tractogram = mitk::IOUtil::Load<mitk::FiberBundle>(anchors_file);
    if ( !(anchor_tractogram.IsNull() || anchor_tractogram->GetNumFibers()==0) )
    {
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      anchor_tractogram->ResampleLinear(minSpacing/10.0);
      std::cout.rdbuf (old);              // <-- restore
      input_reference.push_back(anchor_tractogram);

      // Fit known tracts to peak image to obtain underexplained image
      MITK_INFO << "Fit anchor tracts";
      itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
      fitter->SetTractograms(input_reference);
      fitter->SetLambda(lambda);
      fitter->SetFilterOutliers(filter_outliers);
      fitter->SetPeakImage(peak_image);
      fitter->SetVerbose(true);
      fitter->SetResampleFibers(false);
      fitter->SetMaskImage(mask);
      fitter->SetRegularization(VnlCostFunction::REGU::NONE);
      fitter->Update();
      rmse = fitter->GetRMSE();
      vnl_vector<double> rms_diff = fitter->GetRmsDiffPerBundle();
      logfile << "RMS_DIFF: " << setprecision(5) << rms_diff[0] << " " << name << " RMSE: " << rmse << "\n";

      name = ist::GetFilenameWithoutExtension(anchors_file);
      mitk::FiberBundle::Pointer anchor_tracts = fitter->GetTractograms().at(0);
      anchor_tracts->SetFiberColors(255,255,255);
      mitk::IOUtil::Save(anchor_tracts, out_folder + boost::lexical_cast<std::string>((int)(100000*rms_diff[0])) + "_" + name + ".fib");

      peak_image = fitter->GetUnderexplainedImage();
      peak_image_writer->SetInput(peak_image);
      peak_image_writer->SetFileName(out_folder + "Residual_" + name + ".nii.gz");
      peak_image_writer->Update();
    }

    if (use_weights || use_num_streamlines)
    {
      MITK_INFO << "Using tract weights as scores";
      int c = 0;
      for (auto fib : input_candidates)
      {
        int mod = 1;
        double score = 0;
        if (use_weights)
        {
          score = fib->GetFiberWeight(0);
          mod = 100000;
        }
        else if (use_num_streamlines)
          score = fib->GetNumFibers();
        fib->ColorFibersByOrientation();

        std::string bundle_name = ist::GetFilenameWithoutExtension(candidate_tract_files.at(c));

        std::streambuf *old = cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf (ss.rdbuf());       // <-- redirect
        mitk::IOUtil::Save(fib, out_folder + boost::lexical_cast<std::string>((int)(mod*score)) + "_" + bundle_name + ".fib");

        float best_overlap = 0;
        int best_overlap_index = -1;
        int m_idx = 0;
        for (auto ref_mask : reference_masks)
        {
          float overlap = fib->GetOverlap(ref_mask, false);
          if (overlap>best_overlap)
          {
            best_overlap = overlap;
            best_overlap_index = m_idx;
          }
          ++m_idx;
        }

        unsigned int num_voxels = 0;
        {
          itk::TractDensityImageFilter< ItkUcharImageType >::Pointer masks_filter = itk::TractDensityImageFilter< ItkUcharImageType >::New();
          masks_filter->SetInputImage(mask);
          masks_filter->SetBinaryOutput(true);
          masks_filter->SetFiberBundle(fib);
          masks_filter->SetUseImageGeometry(true);
          masks_filter->Update();
          num_voxels = masks_filter->GetNumCoveredVoxels();
        }

        double weight_sum = 0;
        for (int i=0; i<fib->GetNumFibers(); i++)
          weight_sum += fib->GetFiberWeight(i);

        std::cout.rdbuf (old);              // <-- restore

        logfile << "RMS_DIFF: " << setprecision(5) << score << " " << bundle_name << " " << num_voxels << " " << fib->GetNumFibers() << " " << weight_sum << "\n";
        if (best_overlap_index>=0)
          logfile << "Best_overlap: " << setprecision(5) << best_overlap << " " << ist::GetFilenameWithoutExtension(anchor_mask_files.at(best_overlap_index)) << "\n";
        else
          logfile << "No_overlap\n";
        ++c;
      }

    }
    else if (!greedy_add)
    {
      MITK_INFO << "Fit candidate tracts";
      itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
      fitter->SetLambda(lambda);
      fitter->SetFilterOutliers(filter_outliers);
      fitter->SetVerbose(true);
      fitter->SetPeakImage(peak_image);
      fitter->SetResampleFibers(false);
      fitter->SetMaskImage(mask);
      fitter->SetTractograms(input_candidates);
      fitter->SetFitIndividualFibers(true);

      if (regu=="MSM")
        fitter->SetRegularization(VnlCostFunction::REGU::MSM);
      else if (regu=="Variance")
        fitter->SetRegularization(VnlCostFunction::REGU::VARIANCE);
      else if (regu=="Lasso")
        fitter->SetRegularization(VnlCostFunction::REGU::LASSO);
      else if (regu=="VoxelVariance")
        fitter->SetRegularization(VnlCostFunction::REGU::VOXEL_VARIANCE);
      else if (regu=="GroupLasso")
        fitter->SetRegularization(VnlCostFunction::REGU::GROUP_LASSO);
      else if (regu=="GroupVariance")
        fitter->SetRegularization(VnlCostFunction::REGU::GROUP_VARIANCE);
      else if (regu=="NONE")
        fitter->SetRegularization(VnlCostFunction::REGU::NONE);

      fitter->Update();
      vnl_vector<double> rms_diff = fitter->GetRmsDiffPerBundle();

//      vnl_vector<double> log_rms_diff = rms_diff-rms_diff.min_value() + 1;
//      log_rms_diff = log_rms_diff.apply(std::log);
//      log_rms_diff /= log_rms_diff.max_value();
      int c = 0;
      for (auto fib : input_candidates)
      {
//        fib->SetFiberWeights( log_rms_diff[c] );
//        fib->ColorFibersByOrientation();

        std::string bundle_name = ist::GetFilenameWithoutExtension(candidate_tract_files.at(c));

        std::streambuf *old = cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf (ss.rdbuf());       // <-- redirect
        if (filter_zero_weights)
          fib = fib->FilterByWeights(0);
        mitk::IOUtil::Save(fib, out_folder + boost::lexical_cast<std::string>((int)(100000*rms_diff[c])) + "_" + bundle_name + ".fib");

        float best_overlap = 0;
        int best_overlap_index = -1;
        int m_idx = 0;
        for (auto ref_mask : reference_masks)
        {
          float overlap = fib->GetOverlap(ref_mask, false);
          if (overlap>best_overlap)
          {
            best_overlap = overlap;
            best_overlap_index = m_idx;
          }
          ++m_idx;
        }

        unsigned int num_voxels = 0;
        {
          itk::TractDensityImageFilter< ItkUcharImageType >::Pointer masks_filter = itk::TractDensityImageFilter< ItkUcharImageType >::New();
          masks_filter->SetInputImage(mask);
          masks_filter->SetBinaryOutput(true);
          masks_filter->SetFiberBundle(fib);
          masks_filter->SetUseImageGeometry(true);
          masks_filter->Update();
          num_voxels = masks_filter->GetNumCoveredVoxels();
        }

        double weight_sum = 0;
        for (int i=0; i<fib->GetNumFibers(); i++)
          weight_sum += fib->GetFiberWeight(i);

        std::cout.rdbuf (old);              // <-- restore

        logfile << "RMS_DIFF: " << setprecision(5) << rms_diff[c] << " " << bundle_name << " " << num_voxels << " " << fib->GetNumFibers() << " " << weight_sum << "\n";
        if (best_overlap_index>=0)
          logfile << "Best_overlap: " << setprecision(5) << best_overlap << " " << ist::GetFilenameWithoutExtension(anchor_mask_files.at(best_overlap_index)) << "\n";
        else
          logfile << "No_overlap\n";

        ++c;
      }

      mitk::FiberBundle::Pointer out_fib = mitk::FiberBundle::New();
      out_fib = out_fib->AddBundles(input_candidates);
      out_fib->ColorFibersByFiberWeights(false, true);
      mitk::IOUtil::Save(out_fib, out_folder + "AllCandidates.fib");

      peak_image = fitter->GetUnderexplainedImage();
      peak_image_writer->SetInput(peak_image);
      peak_image_writer->SetFileName(out_folder + "Residual_AllCandidates.nii.gz");
      peak_image_writer->Update();
    }
    else
    {
      MITK_INFO << "RMSE: " << setprecision(5) << rmse;
      //    fitter->SetPeakImage(peak_image);

      // Iteratively add candidate bundles in a greedy manner
      while (!input_candidates.empty())
      {
        double next_rmse = rmse;
        double num_peaks = 0;
        mitk::FiberBundle::Pointer best_candidate = nullptr;
        PeakImgType::Pointer best_candidate_peak_image = nullptr;

        for (int i=0; i<(int)input_candidates.size(); ++i)
        {
          // WHY NECESSARY AGAIN??
          itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
          fitter->SetLambda(lambda);
          fitter->SetFilterOutliers(filter_outliers);
          fitter->SetVerbose(false);
          fitter->SetPeakImage(peak_image);
          fitter->SetResampleFibers(false);
          fitter->SetMaskImage(mask);
          // ******************************
          fitter->SetTractograms({input_candidates.at(i)});

          std::streambuf *old = cout.rdbuf(); // <-- save
          std::stringstream ss;
          std::cout.rdbuf (ss.rdbuf());       // <-- redirect
          fitter->Update();
          std::cout.rdbuf (old);              // <-- restore

          double candidate_rmse = fitter->GetRMSE();

          if (candidate_rmse<next_rmse)
          {
            next_rmse = candidate_rmse;
            num_peaks = fitter->GetNumCoveredDirections();
            best_candidate = fitter->GetTractograms().at(0);
            best_candidate_peak_image = fitter->GetUnderexplainedImage();
          }
        }

        if (best_candidate.IsNull())
          break;

        //      fitter->SetPeakImage(peak_image);
        peak_image = best_candidate_peak_image;

        int i=0;
        std::vector< mitk::FiberBundle::Pointer > remaining_candidates;
        std::vector< std::string > remaining_candidate_files;
        for (auto fib : input_candidates)
        {
          if (fib!=best_candidate)
          {
            remaining_candidates.push_back(fib);
            remaining_candidate_files.push_back(candidate_tract_files.at(i));
          }
          else
            name = ist::GetFilenameWithoutExtension(candidate_tract_files.at(i));
          ++i;
        }
        input_candidates = remaining_candidates;
        candidate_tract_files = remaining_candidate_files;

        iteration++;
        std::streambuf *old = cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf (ss.rdbuf());       // <-- redirect
        // Save winning candidate
        if (filter_zero_weights)
          best_candidate = best_candidate->FilterByWeights(0);
        mitk::IOUtil::Save(best_candidate, out_folder + boost::lexical_cast<std::string>(iteration) + "_" + name + ".fib");
        peak_image_writer->SetInput(peak_image);
        peak_image_writer->SetFileName(out_folder + boost::lexical_cast<std::string>(iteration) + "_" + name + ".nrrd");
        peak_image_writer->Update();

        // Calculate best overlap with reference masks for evaluation purposes
        float best_overlap = 0;
        int best_overlap_index = -1;
        i = 0;
        for (auto ref_mask : reference_masks)
        {
          float overlap = best_candidate->GetOverlap(ref_mask, false);
          if (overlap>best_overlap)
          {
            best_overlap = overlap;
            best_overlap_index = i;
          }
          ++i;
        }
        std::cout.rdbuf (old);              // <-- restore

        logfile << "RMSE: " << setprecision(5) << rmse << " " << name << " " << num_peaks << "\n";
        if (best_overlap_index>=0)
          logfile << "Best_overlap: " << setprecision(5) << best_overlap << " " << ist::GetFilenameWithoutExtension(anchor_mask_files.at(best_overlap_index)) << "\n";
        else
          logfile << "No_overlap\n";
      }
    }

    clock.Stop();
    int h = clock.GetTotal()/3600;
    int m = ((int)clock.GetTotal()%3600)/60;
    int s = (int)clock.GetTotal()%60;
    MITK_INFO << "Plausibility estimation took " << h << "h, " << m << "m and " << s << "s";
    logfile.close();
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
