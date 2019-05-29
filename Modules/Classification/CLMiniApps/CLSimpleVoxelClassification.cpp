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
#include <mitkVigraRandomForestClassifier.h>

// CTK
#include "mitkCommandLineParser.h"




int main(int argc, char* argv[])
{
  // Setup CLI Module parsable interface
  mitkCommandLineParser parser;
  parser.setTitle("Simple Random Forest Classifier");
  parser.setCategory("Classification");
  parser.setDescription("Learns and predicts classes");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Show options");
  parser.addArgument("loadFile", "l", mitkCommandLineParser::File,
                     "DataCollection File", "", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
        "colIds", "c", mitkCommandLineParser::String,
        "Patient Identifiers from DataCollection used for training");
  parser.addArgument("testId", "t", mitkCommandLineParser::String,
                     "Patient Identifier from DataCollection used for testing");
  parser.addArgument("features", "b", mitkCommandLineParser::String,
                     "Features");
  parser.addArgument("stats", "s", mitkCommandLineParser::String,
                     "Output file for stats");
  parser.addArgument("treeDepth", "d", mitkCommandLineParser::Int,
                     "limits tree depth");
  parser.addArgument("forestSize", "f", mitkCommandLineParser::Int,
                     "number of trees");
  parser.addArgument("configName", "n", mitkCommandLineParser::String,
                     "human readable name for configuration");
  parser.addArgument("output", "o", mitkCommandLineParser::Directory,
                     "output folder for results", "", us::Any(), true, false, false, mitkCommandLineParser::Input);

  parser.addArgument("classmap", "m", mitkCommandLineParser::String,
                     "name of class that is to be learnt");


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  // Show a help message
  if (parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h")) {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  // Default values
  unsigned int forestSize = 8;
  unsigned int treeDepth = 10;
  std::string configName = "";
  std::string outputFolder = "";

  std::vector<std::string> features;
  std::vector<std::string> trainingIds;
  std::vector<std::string> testingIds;
  std::vector<std::string> loadIds; // features + masks needed for training and evaluation
  std::string outputFile;
  std::string classMap;
  std::string xmlFile;
  std::ofstream experimentFS;

  // Parse input parameters
  {
    if (parsedArgs.count("colIds") || parsedArgs.count("c")) {
      std::istringstream ss(us::any_cast<std::string>(parsedArgs["colIds"]));
      std::string token;

      while (std::getline(ss, token, ','))
        trainingIds.push_back(token);
    }

    if (parsedArgs.count("output") || parsedArgs.count("o")) {
      outputFolder = us::any_cast<std::string>(parsedArgs["output"]);
    }

    if (parsedArgs.count("classmap") || parsedArgs.count("m")) {
      classMap = us::any_cast<std::string>(parsedArgs["classmap"]);
    }

    if (parsedArgs.count("configName") || parsedArgs.count("n")) {
      configName = us::any_cast<std::string>(parsedArgs["configName"]);
    }

    if (parsedArgs.count("features") || parsedArgs.count("b")) {
      std::istringstream ss(us::any_cast<std::string>(parsedArgs["features"]));
      std::string token;

      while (std::getline(ss, token, ','))
        features.push_back(token);
    }

    if (parsedArgs.count("treeDepth") || parsedArgs.count("d")) {
      treeDepth = us::any_cast<int>(parsedArgs["treeDepth"]);
    }


    if (parsedArgs.count("forestSize") || parsedArgs.count("f")) {
      forestSize = us::any_cast<int>(parsedArgs["forestSize"]);
    }

    if (parsedArgs.count("stats") || parsedArgs.count("s")) {
      experimentFS.open(us::any_cast<std::string>(parsedArgs["stats"]).c_str(),
          std::ios_base::app);
    }


    if (parsedArgs.count("testId") || parsedArgs.count("t")) {
      std::istringstream ss(us::any_cast<std::string>(parsedArgs["testId"]));
      std::string token;

      while (std::getline(ss, token, ','))
        testingIds.push_back(token);
    }

    for (unsigned int i = 0; i < features.size(); i++) {
      loadIds.push_back(features.at(i));
    }
    loadIds.push_back(classMap);

    if (parsedArgs.count("stats") || parsedArgs.count("s")) {
      outputFile = us::any_cast<std::string>(parsedArgs["stats"]);
    }

    if (parsedArgs.count("loadFile") || parsedArgs.count("l")) {
      xmlFile = us::any_cast<std::string>(parsedArgs["loadFile"]);
    } else {
      MITK_ERROR << parser.helpText();
      return EXIT_FAILURE;
    }
  }

  mitk::DataCollection::Pointer trainCollection;
  mitk::DataCollection::Pointer testCollection;
  {
    mitk::CollectionReader colReader;
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

  //////////////////////////////////////////////////////////////////////////////
  // If required do Training....
  //////////////////////////////////////////////////////////////////////////////
  //mitk::DecisionForest forest;

  mitk::VigraRandomForestClassifier::Pointer forest = mitk::VigraRandomForestClassifier::New();

  forest->SetTreeCount(forestSize);
  forest->SetMaximumTreeDepth(treeDepth);


  // create feature matrix
  auto trainDataX = mitk::DCUtilities::DC3dDToMatrixXd(trainCollection, features, classMap);
  // create label matrix
  auto trainDataY = mitk::DCUtilities::DC3dDToMatrixXi(trainCollection, classMap, classMap);

  forest->Train(trainDataX, trainDataY);


  // prepare feature matrix for test case
  auto testDataX = mitk::DCUtilities::DC3dDToMatrixXd(testCollection,features, classMap);
  auto testDataNewY = forest->Predict(testDataX);


  mitk::DCUtilities::MatrixToDC3d(testDataNewY, testCollection, "RESULT", classMap);

  Eigen::MatrixXd Probs = forest->GetPointWiseProbabilities();


  Eigen::MatrixXd prob0 = Probs.col(0);
  Eigen::MatrixXd prob1 = Probs.col(1);

  mitk::DCUtilities::MatrixToDC3d(prob0, testCollection,"prob0", classMap);
  mitk::DCUtilities::MatrixToDC3d(prob1, testCollection,"prob1", classMap);


  std::vector<std::string> outputFilter;
  mitk::CollectionWriter::ExportCollectionToFolder(testCollection,
                                                   outputFolder + "/result_collection.xml",
                                                   outputFilter);
  return EXIT_SUCCESS;
}
