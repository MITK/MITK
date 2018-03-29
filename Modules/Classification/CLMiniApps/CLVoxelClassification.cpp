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
//#include <mitkDecisionForest.h>
#include <mitkVigraRandomForestClassifier.h>
//#include <mitkThresholdSplit.h>
//#include <mitkImpurityLoss.h>
//#include <mitkLinearSplitting.h>
//#include <mitkVigraConverter.h>
// ----------------------- Point weighting ----------------------
//#include <mitkForestWeighting.h>
//#include <mitkKliepDensityEstimation.h>
//#include <mitkExternalWeighting.h>
#include <mitkLRDensityEstimation.h>
//#include <mitkKNNDensityEstimation.h>
//#include <mitkZadroznyWeighting.h>
//#include <mitkSpectralDensityEstimation.h>
//#include <mitkULSIFDensityEstimation.h>

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
    int doTraining = allConfig.IntValue("General","Do Training",1);
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
    int trainSingleDataset = allConfig.IntValue("Data", "Train Single Dataset", 0);
    std::string singleTrainDatasetName = allConfig.Value("Data", "Train Single Dataset Name", "none");

    //////////////////////////////////////////////////////////////////////////////
    // Read Forest Parameter
    //////////////////////////////////////////////////////////////////////////////
    int minimumSplitNodeSize = allConfig.IntValue("Forest", "Minimum split node size",1);
    int numberOfTrees = allConfig.IntValue("Forest", "Number of Trees",255);
    double samplesPerTree = atof(allConfig.Value("Forest", "Samples per Tree").c_str());
    if (samplesPerTree <= 0.0000001)
    {
      samplesPerTree = 1.0;
    }
    MITK_INFO << "Samples per Tree: " << samplesPerTree;
    int sampleWithReplacement = allConfig.IntValue("Forest", "Sample with replacement",1);
    double trainPrecision = atof(allConfig.Value("Forest", "Precision").c_str());
    if (trainPrecision <= 0.0000000001)
    {
      trainPrecision = 0.0;
    }
    double weightLambda = atof(allConfig.Value("Forest", "Weight Lambda").c_str());
    if (weightLambda <= 0.0000000001)
    {
      weightLambda = 0.0;
    }
    int maximumTreeDepth =  allConfig.IntValue("Forest", "Maximum Tree Depth",10000);
    // TODO int randomSplit = allConfig.IntValue("Forest","Use RandomSplit",0);
    //////////////////////////////////////////////////////////////////////////////
    // Read Statistic Parameter
    //////////////////////////////////////////////////////////////////////////////
    std::string statisticFilePath = allConfig.Value("Evaluation", "Statistic output file");
    std::string statisticShortFilePath = allConfig.Value("Evaluation", "Statistic short output file");
    std::string statisticShortFileLabel = allConfig.Value("Evaluation", "Index for short file");
    std::string statisticGoldStandard = allConfig.Value("Evaluation", "Gold Standard Name","GTV");
    // TODO bool statisticWithHeader = allConfig.IntValue("Evaluation", "Write header in short file",0);
    std::vector<std::string> labelGroupA = allConfig.Vector("LabelsA",0);
    std::vector<std::string> labelGroupB = allConfig.Vector("LabelsB",0);
    //////////////////////////////////////////////////////////////////////////////
    // Read Special Parameter
    //////////////////////////////////////////////////////////////////////////////
    bool useWeightedPoints = allConfig.IntValue("Forest", "Use point-based weighting",0);
    // TODO bool writePointsToFile = allConfig.IntValue("Forest", "Write points to file",0);
    // TODO int importanceWeightAlgorithm = allConfig.IntValue("Forest","Importance weight Algorithm",0);
    std::string importanceWeightName = allConfig.Value("Forest","Importance weight name","");

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
    usedModalities.push_back(importanceWeightName);

    if (trainSingleDataset > 0)
    {
      trainPatients.clear();
      trainPatients.push_back(singleTrainDatasetName);
    }

    mitk::CollectionReader* colReader = new mitk::CollectionReader();
    colReader->AddDataElementIds(trainPatients);
    colReader->SetDataItemNames(usedModalities);
    //colReader->SetNames(usedModalities);
    mitk::DataCollection::Pointer trainCollection;
    if (doTraining)
    {
      trainCollection = colReader->LoadCollection(trainingCollectionPath);
    }

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

    /*
    if (writePointsToFile)
    {
    MITK_INFO << "Use external weights...";
    mitk::ExternalWeighting weightReader;
    weightReader.SetModalities(modalities);
    weightReader.SetTestCollection(testCollection);
    weightReader.SetTrainCollection(trainCollection);
    weightReader.SetTestMask(testMask);
    weightReader.SetTrainMask(trainMask);
    weightReader.SetWeightsName("weights");
    weightReader.SetCorrectionFactor(1.0);
    weightReader.SetWeightFileName(writeDataFilePath);
    weightReader.WriteData();
    return 0;
    }*/

    //////////////////////////////////////////////////////////////////////////////
    // If required do Training....
    //////////////////////////////////////////////////////////////////////////////
    //mitk::DecisionForest forest;

    mitk::VigraRandomForestClassifier::Pointer forest = mitk::VigraRandomForestClassifier::New();
    forest->SetSamplesPerTree(samplesPerTree);
    forest->SetMinimumSplitNodeSize(minimumSplitNodeSize);
    forest->SetTreeCount(numberOfTrees);
    forest->UseSampleWithReplacement(sampleWithReplacement);
    forest->SetPrecision(trainPrecision);
    forest->SetMaximumTreeDepth(maximumTreeDepth);
    forest->SetWeightLambda(weightLambda);

    // TODO forest.UseRandomSplit(randomSplit);

    if (doTraining)
    {
      // 0 = LR-Estimation
      // 1 = KNN-Estimation
      // 2 = Kliep
      // 3 = Extern Image
      // 4 = Zadrozny
      // 5 = Spectral
      // 6 = uLSIF
      auto trainDataX = mitk::DCUtilities::DC3dDToMatrixXd(trainCollection, modalities, trainMask);
      auto trainDataY = mitk::DCUtilities::DC3dDToMatrixXi(trainCollection, trainMask, trainMask);

      if (useWeightedPoints)
      //if (false)
      {
        MITK_INFO << "Activated Point-based weighting...";
        //forest.UseWeightedPoints(true);
        forest->UsePointWiseWeight(true);
        //forest.SetWeightName("calculated_weight");
        /*if (importanceWeightAlgorithm == 1)
        {
        mitk::KNNDensityEstimation est;
        est.SetCollection(trainCollection);
        est.SetTrainMask(trainMask);
        est.SetTestMask(testMask);
        est.SetModalities(modalities);
        est.SetWeightName("calculated_weight");
        est.Update();
        } else if (importanceWeightAlgorithm == 2)
        {
        mitk::KliepDensityEstimation est;
        est.SetCollection(trainCollection);
        est.SetTrainMask(trainMask);
        est.SetTestMask(testMask);
        est.SetModalities(modalities);
        est.SetWeightName("calculated_weight");
        est.Update();
        } else if (importanceWeightAlgorithm == 3)
        {
        forest.SetWeightName(importanceWeightName);
        } else if (importanceWeightAlgorithm == 4)
        {
        mitk::ZadroznyWeighting est;
        est.SetCollection(trainCollection);
        est.SetTrainMask(trainMask);
        est.SetTestMask(testMask);
        est.SetModalities(modalities);
        est.SetWeightName("calculated_weight");
        est.Update();
        } else if (importanceWeightAlgorithm == 5)
        {
        mitk::SpectralDensityEstimation est;
        est.SetCollection(trainCollection);
        est.SetTrainMask(trainMask);
        est.SetTestMask(testMask);
        est.SetModalities(modalities);
        est.SetWeightName("calculated_weight");
        est.Update();
        } else if (importanceWeightAlgorithm == 6)
        {
        mitk::ULSIFDensityEstimation est;
        est.SetCollection(trainCollection);
        est.SetTrainMask(trainMask);
        est.SetTestMask(testMask);
        est.SetModalities(modalities);
        est.SetWeightName("calculated_weight");
        est.Update();
        } else*/
        {
          mitk::LRDensityEstimation est;
          est.SetCollection(trainCollection);
          est.SetTrainMask(trainMask);
          est.SetTestMask(testMask);
          est.SetModalities(modalities);
          est.SetWeightName("calculated_weight");
          est.Update();
        }
        auto trainDataW = mitk::DCUtilities::DC3dDToMatrixXd(trainCollection, "calculated_weight", trainMask);
        forest->SetPointWiseWeight(trainDataW);
        forest->UsePointWiseWeight(true);
      }
      MITK_INFO << "Start training the forest";
      forest->Train(trainDataX, trainDataY);

      MITK_INFO << "Save Forest";
      mitk::IOUtil::Save(forest, forestPath);
    } else
    {
      forest = mitk::IOUtil::Load<mitk::VigraRandomForestClassifier>(forestPath);// TODO forest.Load(forestPath);
    }

    time(&now);
    seconds =  std::difftime(now, lastTimePoint);
    MITK_INFO << "Duration for Training: " << seconds;
    timingFile << seconds << ";";
    time(&lastTimePoint);
    //////////////////////////////////////////////////////////////////////////////
    // If required do Save Forest....
    //////////////////////////////////////////////////////////////////////////////

    //writer.// (forest);
    /*
    auto w = forest->GetTreeWeights();
    w(0,0) = 10;
    forest->SetTreeWeights(w);*/

    //////////////////////////////////////////////////////////////////////////////
    // If required do test
    //////////////////////////////////////////////////////////////////////////////
    MITK_INFO << "Convert Test data";
    auto testDataX = mitk::DCUtilities::DC3dDToMatrixXd(testCollection,modalities, testMask);

    MITK_INFO << "Predict Test Data";
    auto testDataNewY = forest->Predict(testDataX);
    auto testDataNewProb = forest->GetPointWiseProbabilities();
    //MITK_INFO << testDataNewY;

    auto maxClassValue = testDataNewProb.cols();
    std::vector<std::string> names;
    for (int i = 0; i < maxClassValue; ++i)
    {
      std::string name = resultProb + std::to_string(i);
      MITK_INFO << name;
      names.push_back(name);
    }
    //names.push_back("prob-1");
    //names.push_back("prob-2");

    mitk::DCUtilities::MatrixToDC3d(testDataNewY, testCollection, resultMask, testMask);
    mitk::DCUtilities::MatrixToDC3d(testDataNewProb, testCollection, names, testMask);
    MITK_INFO << "Converted predicted data";
    //forest.SetMaskName(testMask);
    //forest.SetCollection(testCollection);
    //forest.Test();
    //forest.PrintTree(0);

    time(&now);
    seconds =  std::difftime(now, lastTimePoint);
    timingFile << seconds << ";";
    time(&lastTimePoint);

    //////////////////////////////////////////////////////////////////////////////
    // Cost-based analysis
    //////////////////////////////////////////////////////////////////////////////

    // TODO Reactivate
    //MITK_INFO << "Calculate Cost-based Statistic ";
    //mitk::CostingStatistic costStat;
    //costStat.SetCollection(testCollection);
    //costStat.SetCombinedA("combinedHealty");
    //costStat.SetCombinedB("combinedTumor");
    //costStat.SetCombinedLabel("combinedLabel");
    //costStat.SetMaskName(testMask);
    ////std::vector<std::string> labelHealthy;
    ////labelHealthy.push_back("result_prop_Class-0");
    ////labelHealthy.push_back("result_prop_Class-4");
    ////std::vector<std::string> labelTumor;
    ////labelTumor.push_back("result_prop_Class-1");
    ////labelTumor.push_back("result_prop_Class-2");
    ////labelTumor.push_back("result_prop_Class-3");
    //costStat.SetProbabilitiesA(labelGroupA);
    //costStat.SetProbabilitiesB(labelGroupB);

    //std::ofstream costStatisticFile;
    //costStatisticFile.open((statisticFilePath + ".cost").c_str(), std::ios::app);
    //std::ofstream lcostStatisticFile;

    //lcostStatisticFile.open((statisticFilePath + ".longcost").c_str(), std::ios::app);
    //costStat.WriteStatistic(lcostStatisticFile,costStatisticFile,2.5,statisticShortFileLabel);
    //costStatisticFile.close();

    //costStat.CalculateClass(50);

    //////////////////////////////////////////////////////////////////////////////
    // Save results to folder
    //////////////////////////////////////////////////////////////////////////////
    ////std::vector<std::string> outputFilter;
    //outputFilter.push_back(resultMask);
    //std::vector<std::string> propNames = forest.GetListOfProbabilityNames();
    //outputFilter.insert(outputFilter.begin(), propNames.begin(), propNames.end());
    MITK_INFO << "Write Result to HDD";
    mitk::CollectionWriter::ExportCollectionToFolder(testCollection,
      outputFolder + "/result_collection.xml",
      outputFilter);

    MITK_INFO << "Calculate Statistic...." ;
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
    stat.Print(statisticFile,sstatisticFile,true, statisticShortFileLabel);
    statisticFile.close();

    time(&now);
    seconds =  std::difftime(now, lastTimePoint);
    timingFile << seconds << std::endl;
    time(&lastTimePoint);
    timingFile.close();
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
