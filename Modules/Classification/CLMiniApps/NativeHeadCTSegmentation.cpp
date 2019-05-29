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

#include <mitkCoreObjectFactory.h>
#include "mitkImage.h"
#include <mitkLexicalCast.h>
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>

// ITK
#include <itkImageRegionIterator.h>

// MITK
#include <mitkIOUtil.h>

// Classification
#include <mitkCLUtil.h>
#include <mitkVigraRandomForestClassifier.h>
#include <mitkAbstractFileReader.h>

#include <QDir>
#include <QString>
#include <QStringList>

#include <mitkImageAccessByItk.h>

using namespace mitk;

typedef unsigned int uint;
void ReadMitkProjectImageAndMask(std::string input_file, mitk::Image::Pointer & raw_image, mitk::Image::Pointer & class_mask, Image::Pointer &brain_mask);
std::map<unsigned int, double> VolumeUnderMaskByLabel(mitk::Image::Pointer mask);

/**
*
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  // required params
  parser.addArgument("inputdir", "i", mitkCommandLineParser::Directory, "Input Directory", "Contains input feature files.", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("outputdir", "o", mitkCommandLineParser::Directory, "Output Directory", "Destination of output files.", us::Any(), false, false, false, mitkCommandLineParser::Output);

  // optional params
  parser.addArgument("select", "s", mitkCommandLineParser::String, "Item selection", "Using Regular expression, seperated by space e.g.: '*.nrrd *.vtk *test*'",std::string("*.mitk"),true);
  //  parser.addArgument("treecount", "tc", mitkCommandLineParser::Int, "Treecount", "Number of trees.",50,true);
  //  parser.addArgument("treedepth", "td", mitkCommandLineParser::Int, "Treedepth", "Maximal tree depth.",50,true);
  //  parser.addArgument("minsplitnodesize", "min", mitkCommandLineParser::Int, "Minimum split node size.", "Minimum split node size.",2,true);
  //  parser.addArgument("precision", "p", mitkCommandLineParser::Float, "Split precision.", "Precision.", mitk::eps,true);
  //  parser.addArgument("fraction", "f", mitkCommandLineParser::Float, "Fraction of samples per tree.", "Fraction of samples per tree.", 0.6f,true);
  //  parser.addArgument("replacment", "r", mitkCommandLineParser::Bool, "Sample with replacement.", "Sample with replacement.", true,true);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Native Head CT Segmentation");
  parser.setDescription("Using vigra random forest");
  parser.setContributor("Jonas Cordes");

  // Params parsing
  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inputdir = us::any_cast<std::string>(parsedArgs["inputdir"]);
  std::string outputdir = us::any_cast<std::string>(parsedArgs["outputdir"]);

  //  int treecount = parsedArgs.count("treecount") ? us::any_cast<int>(parsedArgs["treecount"]) : 50;
  //  int treedepth = parsedArgs.count("treedepth") ? us::any_cast<int>(parsedArgs["treedepth"]) : 50;
  //  int minsplitnodesize = parsedArgs.count("minsplitnodesize") ? us::any_cast<int>(parsedArgs["minsplitnodesize"]) : 2;
  //  float precision = parsedArgs.count("precision") ? us::any_cast<float>(parsedArgs["precision"]) : mitk::eps;
  //  float fraction = parsedArgs.count("fraction") ? us::any_cast<float>(parsedArgs["fraction"]) : 0.6;
  //  bool withreplacement = parsedArgs.count("replacment") ? us::any_cast<float>(parsedArgs["replacment"]) : true;
  std::string filt_select = parsedArgs.count("select") ? us::any_cast<std::string>(parsedArgs["select"]) : "*.mitk";

  QString filter(filt_select.c_str());
  QDir dir(inputdir.c_str());
  auto strl = dir.entryList(filter.split(" "),QDir::Files);

  // create the one
  mitk::VigraRandomForestClassifier::Pointer classifier = mitk::VigraRandomForestClassifier::New();
  uint n_samples = 45000;
  uint n_samples_per_image = n_samples / strl.size();
  uint n_features = 2;

  {// Training
    Eigen::MatrixXd feature_matrix(n_samples, n_features);
    Eigen::MatrixXi label_matrix(n_samples, 1);

    uint pos = 0;

    for(auto entry : strl)
    {
      mitk::Image::Pointer raw_image, class_mask, brain_mask;
      ReadMitkProjectImageAndMask(inputdir + entry.toStdString(), raw_image, class_mask, brain_mask);

      mitk::Image::Pointer brain_mask_sampled;
      AccessFixedDimensionByItk_2(brain_mask, mitk::CLUtil::itkSampleLabel, 3, brain_mask_sampled, n_samples_per_image);

      mitk::Image::Pointer csf_prob;
      mitk::CLUtil::ProbabilityMap(raw_image,13.9, 8.3,csf_prob);
      mitk::CLUtil::ErodeGrayscale(csf_prob,1,mitk::CLUtil::Axial,csf_prob);
      mitk::CLUtil::DilateGrayscale(csf_prob,3,mitk::CLUtil::Axial,csf_prob);
      mitk::CLUtil::FillHoleGrayscale(csf_prob,csf_prob);

      feature_matrix.block(pos, 0, n_samples_per_image, 1) = mitk::CLUtil::Transform<double>(raw_image,brain_mask_sampled);
      feature_matrix.block(pos, 1, n_samples_per_image, 1) = mitk::CLUtil::Transform<double>(csf_prob,brain_mask_sampled);

      label_matrix.block(pos, 0, n_samples_per_image, 1) = mitk::CLUtil::Transform<int>(class_mask, brain_mask_sampled);
      pos += n_samples_per_image;
    }
    classifier->Train(feature_matrix, label_matrix);
    classifier->PrintParameter();
  }

  std::map<std::string, std::pair<double, double> > map_error;

  for(auto entry: strl)
  {
    mitk::Image::Pointer raw_image, class_mask, brain_mask;
    ReadMitkProjectImageAndMask(inputdir + entry.toStdString(), raw_image, class_mask, brain_mask);

    auto map_true = VolumeUnderMaskByLabel(class_mask);

    mitk::Image::Pointer csf_prob;
    mitk::CLUtil::ProbabilityMap(raw_image,13.9, 8.3,csf_prob);
    mitk::CLUtil::ErodeGrayscale(csf_prob,1,mitk::CLUtil::Axial,csf_prob);
    mitk::CLUtil::DilateGrayscale(csf_prob,3,mitk::CLUtil::Axial,csf_prob);
    mitk::CLUtil::FillHoleGrayscale(csf_prob,csf_prob);

    uint count = 0;
    mitk::CLUtil::CountVoxel(brain_mask,count);

    Eigen::MatrixXd feature_matrix(count,n_features);
    feature_matrix.block(0, 0, count, 1) = mitk::CLUtil::Transform<double>(raw_image,brain_mask);
    feature_matrix.block(0, 1, count, 1) = mitk::CLUtil::Transform<double>(csf_prob,brain_mask);

    mitk::Image::Pointer result_mask = mitk::CLUtil::Transform<int>(classifier->Predict(feature_matrix),brain_mask);

    std::string name = itksys::SystemTools::GetFilenameWithoutExtension(entry.toStdString());
    mitk::IOUtil::Save(result_mask,outputdir + name + ".nrrd");

    auto map_pred = VolumeUnderMaskByLabel(result_mask);

    map_error[entry.toStdString()] = std::make_pair(std::abs(map_true[1] - map_pred[1]) / map_true[1], map_true[2] != 0 ? std::abs(map_true[2] - map_pred[2]) / map_true[2]: 0);
  }

  double mean_error_csf = 0;
  double mean_error_les = 0;
  double num_subjects = map_error.size();
  for(auto entry: map_error)
  {
    MITK_INFO(entry.first.c_str()) << "CSF error: " << entry.second.first << "%\t LES error: " << entry.second.second << "%";
    mean_error_csf += entry.second.first;
    mean_error_les += entry.second.second;
  }
  MITK_INFO("Mean") << "CSF error: " << mean_error_csf/num_subjects << "%\t LES error: " << mean_error_les/num_subjects << "%";

  return EXIT_SUCCESS;
}

void ReadMitkProjectImageAndMask(std::string input_file, mitk::Image::Pointer & raw_image, mitk::Image::Pointer & class_mask, mitk::Image::Pointer & brain_mask)
{
  auto so = mitk::IOUtil::Load(input_file);

  std::map<uint, uint> map;

  mitk::CLUtil::CountVoxel(dynamic_cast<mitk::Image *>(so[1].GetPointer()), map);

  raw_image = map.size() <= 7 ? dynamic_cast<mitk::Image *>(so[0].GetPointer()) : dynamic_cast<mitk::Image *>(so[1].GetPointer());
  class_mask = map.size() <= 7 ? dynamic_cast<mitk::Image *>(so[1].GetPointer()) : dynamic_cast<mitk::Image *>(so[0].GetPointer());

  std::map<uint, uint> merge_instructions;// = {{0,0},{1,1},{2,1},{3,1},{4,2},{5,3},{6,3}};
  merge_instructions[0] = 0;
  merge_instructions[1] = 1;
  merge_instructions[2] = 1;
  merge_instructions[3] = 1;
  merge_instructions[4] = 2;
  merge_instructions[5] = 3;
  merge_instructions[6] = 3;
  mitk::CLUtil::MergeLabels(class_mask, merge_instructions);

  brain_mask = class_mask->Clone();
  //merge_instructions = {{0,0},{1,1},{2,1},{3,1},{4,1},{5,1},{6,1}};
  merge_instructions[0] = 0;
  merge_instructions[1] = 1;
  merge_instructions[2] = 1;
  merge_instructions[3] = 1;
  merge_instructions[4] = 1;
  merge_instructions[5] = 1;
  merge_instructions[6] = 1;
  mitk::CLUtil::MergeLabels(brain_mask, merge_instructions);
}

std::map<unsigned int, double> VolumeUnderMaskByLabel(mitk::Image::Pointer mask)
{
  itk::Image<unsigned int, 3>::Pointer img;
  mitk::CastToItkImage(mask,img);

  std::map<unsigned int, double> volume_map;
  itk::ImageRegionConstIterator<itk::Image<unsigned int, 3>> it(img,img->GetLargestPossibleRegion());
  while(!it.IsAtEnd())
  {
    itk::Image<unsigned int, 3>::PixelType value = it.Value();
    if(volume_map.find(value)== volume_map.end())
      volume_map[value] = 0;
    volume_map[value]++;
    ++it;
  }

  auto spacing = img->GetSpacing();
  double volumeUnit = spacing[0] * spacing[1] * spacing[2];
  volumeUnit *= 0.001;

  for(auto entry: volume_map)
    entry.second *= volumeUnit;

  return volume_map;
}
