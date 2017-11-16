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
#include <boost/lexical_cast.hpp>
#include <mitkIOUtil.h>
#include <itkTractClusteringFilter.h>

typedef itksys::SystemTools ist;

mitk::FiberBundle::Pointer LoadFib(std::string filename)
{
  std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(filename);
  if( fibInfile.empty() )
    std::cout << "File " << filename << " could not be read!";
  mitk::BaseData::Pointer baseData = fibInfile.at(0);
  return dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
}

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
  parser.addArgument("", "i", mitkCommandLineParser::InputFile, "Input:", "input fiber bundle (.fib, .trk, .tck)", us::Any(), false);
  parser.addArgument("", "c", mitkCommandLineParser::StringList, "Centroids:", "input centroids (.fib, .trk, .tck)", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);
  parser.addArgument("distance", "", mitkCommandLineParser::Int, "Distance:", "", 10);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string in_fib = us::any_cast<std::string>(parsedArgs["i"]);
  mitkCommandLineParser::StringContainerType in_centroids = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["c"]);
  std::string out_root = us::any_cast<std::string>(parsedArgs["o"]);

  int distance = 10;
  if (parsedArgs.count("distance"))
    distance = us::any_cast<int>(parsedArgs["distance"]);

  try
  {
    mitk::FiberBundle::Pointer fib = LoadFib(in_fib);
    fib->ResampleToNumPoints(12);

    std::vector< mitk::FiberBundle::Pointer > centroid_fibs;
    for (std::size_t i=0; i<in_centroids.size(); ++i)
    {
      try
      {
        centroid_fibs.push_back(LoadFib(in_centroids.at(i)));
      }
      catch(...){
        std::cout << "could not load: " << in_centroids.at(i);
        return EXIT_FAILURE;
      }
    }

    std::vector< float > distances;
    distances.push_back(distance);

    unsigned int c = 0;
    for (auto centroid : centroid_fibs)
    {
      {
        itk::TractClusteringFilter::Pointer clusterer = itk::TractClusteringFilter::New();
        clusterer->SetDistances({10});
        clusterer->SetTractogram(centroid);
        clusterer->SetMetric(itk::TractClusteringFilter::Metric::MDF_STD);
        clusterer->Update();
        std::vector<mitk::FiberBundle::Pointer> tracts = clusterer->GetOutCentroids();
        centroid = mitk::FiberBundle::New(nullptr);
        centroid = centroid->AddBundles(tracts);
        mitk::IOUtil::Save(centroid, out_root + "centroids_" + ist::GetFilenameName(in_centroids.at(c)));
      }
      itk::TractClusteringFilter::Pointer clusterer = itk::TractClusteringFilter::New();
      clusterer->SetDistances(distances);
      clusterer->SetTractogram(fib);
      clusterer->SetInCentroids(centroid);
      clusterer->SetDoResampling(false);
      clusterer->SetMetric(itk::TractClusteringFilter::Metric::MAX_MDF);
      clusterer->Update();

      std::vector<mitk::FiberBundle::Pointer> tracts = clusterer->GetOutTractograms();
      fib = tracts.back();
      tracts.pop_back();
      mitk::FiberBundle::Pointer result = mitk::FiberBundle::New(nullptr);
      result = result->AddBundles(tracts);
      mitk::IOUtil::Save(result, out_root + "anchor_" + ist::GetFilenameName(in_centroids.at(c)));
      ++c;
    }
    mitk::IOUtil::Save(fib, out_root + "candidate_tractogram.trk");
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
