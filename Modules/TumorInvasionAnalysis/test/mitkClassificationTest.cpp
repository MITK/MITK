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
#pragma warning (disable : 4996)

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkCompareImageDataFilter.h"
#include "mitkIOUtil.h"

#include "mitkDataCollection.h"
#include "mitkCollectionReader.h"
#include "mitkCollectionWriter.h"

#include <vtkSmartPointer.h>
#include "mitkTumorInvasionClassification.h"

/**
 * @brief mitkClassificationTestSuite
 *
 * Tests mitkDecisionForest, mitkClassifyProgression, mitkDataCollection, mitkCollectionReader
 * \warn Reference is compared to results computed based on random forests, which might be a source of random test fails
 * such sporadic fails do represent total fails, as the result is no longer consitently under the provided margin.
 *
 */
class mitkClassificationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkClassificationTestSuite);
  MITK_TEST(TestClassification);
  CPPUNIT_TEST_SUITE_END();


public:

  /**
   * @brief Setup - Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void TestClassification()
  {
    size_t forestSize = 10;
    size_t treeDepth = 2;

    std::string train = GetTestDataFilePath("DiffusionImaging/ProgressionAnalysis/Classification/Train.xml");
    std::string eval = GetTestDataFilePath("DiffusionImaging/ProgressionAnalysis/Classification/Test.xml");


    std::vector<std::string> modalities;

    modalities.push_back("MOD0");
    modalities.push_back("MOD1");


    mitk::CollectionReader colReader;
    mitk::DataCollection::Pointer collection = colReader.LoadCollection(train);
    colReader.Clear();
    // read evaluation collection

    mitk::DataCollection::Pointer evaluation = colReader.LoadCollection(eval);

    mitk::TumorInvasionClassification progression;

    progression.SetClassRatio(1);
    progression.SetTrainMargin(4,0);
    progression.SetMaskID("MASK");

    progression.SelectTrainingSamples(collection);
    progression.LearnProgressionFeatures(collection, modalities, forestSize,treeDepth);

    progression.PredictInvasion(evaluation,modalities);

    mitk::Image::Pointer refImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/ProgressionAnalysis/Classification/TESTING_RESULT.nrrd"));

    mitk::DataCollection* patCol = dynamic_cast<mitk::DataCollection*> (evaluation->GetData(0).GetPointer());
    mitk::DataCollection* subCol = dynamic_cast<mitk::DataCollection*> (patCol->GetData(0).GetPointer());
    mitk::Image::Pointer resultImage = subCol->GetMitkImage("RESULT");

    // Test result against fixed reference.
    // Require more than 90% to be correct.
    // 10% margin due to
    // 1) unsure classification in transitional regions and
    // 2) stochastic training procedure

    // Total number of voxels 2400 -> 10% err -> 240 voxels margin

    mitk::CompareImageDataFilter::Pointer compareFilter = mitk::CompareImageDataFilter::New();
    compareFilter->SetInput(0, refImage.GetPointer());
    compareFilter->SetInput(1, resultImage);
    compareFilter->SetTolerance(.1);
    compareFilter->Update();

    MITK_TEST_CONDITION_REQUIRED(compareFilter->GetResult(240) , "Compare prediction results to reference image.")
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkClassification)
