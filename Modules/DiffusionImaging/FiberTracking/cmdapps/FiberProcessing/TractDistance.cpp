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
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>

typedef itksys::SystemTools ist;

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
  std::sort(file_list.begin(), file_list.end());
  return file_list;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Tract Distance");
  parser.setCategory("Fiber Processing");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i1", mitkCommandLineParser::String, "Input Folder 1:", "input tracts folder 1", us::Any(), false);
  parser.addArgument("", "i2", mitkCommandLineParser::String, "Input Folder 2:", "input tracts folder 2", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output logfile", us::Any(), false);

  parser.addArgument("fiber_points", "", mitkCommandLineParser::Int, "Fiber points:", "", 12);
  parser.addArgument("metrics", "", mitkCommandLineParser::StringList, "Metrics:", "EU_MEAN (default), EU_STD, EU_MAX");
  parser.addArgument("metric_weights", "", mitkCommandLineParser::StringList, "Metric weights:", "add one float weight for each used metric");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string t1_folder = us::any_cast<std::string>(parsedArgs["i1"]);
  std::string t2_folder = us::any_cast<std::string>(parsedArgs["i2"]);
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
    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect
    std::vector< mitk::FiberBundle::Pointer > tractograms1;
    std::vector< mitk::FiberBundle::Pointer > tractograms2;

    auto t1_files = get_file_list(t1_folder, {".fib",".trk",".tck"});
    for (auto f : t1_files)
    {
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      tractograms1.push_back(fib);
    }
    auto t2_files = get_file_list(t2_folder, {".fib",".trk",".tck"});
    for (auto f : t2_files)
    {
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      tractograms2.push_back(fib);
    }

    std::cout.rdbuf (old);              // <-- restore
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
    auto distances = distance_calculator->GetDistances();
    auto indices = distance_calculator->GetIndices();

    ofstream logfile;
    logfile.open (out_file);
    for (unsigned int i=0; i<distances.size(); ++i)
    {
      std::cout << ist::GetFilenameWithoutExtension(t1_files.at(i)) << " << " << ist::GetFilenameWithoutExtension(t2_files.at(indices.at(i))) << ": " << distances.at(i) << std::endl;
      logfile << ist::GetFilenameWithoutExtension(t1_files.at(i)) << " " << ist::GetFilenameWithoutExtension(t2_files.at(indices.at(i))) << " " << distances.at(i) << std::endl;
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
