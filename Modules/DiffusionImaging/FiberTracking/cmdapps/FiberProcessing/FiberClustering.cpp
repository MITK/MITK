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

  parser.setTitle("Fiber Clustering");
  parser.setCategory("Fiber Processing");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::InputFile, "Input:", "input fiber bundle (.fib, .trk, .tck)", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);

  parser.addArgument("cluster_size", "", mitkCommandLineParser::Int, "Cluster size:", "", 10);
  parser.addArgument("fiber_points", "", mitkCommandLineParser::Int, "Fiber points:", "", 12);
  parser.addArgument("min_fibers", "", mitkCommandLineParser::Int, "Min. fibers per cluster:", "", 1);
  parser.addArgument("max_clusters", "", mitkCommandLineParser::Int, "Max. clusters:", "");
  parser.addArgument("merge_clusters", "", mitkCommandLineParser::Float, "Merge clusters:", "", -1.0);
  parser.addArgument("output_centroids", "", mitkCommandLineParser::Bool, "Output centroids:", "");
  parser.addArgument("metric", "", mitkCommandLineParser::String, "Metric:", "");
  parser.addArgument("input_centroids", "", mitkCommandLineParser::String, "Input centroids:", "");
  parser.addArgument("scalar_map", "", mitkCommandLineParser::String, "Scalar map:", "");
  parser.addArgument("map_scale", "", mitkCommandLineParser::Float, "Map scale:", "", 0.0);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string out_root = us::any_cast<std::string>(parsedArgs["o"]);

  int cluster_size = 10;
  if (parsedArgs.count("cluster_size"))
    cluster_size = us::any_cast<int>(parsedArgs["cluster_size"]);

  int fiber_points = 12;
  if (parsedArgs.count("fiber_points"))
    fiber_points = us::any_cast<int>(parsedArgs["fiber_points"]);

  int min_fibers = 1;
  if (parsedArgs.count("min_fibers"))
    min_fibers = us::any_cast<int>(parsedArgs["min_fibers"]);

  int max_clusters = 0;
  if (parsedArgs.count("max_clusters"))
    max_clusters = us::any_cast<int>(parsedArgs["max_clusters"]);

  float map_scale = 0.0;
  if (parsedArgs.count("map_scale"))
    map_scale = us::any_cast<float>(parsedArgs["map_scale"]);

  float merge_clusters = -1.0;
  if (parsedArgs.count("merge_clusters"))
    merge_clusters = us::any_cast<float>(parsedArgs["merge_clusters"]);

  bool output_centroids = false;
  if (parsedArgs.count("output_centroids"))
    output_centroids = us::any_cast<bool>(parsedArgs["output_centroids"]);

  std::string metric = "MDF_STD";
  if (parsedArgs.count("metric"))
    metric = us::any_cast<std::string>(parsedArgs["metric"]);

  std::string input_centroids = "";
  if (parsedArgs.count("input_centroids"))
    input_centroids = us::any_cast<std::string>(parsedArgs["input_centroids"]);

  std::string scalar_map = "";
  if (parsedArgs.count("scalar_map"))
    scalar_map = us::any_cast<std::string>(parsedArgs["scalar_map"]);

  try
  {
    typedef itk::Image< float, 3 > FloatImageType;

    mitk::FiberBundle::Pointer fib = LoadFib(inFileName);

    float max_d = 0;
    int i=1;
    std::vector< float > distances;
    while (max_d < fib->GetGeometry()->GetDiagonalLength()/2)
    {
      distances.push_back(cluster_size*i);
      max_d = cluster_size*i;
      ++i;
    }

    itk::TractClusteringFilter::Pointer clusterer = itk::TractClusteringFilter::New();
    clusterer->SetDistances(distances);
    clusterer->SetTractogram(fib);

    if (input_centroids!="")
    {

      mitk::FiberBundle::Pointer in_centroids = LoadFib(input_centroids);
      clusterer->SetInCentroids(in_centroids);
    }

    if (metric=="MDF")
      clusterer->SetMetric(itk::TractClusteringFilter::Metric::MDF);
    else if (metric=="MDF_STD")
      clusterer->SetMetric(itk::TractClusteringFilter::Metric::MDF_STD);
    else if (metric=="MAX_MDF")
      clusterer->SetMetric(itk::TractClusteringFilter::Metric::MAX_MDF);

    if (scalar_map!="")
    {
      mitk::Image::Pointer mitk_map = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(scalar_map)[0].GetPointer());
      if (mitk_map->GetDimension()==3)
      {
        FloatImageType::Pointer itk_map = FloatImageType::New();
        mitk::CastToItkImage(mitk_map, itk_map);
        clusterer->SetScalarMap(itk_map);
      }
    }
    clusterer->SetMergeDuplicateThreshold(merge_clusters);
    clusterer->SetScale(map_scale);
    clusterer->SetNumPoints(fiber_points);
    clusterer->SetMaxClusters(max_clusters);
    clusterer->SetMinClusterSize(min_fibers);
    clusterer->Update();
    std::vector<mitk::FiberBundle::Pointer> tracts = clusterer->GetOutTractograms();
    std::vector<mitk::FiberBundle::Pointer> centroids = clusterer->GetOutCentroids();

    MITK_INFO << "Saving clusters";
    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect
    unsigned int c = 0;
    for (auto f : tracts)
    {
      mitk::IOUtil::Save(f, out_root + "Cluster_" + boost::lexical_cast<std::string>(c) + ".fib");

      if (output_centroids)
        mitk::IOUtil::Save(centroids.at(c), out_root + "Centroid_" + boost::lexical_cast<std::string>(c) + ".fib");
      ++c;
    }
    std::cout.rdbuf (old);              // <-- restore

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
