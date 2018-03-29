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

#include <mitkConfigFileReader.h>
#include <mitkDataCollection.h>
#include <mitkCollectionReader.h>
#include <mitkCollectionWriter.h>
#include <mitkCollectionStatistic.h>
#include <mitkCostingStatistic.h>
#include <vtkSmartPointer.h>
#include <mitkIOUtil.h>

#include <mitkDataCollectionUtilities.h>
#include <mitkRandomForestIO.h>

// ----------------------- Forest Handling ----------------------
#include <mitkVigraRandomForestClassifier.h>


int main(int argc, char* argv[])
{
  MITK_INFO << "Starting MITK_Forest Mini-App";

  //////////////////////////////////////////////////////////////////////////////
  // Read Console Input Parameter
  //////////////////////////////////////////////////////////////////////////////
  ConfigFileReader allConfig(argv[1]);

  bool readFile = true;
  std::stringstream ss;
  for (int i = 0; i < argc; ++i )
  {
    MITK_INFO << "-----"<< argv[i]<<"------";
    if (readFile)
    {
      if (argv[i][0] == '+')
      {
        readFile = false;
        continue;
      } else
      {
        try
        {
          allConfig.ReadFile(argv[i]);
        }
        catch (std::exception &e)
        {
          MITK_INFO << e.what();
        }
      }
    }
    else
    {
      std::string input = argv[i];
      std::replace(input.begin(), input.end(),'_',' ');
      ss << input << std::endl;
    }
  }
  allConfig.ReadStream(ss);

  try
  {
    //////////////////////////////////////////////////////////////////////////////
    // General
    //////////////////////////////////////////////////////////////////////////////
    int currentRun = allConfig.IntValue("General","Run",0);
    //int doTraining = allConfig.IntValue("General","Do Training",1);
    std::string forestPath = allConfig.Value("General","Forest Path");
    std::string trainingCollectionPath = allConfig.Value("General","Patient Collection");
    std::string testCollectionPath = allConfig.Value("General", "Patient Test Collection", trainingCollectionPath);

    //////////////////////////////////////////////////////////////////////////////
    // Read Default Classification
    //////////////////////////////////////////////////////////////////////////////
    std::vector<std::string> trainPatients = allConfig.Vector("Training Group",currentRun);
    std::vector<std::string> testPatients = allConfig.Vector("Test Group",currentRun);
    std::vector<std::string> modalities = allConfig.Vector("Modalities", 0);
    std::vector<std::string> outputFilter = allConfig.Vector("Output Filter", 0);
    std::string trainMask = allConfig.Value("Data","Training Mask");
    std::string completeTrainMask = allConfig.Value("Data","Complete Training Mask");
    std::string testMask = allConfig.Value("Data","Test Mask");
    std::string resultMask = allConfig.Value("Data", "Result Mask");
    std::string resultProb = allConfig.Value("Data", "Result Propability");
    std::string outputFolder = allConfig.Value("General","Output Folder");

    std::string writeDataFilePath = allConfig.Value("Forest","File to write data to");

    //////////////////////////////////////////////////////////////////////////////
    // Read Data Forest Parameter
    //////////////////////////////////////////////////////////////////////////////
    int testSingleDataset = allConfig.IntValue("Data", "Test Single Dataset",0);
    std::string singleDatasetName = allConfig.Value("Data", "Single Dataset Name", "none");
    std::vector<std::string> forestVector = allConfig.Vector("Forests", 0);

    //////////////////////////////////////////////////////////////////////////////
    // Read Statistic Parameter
    //////////////////////////////////////////////////////////////////////////////
    std::string statisticFilePath = allConfig.Value("Evaluation", "Statistic output file");
    std::string statisticShortFilePath = allConfig.Value("Evaluation", "Statistic short output file");
    std::string statisticShortFileLabel = allConfig.Value("Evaluation", "Index for short file");
    std::string statisticGoldStandard = allConfig.Value("Evaluation", "Gold Standard Name","GTV");
    //bool statisticWithHeader = allConfig.IntValue("Evaluation", "Write header in short file",0);
    std::vector<std::string> labelGroupA = allConfig.Vector("LabelsA",0);
    std::vector<std::string> labelGroupB = allConfig.Vector("LabelsB",0);


    std::ofstream timingFile;
    timingFile.open((statisticFilePath + ".timing").c_str(), std::ios::app);
    timingFile << statisticShortFileLabel << ";";
    std::time_t lastTimePoint;
    time(&lastTimePoint);

    //////////////////////////////////////////////////////////////////////////////
    // Read Images
    //////////////////////////////////////////////////////////////////////////////
    std::vector<std::string> usedModalities;
    for (std::size_t i = 0; i < modalities.size(); ++i)
    {
      usedModalities.push_back(modalities[i]);
    }
    usedModalities.push_back(trainMask);
    usedModalities.push_back(completeTrainMask);
    usedModalities.push_back(testMask);
    usedModalities.push_back(statisticGoldStandard);

    //  vtkSmartPointer<mitk::CollectionReader> colReader = vtkSmartPointer<mitk::CollectionReader>::New();
    mitk::CollectionReader* colReader = new mitk::CollectionReader();
    colReader->AddDataElementIds(trainPatients);
    colReader->SetDataItemNames(usedModalities);

    if (testSingleDataset > 0)
    {
      testPatients.clear();
      testPatients.push_back(singleDatasetName);
    }
    colReader->ClearDataElementIds();
    colReader->AddDataElementIds(testPatients);
    mitk::DataCollection::Pointer testCollection = colReader->LoadCollection(testCollectionPath);

    std::time_t now;
    time(&now);
    double seconds =  std::difftime(now, lastTimePoint);
    timingFile << seconds << ";";
    time(&lastTimePoint);


    mitk::VigraRandomForestClassifier::Pointer forest = mitk::VigraRandomForestClassifier::New();
    MITK_INFO << "Convert Test data";
    auto testDataX = mitk::DCUtilities::DC3dDToMatrixXd(testCollection, modalities, testMask);

    for (std::size_t i = 0; i < forestVector.size(); ++i)
    {
      forest = mitk::IOUtil::Load<mitk::VigraRandomForestClassifier>(forestVector[i]);

      time(&now);
      seconds = std::difftime(now, lastTimePoint);
      MITK_INFO << "Duration for Training: " << seconds;
      timingFile << seconds << ";";
      time(&lastTimePoint);

      MITK_INFO << "Predict Test Data";
      auto testDataNewY = forest->Predict(testDataX);
      auto testDataNewProb = forest->GetPointWiseProbabilities();

      auto maxClassValue = testDataNewProb.cols();
      std::vector<std::string> names;
      for (int j = 0; j < maxClassValue; ++j)
      {
        std::string name = resultProb + std::to_string(j);
        names.push_back(name);
      }

      mitk::DCUtilities::MatrixToDC3d(testDataNewY, testCollection, resultMask, testMask);
      mitk::DCUtilities::MatrixToDC3d(testDataNewProb, testCollection, names, testMask);
      MITK_INFO << "Converted predicted data";

      time(&now);
      seconds = std::difftime(now, lastTimePoint);
      timingFile << seconds << ";";
      time(&lastTimePoint);

      //////////////////////////////////////////////////////////////////////////////
      // Save results to folder
      //////////////////////////////////////////////////////////////////////////////
      MITK_INFO << "Write Result to HDD";
      mitk::CollectionWriter::ExportCollectionToFolder(testCollection,
        outputFolder + "/result_collection.xml",
        outputFilter);

      MITK_INFO << "Calculate Statistic....";
      //////////////////////////////////////////////////////////////////////////////
      // Calculate and Print Statistic
      //////////////////////////////////////////////////////////////////////////////
      std::ofstream statisticFile;
      statisticFile.open(statisticFilePath.c_str(), std::ios::app);
      std::ofstream sstatisticFile;
      sstatisticFile.open(statisticShortFilePath.c_str(), std::ios::app);

      mitk::CollectionStatistic stat;
      stat.SetCollection(testCollection);
      stat.SetClassCount(5);
      stat.SetGoldName(statisticGoldStandard);
      stat.SetTestName(resultMask);
      stat.SetMaskName(testMask);
      mitk::BinaryValueminusOneToIndexMapper mapper;
      stat.SetGroundTruthValueToIndexMapper(&mapper);
      stat.SetTestValueToIndexMapper(&mapper);
      stat.Update();
      //stat.Print(statisticFile,sstatisticFile,statisticWithHeader, statisticShortFileLabel);
      stat.Print(statisticFile, sstatisticFile, true, statisticShortFileLabel + "_"+std::to_string(i));
      statisticFile.close();

      time(&now);
      seconds = std::difftime(now, lastTimePoint);
      timingFile << seconds << std::endl;
      time(&lastTimePoint);
      timingFile.close();
    }
  }
  catch (std::string s)
  {
    MITK_INFO << s;
    return 0;
  }
  catch (char* s)
  {
    MITK_INFO << s;
  }

  return 0;
}

#endif
