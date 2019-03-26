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
#ifndef mitkForest_cpp
#define mitkForest_cpp

#include "time.h"
#include <sstream>
#include <fstream>
#include "mitkCommandLineParser.h"
#include <mitkSplitParameterToVector.h>
#include <mitkLog.h>
#include <mitkLogMacros.h>


//#include <mitkLRDensityEstimation.h>
#include <mitkGeneralizedLinearModel.h>

struct TrainingSet {
  vnl_matrix<double> feature;
  vnl_vector<double> label;
  double ratio;
};


const
vnl_matrix<double> ReadMatrix(std::string path)
{
  std::fstream file(path);
  std::vector<std::vector<double> > listOfRows;

  std::string line;
  while (std::getline(file, line))
  {
    auto current_row = mitk::cl::splitDouble(line, ',');
    if (listOfRows.size() < 1)
    {
      listOfRows.push_back(current_row);
    }
    else if (current_row.size() == listOfRows[0].size())
    {
      listOfRows.push_back(current_row);
    }
  }
  file.close();

  vnl_matrix<double> result(listOfRows.size(), listOfRows[0].size());
  for (std::size_t i = 0; i < listOfRows.size(); ++i)
  {
    for (std::size_t j = 0; j < listOfRows[0].size(); ++j)
    {
      result(i, j) = listOfRows[i][j];
    }
  }
  return result;
}

const
TrainingSet ReadBothMatrix(std::string train, std::string test)
{
  std::fstream file(train);
  std::vector<std::vector<double> > listOfRows;
  std::vector<double> label;

  double trSamples = 0;
  double teSamples = 0;

  std::string line;
  while (std::getline(file, line))
  {
    auto current_row = mitk::cl::splitDouble(line, ',');
    if (listOfRows.size() < 1)
    {
      listOfRows.push_back(current_row);
      label.push_back(0);
      trSamples += 1;
    }
    else if (current_row.size() == listOfRows[0].size())
    {
      listOfRows.push_back(current_row);
      label.push_back(0);
      trSamples += 1;
    }
  }
  file.close();

  std::fstream file2(test);
  while (std::getline(file2, line))
  {
    auto current_row = mitk::cl::splitDouble(line, ',');
    if (listOfRows.size() < 1)
    {
      listOfRows.push_back(current_row);
      label.push_back(1);
      teSamples += 1;
    }
    else if (current_row.size() == listOfRows[0].size())
    {
      listOfRows.push_back(current_row);
      label.push_back(1);
      teSamples += 1;
    }
  }
  file2.close();

  vnl_matrix<double> resultMatrix(listOfRows.size(), listOfRows[0].size());
  vnl_vector<double> resultLabel(listOfRows.size());
  for (std::size_t i = 0; i < listOfRows.size(); ++i)
  {
    for (std::size_t j = 0; j < listOfRows[0].size(); ++j)
    {
      resultMatrix(i, j) = listOfRows[i][j];
    }
    resultLabel(i) = label[i];
  }
  TrainingSet set;
  set.feature = resultMatrix;
  set.label = resultLabel;
  set.ratio = trSamples / teSamples;
  return set;
}


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("training", "t", mitkCommandLineParser::Image, "Input Image", "desc", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("prediction", "p", mitkCommandLineParser::Image, "Input Image", "desc", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::Image, "Normalisation mode", "desc", us::Any(), false, false, false, mitkCommandLineParser::Input);
  //parser.addArgument("algorithm", "a", mitkCommandLineParser::InputImage, "Input Mask", "desc", us::Any(), false);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Importance weighting algorithm");
  parser.setDescription("Calculates the importance weighting of two input matrixes. ");
  parser.setContributor("MBI");

  MITK_INFO << "Extracting Parameters....";
  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  std::string trainingPath = us::any_cast<std::string>(parsedArgs["training"]);
  std::string predictionPath = us::any_cast<std::string>(parsedArgs["prediction"]);
  std::string outputPath = us::any_cast<std::string>(parsedArgs["output"]);
  //std::string algorithm = us::any_cast<std::string>(parsedArgs["algorithm"]);

  MITK_INFO << "Reading Data...";
  auto input = ReadBothMatrix(trainingPath, predictionPath);

  MITK_INFO << "Calculating Weights...";
  mitk::GeneralizedLinearModel glm(input.feature, input.label);
  auto weights = glm.ExpMu(input.feature);

  MITK_INFO << "Writing Weights ...";
  MITK_INFO << outputPath;
  std::ofstream file(outputPath);
  for (unsigned int i = 0; i < input.label.size(); ++i)
  {
    if (input.label(i) < 0.5)
    {
      file << (input.ratio * weights(i)) << std::endl;
    }
  }
  file.close();


  //////////////////////////////////////////////////////////////////////////////
  // Read Images
  //////////////////////////////////////////////////////////////////////////////
  //mitk::DataCollection::Pointer col = mitk::DataCollection::New();

  //MITK_INFO << "Arg 2 " << argv[2];
  //mitk::Image::Pointer sur=mitk::IOUtil::Load<mitk::Image>(argv[2]);
  //col->AddData(sur.GetPointer(),"sur");
  //MITK_INFO << "Arg 3 " << argv[3];
  //mitk::Image::Pointer mask=mitk::IOUtil::Load<mitk::Image>(argv[3]);
  //col->AddData(mask.GetPointer(),"mask");

  //std::vector<std::string> modalities;
  //for (int i = 4; i < argc; ++i)
  //{
  //  MITK_INFO << "Img " << argv[i];
  //  std::stringstream ss;
  //  ss << i;
  //  modalities.push_back(ss.str());
  //  mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(argv[i]);
  //  col->AddData(img.GetPointer(),ss.str());
  //}

  //mitk::LRDensityEstimation est;
  //est.SetCollection(col);
  //est.SetTrainMask("sur");
  //est.SetTestMask("mask");
  //est.SetModalities(modalities);
  //est.SetWeightName("weight");
  //est.Update();

  //mitk::Image::Pointer w= col->GetMitkImage("weight");
  //mitk::IOUtil::SaveImage(w,argv[1]);

  return 0;
}

#endif
