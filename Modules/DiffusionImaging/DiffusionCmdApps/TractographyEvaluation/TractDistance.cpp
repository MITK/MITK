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
#include <itkTractDistanceFilter.h>
#include <mitkClusteringMetricEuclideanMean.h>
#include <mitkClusteringMetricEuclideanMax.h>
#include <mitkClusteringMetricEuclideanStd.h>
#include <mitkDiffusionDataIOHelper.h>


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Tract Distance");
  parser.setCategory("Fiber Processing");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i1", mitkCommandLineParser::StringList, "Input tracts 1:", "input tracts 1", us::Any(), false);
  parser.addArgument("", "i2", mitkCommandLineParser::StringList, "Input tracts 2:", "input tracts 2", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output logfile", us::Any(), false);

  parser.addArgument("fiber_points", "", mitkCommandLineParser::Int, "Fiber points:", "", 12);
  parser.addArgument("metrics", "", mitkCommandLineParser::StringList, "Metrics:", "EU_MEAN (default), EU_STD, EU_MAX");
  parser.addArgument("metric_weights", "", mitkCommandLineParser::StringList, "Metric weights:", "add one float weight for each used metric");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType t1_folder = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i1"]);
  mitkCommandLineParser::StringContainerType t2_folder = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i2"]);
  std::string out_file = us::any_cast<std::string>(parsedArgs["o"]);

  int fiber_points = 12;
  if (parsedArgs.count("fiber_points"))
    fiber_points = us::any_cast<int>(parsedArgs["fiber_points"]);

  std::vector< std::string > metric_strings = {"EU_MEAN"};
  if (parsedArgs.count("metrics"))
    metric_strings = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["metrics"]);

  std::vector< std::string > metric_weights = {"1.0"};
  if (parsedArgs.count("metric_weights"))
    metric_weights = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["metric_weights"]);

  if (metric_strings.size()!=metric_weights.size())
  {
    MITK_INFO << "Each metric needs an associated metric weight!";
    return EXIT_FAILURE;
  }

  try
  {
    std::vector<std::string> t1_files;
    std::vector< mitk::FiberBundle::Pointer > tractograms1 = mitk::DiffusionDataIOHelper::load_fibs(t1_folder, &t1_files);
    std::vector<std::string> t2_files;
    std::vector< mitk::FiberBundle::Pointer > tractograms2 = mitk::DiffusionDataIOHelper::load_fibs(t2_folder, &t2_files);

    MITK_INFO << "Loaded " << tractograms1.size() << " source tractograms.";
    MITK_INFO << "Loaded " << tractograms2.size() << " target tractograms.";

    itk::TractDistanceFilter::Pointer distance_calculator = itk::TractDistanceFilter::New();
    distance_calculator->SetNumPoints(fiber_points);
    distance_calculator->SetTracts1(tractograms1);
    distance_calculator->SetTracts2(tractograms2);

    std::vector< mitk::ClusteringMetric* > metrics;
    int mc = 0;
    for (auto m : metric_strings)
    {
      float w = boost::lexical_cast<float>(metric_weights.at(mc));
      MITK_INFO << "Metric: " << m << " (w=" << w << ")";
      if (m=="EU_MEAN")
        metrics.push_back({new mitk::ClusteringMetricEuclideanMean()});
      else if (m=="EU_STD")
        metrics.push_back({new mitk::ClusteringMetricEuclideanStd()});
      else if (m=="EU_MAX")
        metrics.push_back({new mitk::ClusteringMetricEuclideanMax()});
      metrics.back()->SetScale(w);
      mc++;
    }

    if (metrics.empty())
    {
      MITK_INFO << "No metric selected!";
      return EXIT_FAILURE;
    }

    distance_calculator->SetMetrics(metrics);
    distance_calculator->Update();
    MITK_INFO << "Distances:";
    auto distances = distance_calculator->GetMinDistances();
    auto indices = distance_calculator->GetMinIndices();

    ofstream logfile;
    logfile.open (out_file);
    for (unsigned int i=0; i<distances.size(); ++i)
    {
      std::cout << ist::GetFilenameWithoutExtension(t1_files.at(i)) << " << " << ist::GetFilenameWithoutExtension(t2_files.at(indices[i])) << ": " << distances[i] << std::endl;
      logfile << ist::GetFilenameWithoutExtension(t1_files.at(i)) << " " << ist::GetFilenameWithoutExtension(t2_files.at(indices[i])) << " " << distances[i] << std::endl;
    }
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
