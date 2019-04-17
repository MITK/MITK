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

#include <mitkFiberBundle.h>
#include <mitkCommandLineParser.h>
#include <mitkLexicalCast.h>
#include <mitkIOUtil.h>
#include <itkTractClusteringFilter.h>
#include <mitkClusteringMetricEuclideanMean.h>
#include <mitkClusteringMetricEuclideanMax.h>
#include <mitkClusteringMetricEuclideanStd.h>
#include <mitkClusteringMetricAnatomic.h>
#include <mitkClusteringMetricScalarMap.h>
#include <mitkDiffusionDataIOHelper.h>

typedef itk::Image<unsigned char, 3>    ItkFloatImgType;

/*!
\brief Spatially cluster fibers
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Extract Similar Tracts");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input fiber bundle (.fib, .trk, .tck)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("ref_tracts", "", mitkCommandLineParser::StringList, "Ref. Tracts:", "reference tracts (.fib, .trk, .tck)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("ref_masks", "", mitkCommandLineParser::StringList, "Ref. Masks:", "reference bundle masks", us::Any(), true, false, false, mitkCommandLineParser::Input);

  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output root", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("distance", "", mitkCommandLineParser::Int, "Distance:", "", 10);
  parser.addArgument("metric", "", mitkCommandLineParser::String, "Metric:", "EU_MEAN (default), EU_STD, EU_MAX");
  parser.addArgument("subsample", "", mitkCommandLineParser::Float, "Subsampling factor:", "Only use specified fraction of input fibers", 1.0);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string in_fib = us::any_cast<std::string>(parsedArgs["i"]);
  std::string out_root = us::any_cast<std::string>(parsedArgs["o"]);
  mitkCommandLineParser::StringContainerType ref_bundle_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["ref_tracts"]);
  mitkCommandLineParser::StringContainerType ref_mask_files;
  if (parsedArgs.count("ref_masks"))
    ref_mask_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["ref_masks"]);

  if (ref_mask_files.size()>0 && ref_mask_files.size()!=ref_bundle_files.size())
  {
    MITK_INFO << "If reference masks are used, there has to be one mask per reference tract.";
    return EXIT_FAILURE;
  }

  int distance = 10;
  if (parsedArgs.count("distance"))
    distance = us::any_cast<int>(parsedArgs["distance"]);

  std::string metric = "EU_MEAN";
  if (parsedArgs.count("metric"))
    metric = us::any_cast<std::string>(parsedArgs["metric"]);

  float subsample = 1.0;
  if (parsedArgs.count("subsample"))
    subsample = us::any_cast<float>(parsedArgs["subsample"]);

  try
  {
    mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(in_fib);

    std::srand(0);
    if (subsample<1.0f)
      fib = fib->SubsampleFibers(subsample, true);

    mitk::FiberBundle::Pointer resampled_fib = fib->GetDeepCopy();
    resampled_fib->ResampleToNumPoints(12);

    auto ref_fibs = mitk::DiffusionDataIOHelper::load_fibs(ref_bundle_files);
    auto ref_masks = mitk::DiffusionDataIOHelper::load_itk_images<ItkFloatImgType>(ref_mask_files);

    std::vector< float > distances;
    distances.push_back(distance);

    mitk::FiberBundle::Pointer extracted = mitk::FiberBundle::New(nullptr);
    unsigned int c = 0;
    for (auto ref_fib : ref_fibs)
    {
      MITK_INFO << "Extracting " << ist::GetFilenameName(ref_bundle_files.at(c));

      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      try
      {
        itk::TractClusteringFilter::Pointer segmenter = itk::TractClusteringFilter::New();

        // calculate centroids from reference bundle
        {
          itk::TractClusteringFilter::Pointer clusterer = itk::TractClusteringFilter::New();
          clusterer->SetDistances({10,20,30});
          clusterer->SetTractogram(ref_fib);
          clusterer->SetMetrics({new mitk::ClusteringMetricEuclideanStd()});
          clusterer->SetMergeDuplicateThreshold(0.0);
          clusterer->Update();
          std::vector<mitk::FiberBundle::Pointer> tracts = clusterer->GetOutCentroids();
          ref_fib = mitk::FiberBundle::New(nullptr);
          ref_fib = ref_fib->AddBundles(tracts);
          mitk::IOUtil::Save(ref_fib, out_root + "centroids_" + ist::GetFilenameName(ref_bundle_files.at(c)));
          segmenter->SetInCentroids(ref_fib);
        }

        // segment tract
        if (c<ref_masks.size())
        {
          segmenter->SetFilterMask(ref_masks.at(c));
          segmenter->SetOverlapThreshold(0.8f);
        }
        segmenter->SetDistances(distances);
        segmenter->SetTractogram(resampled_fib);
        segmenter->SetMergeDuplicateThreshold(0.0);
        segmenter->SetDoResampling(false);
        if (metric=="EU_MEAN")
          segmenter->SetMetrics({new mitk::ClusteringMetricEuclideanMean()});
        else if (metric=="EU_STD")
          segmenter->SetMetrics({new mitk::ClusteringMetricEuclideanStd()});
        else if (metric=="EU_MAX")
          segmenter->SetMetrics({new mitk::ClusteringMetricEuclideanMax()});
        segmenter->Update();

        std::vector< std::vector< unsigned int > > clusters = segmenter->GetOutFiberIndices();
        if (clusters.size()>0)
        {
          vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();

          mitk::FiberBundle::Pointer result = mitk::FiberBundle::New(nullptr);
          std::vector< mitk::FiberBundle::Pointer > result_fibs;
          for (unsigned int cluster_index=0; cluster_index<clusters.size()-1; ++cluster_index)
            result_fibs.push_back(mitk::FiberBundle::New(fib->GeneratePolyDataByIds(clusters.at(cluster_index), weights)));
          result = result->AddBundles(result_fibs);

          extracted = extracted->AddBundle(result);
          mitk::IOUtil::Save(result, out_root + "extracted_" + ist::GetFilenameName(ref_bundle_files.at(c)));

          fib = mitk::FiberBundle::New(fib->GeneratePolyDataByIds(clusters.back(), weights));
          resampled_fib = mitk::FiberBundle::New(resampled_fib->GeneratePolyDataByIds(clusters.back(), weights));
        }
      }
      catch(itk::ExceptionObject& excpt)
      {
        MITK_INFO << "Exception while processing " << ist::GetFilenameName(ref_bundle_files.at(c));
        MITK_INFO << excpt.GetDescription();
      }
      catch(std::exception& excpt)
      {
        MITK_INFO << "Exception while processing " << ist::GetFilenameName(ref_bundle_files.at(c));
        MITK_INFO << excpt.what();
      }
      std::cout.rdbuf (old);              // <-- restore
      if (fib->GetNumFibers()==0)
        break;
      ++c;
    }
    MITK_INFO << "Extracted streamlines: " << extracted->GetNumFibers();
    mitk::IOUtil::Save(extracted, out_root + "extracted_streamlines.trk");

    MITK_INFO << "Residual streamlines: " << fib->GetNumFibers();
    mitk::IOUtil::Save(fib, out_root + "residual_streamlines.trk");
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
