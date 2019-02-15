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
#include <mitkDiffusionDataIOHelper.h>
#include <mitkLexicalCast.h>
#include <itkTractDistanceFilter.h>
#include <mitkClusteringMetricEuclideanMean.h>
#include <mitkClusteringMetricEuclideanMax.h>
#include <mitkClusteringMetricEuclideanStd.h>
#include <boost/progress.hpp>

typedef itk::Image< unsigned char, 3 >  ItkUcharImageType;
typedef itk::Image< float, 4 >  ItkPeakImgType;

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Reference Similarity");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::StringList, "Input Tracts:", "input tracts folder", us::Any(), false);
  parser.addArgument("reference_tracts", "", mitkCommandLineParser::StringList, "", "", us::Any(), false);
  parser.addArgument("reference_masks", "", mitkCommandLineParser::StringList, "", "", us::Any(), false);
  parser.addArgument("reference_peaks", "", mitkCommandLineParser::StringList, "", "", us::Any(), false);

  parser.addArgument("", "o", mitkCommandLineParser::String, "", "", us::Any(), false);

  parser.addArgument("fiber_points", "", mitkCommandLineParser::Int, "Fiber points:", "", 20);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string out_folder = us::any_cast<std::string>(parsedArgs["o"]);
  mitkCommandLineParser::StringContainerType input_tract_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i"]);
  mitkCommandLineParser::StringContainerType reference_tract_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference_tracts"]);
  mitkCommandLineParser::StringContainerType reference_mask_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference_masks"]);
  mitkCommandLineParser::StringContainerType reference_peak_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference_peaks"]);

  int fiber_points = 20;
  if (parsedArgs.count("fiber_points"))
    fiber_points = us::any_cast<int>(parsedArgs["fiber_points"]);

  try
  {
    std::vector<std::string>  input_tract_names;
    std::vector<std::string>  ref_tract_names;
    std::vector< mitk::FiberBundle::Pointer > input_tracts = mitk::DiffusionDataIOHelper::load_fibs(input_tract_files, &input_tract_names);
    std::vector< mitk::FiberBundle::Pointer > reference_tracts = mitk::DiffusionDataIOHelper::load_fibs(reference_tract_files, &ref_tract_names);
    std::vector< ItkUcharImageType::Pointer > reference_masks = mitk::DiffusionDataIOHelper::load_itk_images<ItkUcharImageType>(reference_mask_files);
    std::vector< ItkPeakImgType::Pointer > reference_peaks = mitk::DiffusionDataIOHelper::load_itk_images<ItkPeakImgType>(reference_peak_files);

    MITK_INFO << "Calculating distances";
    itk::TractDistanceFilter::Pointer distance_calculator = itk::TractDistanceFilter::New();
    distance_calculator->SetNumPoints(fiber_points);
    distance_calculator->SetTracts1(input_tracts);
    distance_calculator->SetTracts2(reference_tracts);
    distance_calculator->SetMetrics({new mitk::ClusteringMetricEuclideanMean()});
    distance_calculator->Update();
    auto distances = distance_calculator->GetAllDistances();

    vnl_matrix<float> voxel_overlap; voxel_overlap.set_size(input_tracts.size(), reference_tracts.size());
    vnl_matrix<float> dir_overlap; dir_overlap.set_size(input_tracts.size(), reference_tracts.size());

    MITK_INFO << "Calculating overlap";
    boost::progress_display disp(input_tracts.size()*reference_tracts.size());
    int r=0;
    for (auto fib : input_tracts)
    {
      int c=0;
      for (auto ref_mask : reference_masks)
      {
//        ++disp;
//        std::streambuf *old = cout.rdbuf(); // <-- save
//        std::stringstream ss;
//        std::cout.rdbuf (ss.rdbuf());       // <-- redirect
        float overlap = 0;
        float directional_overlap = 0;
        std::tie(directional_overlap, overlap) = fib->GetDirectionalOverlap(ref_mask, reference_peaks.at(c));
        voxel_overlap[r][c] = overlap;
        dir_overlap[r][c] = directional_overlap;
//        std::cout.rdbuf (old);              // <-- restore
        ++c;
      }
      ++r;
    }

    ofstream logfile;
    logfile.open(out_folder + "ref_tract_names.txt");
    for (unsigned int i=0; i<ref_tract_names.size(); ++i)
      logfile << ist::GetFilenameWithoutExtension(ref_tract_names.at(i)) << std::endl;
    logfile << "----------" << std::endl;
    for (unsigned int i=0; i<input_tract_names.size(); ++i)
      logfile << ist::GetFilenameWithoutExtension(input_tract_names.at(i)) << std::endl;
    logfile.close();

    MITK_INFO << "Saving matrices";
    {
      std::ofstream OutputStream(out_folder + "ref_distances.txt");
      OutputStream << distances;
      OutputStream.close();
    }
    {
      std::ofstream OutputStream(out_folder + "ref_voxel_overlap.txt");
      OutputStream << voxel_overlap;
      OutputStream.close();
    }
    {
      std::ofstream OutputStream(out_folder + "ref_dir_overlap.txt");
      OutputStream << dir_overlap;
      OutputStream.close();
    }

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
