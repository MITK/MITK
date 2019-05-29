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

// MITK - DataCollection
#include <mitkDiffusionCollectionReader.h>
#include <mitkDiffusionCollectionWriter.h>
#include <mitkDataCollection.h>
#include <mitkImageCast.h>

#include "mitkDataCollectionImageIterator.h"
#include <mitkCollectionStatistic.h>
#include <mitkTumorInvasionClassification.h>
// CTK
#include "mitkCommandLineParser.h"
// ITK
#include <itkImageRegionIterator.h>

using namespace std;

int main(int argc, char *argv[])
{
  // Setup CLI Module parsable interface
  mitkCommandLineParser parser;
  parser.setTitle("Tumor Invasion Analysis");
  parser.setCategory("Tumor Analysis");
  parser.setDescription("Learns and predicts Invasion behavior");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Show options");
  parser.addArgument("loadFile", "l", mitkCommandLineParser::File, "DataCollection File", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "colIds", "c", mitkCommandLineParser::String, "Patient Identifiers from DataCollection used for training");
  parser.addArgument(
    "testId", "t", mitkCommandLineParser::String, "Patient Identifier from DataCollection used for testing");
  parser.addArgument("features", "b", mitkCommandLineParser::String, "Features");
  parser.addArgument("stats", "s", mitkCommandLineParser::String, "Output file for stats");
  parser.addArgument("ratio", "q", mitkCommandLineParser::Float, "ratio of tumor to healthy");
  parser.addArgument("treeDepth", "d", mitkCommandLineParser::Int, "limits tree depth");
  parser.addArgument("forestSize", "f", mitkCommandLineParser::Int, "number of trees");
  parser.addArgument("samplingMode", "m", mitkCommandLineParser::Int, "mode of sample selection");
  parser.addArgument("configName", "n", mitkCommandLineParser::String, "human readable name for configuration");
  parser.addArgument("output", "o", mitkCommandLineParser::Directory, "output folder for results", "", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("forest", "t", mitkCommandLineParser::File, "store trained forest to file", "", us::Any(), false, false, false, mitkCommandLineParser::Output);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  // Show a help message
  if (parsedArgs.size() == 0)
    return EXIT_SUCCESS;
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  // Default values
  float ratio = 1.0;
  bool useStatsFile = false;
  unsigned int forestSize = 250;
  unsigned int treeDepth = 0;
  unsigned int samplingMode = 1;
  std::string configName = "";
  std::string outputFolder = "";
  std::string forestFile = "";

  std::vector<std::string> features;
  std::vector<std::string> trainingIds;
  std::vector<std::string> testingIds;
  std::vector<std::string> loadIds; // features + masks needed for training and evaluation
  std::string outputFile;
  std::string xmlFile;
  std::ofstream experimentFS;

  // Parse input parameters
  {
    if (parsedArgs.count("colIds") || parsedArgs.count("c"))
    {
      std::istringstream ss(us::any_cast<string>(parsedArgs["colIds"]));
      std::string token;

      while (std::getline(ss, token, ','))
        trainingIds.push_back(token);
    }

    if (parsedArgs.count("output") || parsedArgs.count("o"))
    {
      outputFolder = us::any_cast<string>(parsedArgs["output"]);
    }

    if (parsedArgs.count("configName") || parsedArgs.count("n"))
    {
      configName = us::any_cast<string>(parsedArgs["configName"]);
    }

    if (parsedArgs.count("features") || parsedArgs.count("b"))
    {
      std::istringstream ss(us::any_cast<string>(parsedArgs["features"]));
      std::string token;

      while (std::getline(ss, token, ','))
        features.push_back(token);
    }

    if (parsedArgs.count("treeDepth") || parsedArgs.count("d"))
    {
      treeDepth = us::any_cast<int>(parsedArgs["treeDepth"]);
    }

    if (parsedArgs.count("ratio") || parsedArgs.count("q"))
    {
      ratio = us::any_cast<float>(parsedArgs["ratio"]);
    }

    if (parsedArgs.count("forestSize") || parsedArgs.count("f"))
    {
      forestSize = us::any_cast<int>(parsedArgs["forestSize"]);
    }

    if (parsedArgs.count("samplingMode") || parsedArgs.count("m"))
    {
      samplingMode = us::any_cast<int>(parsedArgs["samplingMode"]);
    }

    if (parsedArgs.count("stats") || parsedArgs.count("s"))
    {
      useStatsFile = true;
      experimentFS.open(us::any_cast<string>(parsedArgs["stats"]).c_str(), std::ios_base::app);
    }

    if (parsedArgs.count("forest") || parsedArgs.count("t"))
    {
      forestFile = us::any_cast<string>(parsedArgs["stats"]);
    }

    if (parsedArgs.count("testId") || parsedArgs.count("t"))
    {
      std::istringstream ss(us::any_cast<string>(parsedArgs["testId"]));
      std::string token;

      while (std::getline(ss, token, ','))
        testingIds.push_back(token);
    }

    for (unsigned int i = 0; i < features.size(); i++)
    {
      loadIds.push_back(features.at(i));
    }
    loadIds.push_back("GTV");
    loadIds.push_back("BRAINMASK");
    loadIds.push_back("TARGET");

    if (parsedArgs.count("stats") || parsedArgs.count("s"))
    {
      outputFile = us::any_cast<string>(parsedArgs["stats"]);
    }

    if (parsedArgs.count("loadFile") || parsedArgs.count("l"))
    {
      xmlFile = us::any_cast<string>(parsedArgs["loadFile"]);
    }
    else
    {
      MITK_ERROR << parser.helpText();
      return EXIT_FAILURE;
    }
  }

  mitk::DataCollection::Pointer trainCollection;
  mitk::DataCollection::Pointer testCollection;
  {
    mitk::DiffusionCollectionReader colReader;
    // Load only relevant images
    colReader.SetDataItemNames(loadIds);
    colReader.AddSubColIds(testingIds);
    testCollection = colReader.LoadCollection(xmlFile);
    colReader.ClearDataElementIds();
    colReader.ClearSubColIds();
    colReader.SetDataItemNames(loadIds);
    colReader.AddSubColIds(trainingIds);
    trainCollection = colReader.LoadCollection(xmlFile);
  }

  std::cout << "Setup Training" << std::endl;
  mitk::TumorInvasionClassification classifier;

  classifier.SetClassRatio(ratio);
  classifier.SetTrainMargin(7, 1);
  classifier.SamplesWeightingActivated(true);
  classifier.SelectTrainingSamples(trainCollection, samplingMode);
  // Learning stage
  std::cout << "Start Training" << std::endl;
  classifier.LearnProgressionFeatures(trainCollection, features, forestSize, treeDepth);

  if (forestFile != "")
    classifier.SaveRandomForest(forestFile);

  std::cout << "Start Predict" << std::endl;
  classifier.PredictInvasion(testCollection, features);

  if (false && outputFolder != "")
  {
    std::cout << "Saving files to " << outputFolder << std::endl;
    mitk::DiffusionCollectionWriter::ExportCollectionToFolder(trainCollection, "/tmp/dumple");
  }
  classifier.SanitizeResults(testCollection);

  {
    mitk::DataCollectionImageIterator<unsigned char, 3> gtvIt(testCollection, "GTV");
    mitk::DataCollectionImageIterator<unsigned char, 3> result(testCollection, "RESULTOPEN");

    while (!gtvIt.IsAtEnd())
    {
      if (gtvIt.GetVoxel() != 0)
      {
        result.SetVoxel(2);
      }

      result++;
      gtvIt++;
    }
  }

  mitk::CollectionStatistic stat2;
  mitk::ProgressionValueToIndexMapper progressionValueToIndexMapper;
  mitk::BinaryValueToIndexMapper binaryValueToIndexMapper;

  stat2.SetCollection(testCollection);
  stat2.SetClassCount(2);
  stat2.SetGoldName("TARGET");
  stat2.SetTestName("RESULTOPEN");
  stat2.SetMaskName("BRAINMASK");
  stat2.SetGroundTruthValueToIndexMapper(&binaryValueToIndexMapper);
  stat2.SetTestValueToIndexMapper(&progressionValueToIndexMapper);

  stat2.Update();
  stat2.ComputeRMSD();

  // FIXME: DICE value available after calling Print method
  std::ostringstream out2;
  stat2.Print(out2, std::cout, true);
  std::cout << std::endl << std::endl << out2.str() << std::endl;

  // Exclude GTV from Statistics by removing it from brain mask,
  // insert GTV as tumor region, since it is known before, in the result.
  {
    mitk::DataCollectionImageIterator<unsigned char, 3> gtvIt(testCollection, "GTV");
    mitk::DataCollectionImageIterator<unsigned char, 3> brainMaskIter(testCollection, "BRAINMASK");
    mitk::DataCollectionImageIterator<unsigned char, 3> result(testCollection, "RESULTOPEN");

    while (!gtvIt.IsAtEnd())
    {
      if (gtvIt.GetVoxel() != 0)
      {
        brainMaskIter.SetVoxel(0);
        result.SetVoxel(2);
      }
      result++;
      gtvIt++;
      brainMaskIter++;
    }
  }

  mitk::CollectionStatistic stat;
  stat.SetCollection(testCollection);
  stat.SetClassCount(2);
  stat.SetGoldName("TARGET");
  stat.SetTestName("RESULTOPEN");
  stat.SetMaskName("BRAINMASK");
  stat.SetGroundTruthValueToIndexMapper(&binaryValueToIndexMapper);
  stat.SetTestValueToIndexMapper(&progressionValueToIndexMapper);

  stat.Update();
  stat.ComputeRMSD();

  // WARN: DICE value computed within Print method, so values are only available
  // after
  // calling Print()
  std::ostringstream out;
  stat.Print(out, std::cout, true);
  std::cout << std::endl << std::endl << out.str() << std::endl;

  // Statistics for original GTV excluded (Dice,Sensitivity) and for Gold
  // Standard vs prediction (RMSE)
  mitk::StatisticData statData = stat.GetStatisticData(1).at(0);
  mitk::StatisticData statData2 = stat2.GetStatisticData(1).at(0);

  std::cout << "Writing Stats to file" << std::endl;
  // one line output
  if (useStatsFile)
  {
    experimentFS << "Tree_Depth " << treeDepth << ',';
    experimentFS << "Forest_Size " << forestSize << ',';
    experimentFS << "Tumor/healthy_ratio " << ratio << ',';
    experimentFS << "Sample_Selection " << samplingMode << ',';

    experimentFS << "Trainined_on: " << ',';
    for (unsigned int i = 0; i < trainingIds.size(); i++)
    {
      experimentFS << trainingIds.at(i) << "/";
    }
    experimentFS << ',';

    experimentFS << "Tested_on: " << ',';
    for (unsigned int i = 0; i < testingIds.size(); i++)
    {
      experimentFS << testingIds.at(i) << "/";
    }
    experimentFS << ',';

    experimentFS << "Features_used: " << ',';
    if (configName == "")
    {
      for (unsigned int i = 0; i < features.size(); i++)
      {
        experimentFS << features.at(i) << "/";
      }
    }
    else
      experimentFS << configName;

    experimentFS << ',';
    experimentFS << "---- STATS ---" << ',';
    experimentFS << " Sensitivity " << statData.m_Sensitivity << ',';
    experimentFS << " DICE " << statData.m_DICE << ',';
    experimentFS << " RMSE " << statData2.m_RMSD << ',';
    experimentFS << std::endl;
  }

  if (outputFolder != "")
  {
    std::cout << "Saving files to " << outputFolder << std::endl;
    mitk::DiffusionCollectionWriter::ExportCollectionToFolder(testCollection, outputFolder);
  }

  return EXIT_SUCCESS;
}
