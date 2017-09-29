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

std::vector< string > get_file_list(const std::string& path)
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
      if (ext==".fib" || ext==".trk")
        file_list.push_back(path + '/' + filename);
    }
  }
  return file_list;
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
  parser.addArgument("min_gain", "", mitkCommandLineParser::Float, "Min. gain:", "process stops if remaining bundles don't contribute enough", 0.05);

  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);

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
  string outRoot = us::any_cast<string>(parsedArgs["o"]);

  bool single_fib = true;
  if (parsedArgs.count("bundle_based"))
    single_fib = !us::any_cast<bool>(parsedArgs["bundle_based"]);

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

  bool filter_outliers = true;
  if (parsedArgs.count("dont_filter_outliers"))
    filter_outliers = !us::any_cast<bool>(parsedArgs["dont_filter_outliers"]);

  try
  {
    itk::TimeProbe clock;
    clock.Start();

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::PeakImage*>(mitk::IOUtil::Load(peak_file_name, &functor)[0].GetPointer());

    typedef mitk::ImageToItk< PeakImgType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(inputImage);
    caster->Update();
    PeakImgType::Pointer peak_image = caster->GetOutput();

    std::vector< mitk::FiberBundle::Pointer > input_reference;
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fib_file)[0].GetPointer());
    if (fib.IsNull())
      return EXIT_FAILURE;
    input_reference.push_back(fib);

    std::vector< mitk::FiberBundle::Pointer > input_candidates;
    std::vector< string > candidate_tract_files = get_file_list(candidate_folder);
    for (string f : candidate_tract_files)
    {
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(f)[0].GetPointer());
      if (fib.IsNull())
        continue;
      input_candidates.push_back(fib);
    }

    int iteration = 0;
    std::string name = ist::GetFilenameWithoutExtension(fib_file);

    itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
    fitter->SetTractograms(input_reference);
    fitter->SetFitIndividualFibers(single_fib);
    fitter->SetMaxIterations(max_iter);
    fitter->SetGradientTolerance(g_tol);
    fitter->SetLambda(lambda);
    fitter->SetFilterOutliers(filter_outliers);
    fitter->SetPeakImage(peak_image);
    fitter->SetVerbose(true);
    fitter->SetDeepCopy(false);
    fitter->Update();

//    fitter->GetTractograms().at(0)->SetFiberWeights(fitter->GetCoverage());
//    fitter->GetTractograms().at(0)->ColorFibersByFiberWeights(false, false);

    mitk::IOUtil::Save(fitter->GetTractograms().at(0), outRoot + "0_" + name + ".fib");
    peak_image = fitter->GetUnderexplainedImage();

    itk::ImageFileWriter< PeakImgType >::Pointer writer = itk::ImageFileWriter< PeakImgType >::New();
    writer->SetInput(peak_image);
    writer->SetFileName(outRoot + boost::lexical_cast<string>(iteration) + "_" + name + ".nrrd");
    writer->Update();

    double coverage = fitter->GetCoverage();
    MITK_INFO << "Iteration: " << iteration;
    MITK_INFO << std::fixed << "Coverage: " << setprecision(1) << 100.0*coverage << "%";
//    fitter->SetPeakImage(peak_image);

    while (!input_candidates.empty())
    {
      streambuf *old = cout.rdbuf(); // <-- save
      stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect

      double next_coverage = 0;
      mitk::FiberBundle::Pointer best_candidate = nullptr;
      for (auto fib : input_candidates)
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
        fitter->SetDeepCopy(false);
        // ******************************
        fitter->SetTractograms({fib});
        fitter->Update();

        double candidate_coverage = fitter->GetCoverage();

        if (candidate_coverage>next_coverage)
        {
          next_coverage = candidate_coverage;
          if ((1.0-coverage) * next_coverage >= min_gain)
          {
            best_candidate = fitter->GetTractograms().at(0);
            peak_image = fitter->GetUnderexplainedImage();
          }
        }
      }

      if (best_candidate.IsNull())
      {
        std::cout.rdbuf (old);              // <-- restore
        break;
      }

//      fitter->SetPeakImage(peak_image);
//      best_candidate->SetFiberWeights((1.0-coverage) * next_coverage);
//      best_candidate->ColorFibersByFiberWeights(false, false);

      coverage += (1.0-coverage) * next_coverage;

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
      mitk::IOUtil::Save(best_candidate, outRoot + boost::lexical_cast<string>(iteration) + "_" + name + ".fib");
      writer->SetInput(peak_image);
      writer->SetFileName(outRoot + boost::lexical_cast<string>(iteration) + "_" + name + ".nrrd");
      writer->Update();
      std::cout.rdbuf (old);              // <-- restore

      MITK_INFO << "Iteration: " << iteration;
      MITK_INFO << std::fixed << "Coverage: " << setprecision(1) << 100.0*coverage << "% (+" << 100*(1.0-coverage) * next_coverage << "%)";
    }

    clock.Stop();
    int h = clock.GetTotal()/3600;
    int m = ((int)clock.GetTotal()%3600)/60;
    int s = (int)clock.GetTotal()%60;
    MITK_INFO << "Plausibility estimation took " << h << "h, " << m << "m and " << s << "s";
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
