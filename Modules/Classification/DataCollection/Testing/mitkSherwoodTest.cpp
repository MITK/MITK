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

#include <mitkTestingMacros.h>
#include <mitkImageDataCollections.h>
#include <mitkImageGenerator.h>
#include <mitkImageDataCollections.h>
#include <mitkSWFeatureResponse.h>
#include <mitkSWTrainingContext.h>

#include <mitkSWForestExample.h>

#include <itkFeatureImageFilter.h>

#include <Forest.h>
#include <ForestTrainer.h>
#include <StatisticsAggregators.h>
#include <TrainingParameters.h>
#include <mitkIOUtil.h>
#include <iostream>
#include <sstream>

#include <mitkSWForest.h>
#include <mitkParallelForestTrainer.h>

#include <itkSWHistogramToImageFilter.h>


class mitkSherwoodTestClass
{
public:
  std::unique_ptr< mitk::SWForest > m_Forest;
  mitk::ImageDataCollections m_TrainingData;
  mitk::ImageDataCollections m_TestData;

  unsigned int m_NumberOfSets;
  unsigned int m_NumberOfModalities;
  unsigned int m_Trees;
  std::vector<std::string> m_TrainingDataPaths;
  std::vector<std::string> m_TestDataPaths;

  void LoadForest(std::string path)
  {
    m_Forest = mitk::SWForest::ReadFromFile(path);
  }

  void SaveForest(std::string path)
  {
    m_Forest->PrintToFile(path);
  }

  void GenerateTrainingData()
  {
    m_TrainingData.SetNumberOfSets(m_NumberOfSets);
    m_TrainingData.SetNumberOfModalities(m_NumberOfModalities);
    for (int set = 0; set < m_NumberOfSets; ++set)
    {
      for (int modality = 0; modality < m_NumberOfModalities; ++modality)
      {
        mitk::Image::Pointer image = mitk::ImageGenerator::GenerateGradientImage<float>(5,5,5);
        m_TrainingData.AddImage(set,modality, image);
      }
    }
  }

  void TrainForest()
  {
    mitk::SWRandom random;

    // Definde Training parameters0
    mitk::SWTrainingParameters parameters;
    parameters.NumberOfTrees = m_Trees;
    parameters.NumberOfCandidateFeatures = 10;
    parameters.NumberOfCandidateThresholdsPerFeature = 200;
    parameters.MaxDecisionLevels = 19;
    parameters.Verbose = false;

    //
    mitk::SWTrainingContext trainingContext;
    trainingContext.SetClasses(4);
    trainingContext.SetModalities(m_TrainingData.GetNumberOfModalities());

    mitk::SWHistogramAggregator hist = trainingContext.GetStatisticsAggregator();
    for (int i = 0; i < m_TrainingData.Count(); ++i)
      {
        if (m_TrainingData.IsLabeled(i))
        {
          hist.Aggregate(m_TrainingData, i);
        }
      }
    trainingContext.SetStatistic(hist);
    mitk::SWForestTrainer forestTrainer;
    mitk::ParallelForestTrainer<mitk::ImageDataCollections, mitk::SWFeatureContainer,mitk::SWHistogramAggregator> trainer;

    trainer.SetTreeNumber(parameters.NumberOfTrees);

    m_Forest = trainer.TrainForest (random, parameters, trainingContext, m_TrainingData);
  }

  void ApplyForest(std::string labelPath, std::string probPath)
  {
    typedef itk::Image<unsigned char, 3> LabelImageType;
    typedef itk::Image<float, 3>         PropImageType;

    typedef itk::SWHistogramToImageFilter<LabelImageType, PropImageType> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetForest(m_Forest);
    filter->SetCollection(m_TestData);
    filter->SetSet(0);
    filter->Update();
    LabelImageType::Pointer labelImage = filter->GetLabelImage();
    PropImageType::Pointer propImage = filter->GetPropabilityImage();

    mitk::Image::Pointer mLabelImage = mitk::Image::New();
    mitk::Image::Pointer mPropImage = mitk::Image::New();
    mitk::CastToMitkImage(labelImage, mLabelImage);
    mitk::CastToMitkImage(propImage, mPropImage);

    mitk::IOUtil::SaveImage(mLabelImage, labelPath);
    mitk::IOUtil::SaveImage(mPropImage, probPath);
  }



  void ParseCommandline(int argc, char* argv[])
  {
    m_TrainingDataPaths.clear();
    m_TestDataPaths.clear();
    for (int i = 0; i < argc; i++)
    {
      if ((std::string(argv[i]) == "--trees" || std::string(argv[i]) == "-t") && i < argc-1)
      {
        MITK_INFO << "trees";
        m_Trees = atoi(argv[i+1]);
        i++;
      }
      if ((std::string(argv[i]) == "--sets" || std::string(argv[i]) == "-s") && i < argc-1)
      {
        MITK_INFO << "sets";
        m_NumberOfSets = atoi(argv[i+1]);
        i++;
      }
      if ((std::string(argv[i]) == "--modalities" || std::string(argv[i]) == "-m") && i < argc-1)
      {
        MITK_INFO << "modalities";
        m_NumberOfModalities = atoi(argv[i+1]);
        i++;
      }
      if (std::string(argv[i]) == "--generate-training-data")
      {
        MITK_INFO << "generate-training-data";
        this->GenerateTrainingData();
      }
      if (std::string(argv[i]) == "--test-equal-training" || (std::string(argv[i]) == "-tet"))
      {
        MITK_INFO << "test-equal-training";
        m_TestData = m_TrainingData;
      }
      if ((std::string(argv[i]) == "--read-forest" || std::string(argv[i]) == "-r") && i < argc-1)
      {
        MITK_INFO << "read-forest";
        this->LoadForest(argv[i+1]);
        i++;
      }
      if ((std::string(argv[i]) == "--save-forest" || std::string(argv[i]) == "-s") && i < argc-1)
      {
        MITK_INFO << "save-forest";
        this->SaveForest(argv[i+1]);
        i++;
      }
      if (std::string(argv[i]) == "--train")
      {
        MITK_INFO << "train";
        m_TrainingData.GetDataPoint(0,0);
        this->TrainForest();
      }
      if (std::string(argv[i]) == "--apply" && i < argc-2)
      {
        MITK_INFO << "apply";
        this->ApplyForest(argv[i+1],argv[i+2]);
        i++;i++;
      }
      if (std::string(argv[i]) == "--testdata")
      {
        MITK_INFO << "testdata";
        m_TestData.SetNumberOfSets(m_NumberOfSets);
        m_TestData.SetNumberOfModalities(m_NumberOfModalities);
        for (int set = 0; set < 1; ++set)
        {
          for (int modality = 0; modality < m_NumberOfModalities; ++modality)
          {
            ++i;
            if ( i >= argc || argv[i][0] == '-')
              break;
            mitk::Image::Pointer image = mitk::IOUtil::LoadImage(argv[i]);
            m_TestData.AddImage(set,modality, image);
          }
          if ( i >= argc || argv[i][0] == '-')
            break;
        }
      }
      if (std::string(argv[i]) == "--traindata")
      {
        MITK_INFO << "traindata";
        m_TrainingData.SetNumberOfSets(m_NumberOfSets);
        m_TrainingData.SetNumberOfModalities(m_NumberOfModalities);
        for (int set = 0; set < m_NumberOfSets; ++set)
        {
          for (int modality = 0; modality < m_NumberOfModalities; ++modality)
          {
            ++i;
            if ( i >= argc || argv[i][0] == '-')
              break;
            mitk::Image::Pointer image = mitk::IOUtil::LoadImage(argv[i]);
            m_TrainingData.AddImage(set,modality, image);
          }
          ++i;
          if ( i >= argc || argv[i][0] == '-')
            break;
          mitk::Image::Pointer image = mitk::IOUtil::LoadImage(argv[i]);
          m_TrainingData.SetLabelImage(set, image);
        }
      }
    }
  }
};


int mitkSherwoodTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkSherwoodTest");

  mitkSherwoodTestClass test;
  test.ParseCommandline(argc,argv);


    // Basically to test features
  bool btest = false;
  if (btest)
  {
    typedef itk::Image<float, 3> ImageType;
    typedef itk::FeatureImageFilter<ImageType, ImageType> Zikic1FilterType;

    Zikic1FilterType::Pointer zikic1 = Zikic1FilterType::New();
  }





  MITK_TEST_CONDITION_REQUIRED(true, "Message");
  MITK_TEST_END();
  MITK_INFO << "Press Key and Enter to leave test";
  char ch;
  std::cin >> ch;
}