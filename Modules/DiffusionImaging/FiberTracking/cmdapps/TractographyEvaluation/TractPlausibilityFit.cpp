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
#include <boost/lexical_cast.hpp>

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Point<float, 4> PointType4;
typedef itk::Image< float, 4 >  PeakImgType;

std::vector< string > get_file_list(const std::string& path, std::vector< string > extensions={".fib", ".trk"})
{
  std::vector< string > file_list;
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

/*!
\brief Fits the tractogram to the input peak image by assigning a weight to each fiber (similar to https://doi.org/10.1016/j.neuroimage.2015.06.092).
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i1", mitkCommandLineParser::InputFile, "Input tractogram:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("", "i2", mitkCommandLineParser::InputFile, "Input peaks:", "input peak image", us::Any(), false);
  parser.addArgument("", "i3", mitkCommandLineParser::InputFile, "", "", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output folder:", "output folder", us::Any(), false);

  parser.addArgument("mask", "", mitkCommandLineParser::InputFile, "", "", us::Any());
  parser.addArgument("min_gain", "", mitkCommandLineParser::Float, "Min. gain:", "process stops if remaining bundles don't contribute enough", 0.05);

  parser.addArgument("reference_mask_folder", "", mitkCommandLineParser::String, "Reference Mask Folder:", "reference masks for accuracy evaluation", false);

  parser.addArgument("subtract", "", mitkCommandLineParser::Bool, "", "", false);
  parser.addArgument("normalize_gain", "", mitkCommandLineParser::Bool, "", "", false);
  parser.addArgument("max_iter", "", mitkCommandLineParser::Int, "Max. iterations:", "maximum number of optimizer iterations", 20);
  parser.addArgument("bundle_based", "", mitkCommandLineParser::Bool, "Bundle based fit:", "fit one weight per input tractogram/bundle, not for each fiber", false);
  parser.addArgument("min_g", "", mitkCommandLineParser::Float, "Min. gradient:", "lower termination threshold for gradient magnitude", 1e-5);
  parser.addArgument("lambda", "", mitkCommandLineParser::Float, "Lambda:", "modifier for regularization", 0.1);
  parser.addArgument("dont_filter_outliers", "", mitkCommandLineParser::Bool, "Don't filter outliers:", "don't perform second optimization run with an upper weight bound based on the first weight estimation (95% quantile)", false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  string fib_file = us::any_cast<string>(parsedArgs["i1"]);
  string peak_file_name = us::any_cast<string>(parsedArgs["i2"]);
  string candidate_folder = us::any_cast<string>(parsedArgs["i3"]);
  string out_folder = us::any_cast<string>(parsedArgs["o"]);

  bool single_fib = true;
  if (parsedArgs.count("bundle_based"))
    single_fib = !us::any_cast<bool>(parsedArgs["bundle_based"]);

  bool greedy_add = true;
  if (parsedArgs.count("subtract"))
    greedy_add = !us::any_cast<bool>(parsedArgs["subtract"]);

  int max_iter = 20;
  if (parsedArgs.count("max_iter"))
    max_iter = us::any_cast<int>(parsedArgs["max_iter"]);

  float g_tol = 1e-5;
  if (parsedArgs.count("min_g"))
    g_tol = us::any_cast<float>(parsedArgs["min_g"]);

  float min_gain = 0.05;
  if (parsedArgs.count("min_gain"))
    min_gain = us::any_cast<float>(parsedArgs["min_gain"]);

  float lambda = 0.1;
  if (parsedArgs.count("lambda"))
    lambda = us::any_cast<float>(parsedArgs["lambda"]);

  bool normalize_gain = false;
  if (parsedArgs.count("normalize_gain"))
    normalize_gain = us::any_cast<bool>(parsedArgs["normalize_gain"]);

  bool filter_outliers = true;
  if (parsedArgs.count("dont_filter_outliers"))
    filter_outliers = !us::any_cast<bool>(parsedArgs["dont_filter_outliers"]);


  string mask_file = "";
  if (parsedArgs.count("mask"))
    mask_file = us::any_cast<string>(parsedArgs["mask"]);


  string reference_mask_folder = "";
  if (parsedArgs.count("reference_mask_folder"))
    reference_mask_folder = us::any_cast<string>(parsedArgs["reference_mask_folder"]);

  try
  {
    itk::TimeProbe clock;
    clock.Start();

    if (ist::PathExists(out_folder))
      ist::RemoveADirectory(out_folder);
    ist::MakeDirectory(out_folder);

    streambuf *old = cout.rdbuf(); // <-- save
    stringstream ss;
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
      mitk::Image::Pointer mitk_mask = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(mask_file)[0].GetPointer());
      mitk::CastToItkImage(mitk_mask, mask);
    }

    // Load masks covering the true positives for evaluation purposes
    std::vector< string > reference_mask_filenames;
    std::vector< itk::FitFibersToImageFilter::UcharImgType::Pointer > reference_masks;
    if (reference_mask_folder.compare("")!=0)
    {
      reference_mask_filenames = get_file_list(reference_mask_folder, {".nii", ".nii.gz", ".nrrd"});
      for (auto filename : reference_mask_filenames)
      {
        itk::FitFibersToImageFilter::UcharImgType::Pointer ref_mask = nullptr;
        mitk::Image::Pointer ref_mitk_mask = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(filename)[0].GetPointer());
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

    // Load reference tractogram consisting of all known tracts
    std::vector< mitk::FiberBundle::Pointer > input_reference;
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fib_file)[0].GetPointer());
    if (fib.IsNull())
      return EXIT_FAILURE;
    fib->ResampleLinear(minSpacing/10.0);
    input_reference.push_back(fib);

    std::cout.rdbuf (old);              // <-- restore

    // Load all candidate tracts
    std::vector< mitk::FiberBundle::Pointer > input_candidates;
    std::vector< string > candidate_tract_files = get_file_list(candidate_folder);
    for (string f : candidate_tract_files)
    {
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(f)[0].GetPointer());
      if (fib.IsNull())
        continue;
      fib->ResampleLinear(minSpacing/10.0);
      input_candidates.push_back(fib);
    }

    // Fit known tracts to peak image to obtain underexplained image
    int iteration = 0;
    itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
    if (greedy_add)
      fitter->SetTractograms(input_reference);
    else
      fitter->SetTractograms(CombineTractograms(input_reference, input_candidates));
    fitter->SetFitIndividualFibers(single_fib);
    fitter->SetMaxIterations(max_iter);
    fitter->SetGradientTolerance(g_tol);
    fitter->SetLambda(lambda);
    fitter->SetFilterOutliers(filter_outliers);
    fitter->SetPeakImage(peak_image);
    fitter->SetVerbose(true);
    fitter->SetResampleFibers(false);
    fitter->SetMaskImage(mask);
    fitter->Update();
    std::string name = ist::GetFilenameWithoutExtension(fib_file);
    mitk::IOUtil::Save(fitter->GetTractograms().at(0), out_folder + "0_" + name + ".fib");
    if (greedy_add)
    {
      peak_image = fitter->GetUnderexplainedImage();
      peak_image_writer->SetInput(peak_image);
      peak_image_writer->SetFileName(out_folder + boost::lexical_cast<string>(iteration) + "_" + name + ".nrrd");
      peak_image_writer->Update();
    }

    double coverage = fitter->GetCoverage();
    MITK_INFO << "Iteration: " << iteration << " - " << std::fixed << "Coverage: " << setprecision(2) << 100.0*coverage << "%";
    logfile << "Iteration: " << iteration << " - " << std::fixed << "Coverage: " << setprecision(2) << 100.0*coverage << "%\n\n";
    //    fitter->SetPeakImage(peak_image);


    {
      itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
      fitter->SetFitIndividualFibers(single_fib);
      fitter->SetMaxIterations(max_iter);
      fitter->SetGradientTolerance(g_tol);
      fitter->SetLambda(lambda);
      fitter->SetFilterOutliers(filter_outliers);
      fitter->SetVerbose(true);
      fitter->SetPeakImage(peak_image);
      fitter->SetResampleFibers(false);
      fitter->SetMaskImage(mask);
      fitter->SetTractograms(input_candidates);
      fitter->Update();
      MITK_INFO << "DONE";
      vnl_vector<double> rms_diff = fitter->GetRmsDiffPerBundle();

      int c = 0;
      for (auto fib : input_candidates)
      {
        fib->SetFiberWeight(c, rms_diff[c]);
        mitk::IOUtil::Save(fib, out_folder + "RMS_DIFF_" + boost::lexical_cast<string>(rms_diff[c]) + "_" + name + ".fib");

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

        string bundle_name = ist::GetFilenameWithoutExtension(candidate_tract_files.at(c));
        logfile << "RMS_DIFF: " << rms_diff[c] << " " << bundle_name << "\n";
        logfile << "Best overlap: " << best_overlap << " " << ist::GetFilenameWithoutExtension(reference_mask_filenames.at(best_overlap_index)) << "\n";

        ++c;
      }

      logfile.close();
      return EXIT_SUCCESS;
    }



    // Iteratively add/subtract candidate bundles in a greedy manner
    while (!input_candidates.empty())
    {
      double next_coverage = 0;
      double next_covered_directions = 1.0;
      mitk::FiberBundle::Pointer best_candidate = nullptr;
      PeakImgType::Pointer best_candidate_peak_image = nullptr;
      if (greedy_add)
      {
        for (int i=0; i<(int)input_candidates.size(); ++i)
        {
          // WHY NECESSARY AGAIN??
          itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
          fitter->SetFitIndividualFibers(single_fib);
          fitter->SetMaxIterations(max_iter);
          fitter->SetGradientTolerance(g_tol);
          fitter->SetLambda(lambda);
          fitter->SetFilterOutliers(filter_outliers);
          fitter->SetVerbose(false);
          fitter->SetPeakImage(peak_image);
          fitter->SetResampleFibers(false);
          fitter->SetMaskImage(mask);
          // ******************************
          fitter->SetTractograms({input_candidates.at(i)});

          streambuf *old = cout.rdbuf(); // <-- save
          stringstream ss;
          std::cout.rdbuf (ss.rdbuf());       // <-- redirect
          fitter->Update();
          std::cout.rdbuf (old);              // <-- restore

          double candidate_coverage = fitter->GetCoverage();
          if (normalize_gain)
            candidate_coverage /= fitter->GetNumCoveredDirections();

          if (candidate_coverage>next_coverage)
          {
            next_covered_directions = fitter->GetNumCoveredDirections();
            next_coverage = candidate_coverage;
            if ((1.0-coverage) * next_coverage >= min_gain)
            {
              best_candidate = fitter->GetTractograms().at(0);
              best_candidate_peak_image = fitter->GetUnderexplainedImage();
            }
          }
        }
      }
      else
      {
        next_coverage = 1.0;
        for (unsigned int i=0; i<input_candidates.size(); ++i)
        {
          // WHY NECESSARY AGAIN??
          itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
          fitter->SetFitIndividualFibers(single_fib);
          fitter->SetMaxIterations(max_iter);
          fitter->SetGradientTolerance(g_tol);
          fitter->SetLambda(lambda);
          fitter->SetFilterOutliers(filter_outliers);
          fitter->SetVerbose(false);
          fitter->SetPeakImage(peak_image);
          fitter->SetResampleFibers(false);
          fitter->SetMaskImage(mask);
          // ******************************
          fitter->SetTractograms(CombineTractograms(input_reference, input_candidates, i));

          streambuf *old = cout.rdbuf(); // <-- save
          stringstream ss;
          std::cout.rdbuf (ss.rdbuf());       // <-- redirect
          fitter->Update();
          std::cout.rdbuf (old);              // <-- restore

          double candidate_coverage = fitter->GetCoverage();
          if (normalize_gain)
            candidate_coverage /= fitter->GetNumCoveredDirections();

          if (candidate_coverage<next_coverage)
          {
//            MITK_INFO << "Coverage: " << candidate_coverage << " (" << ist::GetFilenameWithoutExtension(candidate_tract_files.at(i)) << ")";
            next_covered_directions = fitter->GetNumCoveredDirections();
            next_coverage = candidate_coverage;
//            if ((1.0-coverage) * next_coverage >= min_gain)
//            {
              best_candidate = input_candidates.at(i);
//            }
          }
        }
      }

      if (best_candidate.IsNull())
        break;

      //      fitter->SetPeakImage(peak_image);
      peak_image = best_candidate_peak_image;

      int i=0;
      std::vector< mitk::FiberBundle::Pointer > remaining_candidates;
      std::vector< string > remaining_candidate_files;
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
      streambuf *old = cout.rdbuf(); // <-- save
      stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      // Save winning candidate
      mitk::IOUtil::Save(best_candidate, out_folder + boost::lexical_cast<string>(iteration) + "_" + name + ".fib");
      if (greedy_add)
      {
        peak_image_writer->SetInput(peak_image);
        peak_image_writer->SetFileName(out_folder + boost::lexical_cast<string>(iteration) + "_" + name + ".nrrd");
        peak_image_writer->Update();
      }

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

      if (normalize_gain)
        next_coverage *= next_covered_directions;
      if (greedy_add)
      {
        MITK_INFO << "Iteration: " << iteration << " - " << std::fixed << "Coverage: " << setprecision(2) << 100.0*(coverage + (1.0-coverage) * next_coverage) << "% (+" << 100*(1.0-coverage) * next_coverage << "%)";
        logfile << "Iteration: " << iteration << " - " << std::fixed << "Coverage: " << setprecision(2) << 100.0*(coverage + (1.0-coverage) * next_coverage) << "% (+" << 100*(1.0-coverage) * next_coverage << "%)\n";
        coverage += (1.0-coverage) * next_coverage;
      }
      else
      {
        MITK_INFO << "Iteration: " << iteration << " - " << std::fixed << "Coverage: " << setprecision(2) << 100.0*coverage << "% (-" << 100*(coverage-next_coverage) << "%)";
        logfile << "Iteration: " << iteration << " - " << std::fixed << "Coverage: " << setprecision(2) << 100.0*coverage << "% (-" << 100*(coverage-next_coverage) << "%)\n";
        coverage = next_coverage;
      }

      if (best_overlap_index>=0)
      {
        MITK_INFO << "Best overlap: " << best_overlap << " - " << ist::GetFilenameWithoutExtension(reference_mask_filenames.at(best_overlap_index));
        logfile << "Best overlap: " << best_overlap << " - " << ist::GetFilenameWithoutExtension(reference_mask_filenames.at(best_overlap_index)) << "\n";
      }
      logfile << "\n";
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
