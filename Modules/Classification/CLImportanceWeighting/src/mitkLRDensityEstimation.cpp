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

#include <mitkLRDensityEstimation.h>

#include <mitkDataCollectionImageIterator.h>
#include <mitkDataCollectionSingleImageIterator.h>

#include <itkImageRegionIterator.h>

#include <mitkGeneralizedLinearModel.h>

#include <mitkIOUtil.h>
#include "itkImageFileWriter.h"

static void EnsureDataImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target);
//static std::size_t CountElementsInCollection(mitk::DataCollection::Pointer collection, std::string maskName);

void
  mitk::LRDensityEstimation::SetCollection(DataCollection::Pointer data)
{
  m_Collection = data;
}

mitk::DataCollection::Pointer
  mitk::LRDensityEstimation::GetCollection()
{
  return m_Collection;
}

void
  mitk::LRDensityEstimation::SetTestMask(std::string name)
{
  m_TestMask = name;
}

std::string
  mitk::LRDensityEstimation::GetTestMask()
{
  return m_TestMask;
}

void
  mitk::LRDensityEstimation::SetTrainMask(std::string name)
{
  m_TrainMask = name;
}

std::string
  mitk::LRDensityEstimation::GetTrainMask()
{
  return m_TrainMask;
}

void
  mitk::LRDensityEstimation::SetWeightName(std::string name)
{
  m_WeightName = name;
}

std::string
  mitk::LRDensityEstimation::GetWeightName()
{
  return m_WeightName;
}

void
  mitk::LRDensityEstimation::Update()
{
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef itk::Image<double , 3> FeatureImageType;

  /// Ensuring that weighting image is present in data set.
  EnsureDataImageInCollection(m_Collection, m_TrainMask, m_WeightName);

  /// Create iterators for iterating over images
  DataCollectionSingleImageIterator<unsigned char, 3> train(m_Collection, m_TrainMask);
  DataCollectionSingleImageIterator<unsigned char, 3> test (m_Collection, m_TestMask);
  DataCollectionSingleImageIterator<double, 3> weight (m_Collection, m_WeightName);
  std::vector<DataCollectionSingleImageIterator<double, 3> > featureList;
  for (unsigned int i = 0; i < m_Modalities.size(); ++i)
  {
    DataCollectionSingleImageIterator<double, 3> iter(m_Collection, m_Modalities[i]);
    featureList.push_back(iter);
  }

  // Do for each image...
  //  char buchstabe = 'A';
  while (!train.IsAtEnd())
  {
    itk::ImageRegionIterator<MaskImageType> trainIter(train.GetImage(), train.GetImage()->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<MaskImageType> testIter(test.GetImage(), test.GetImage()->GetLargestPossibleRegion() );

    int trainElements = 0;
    int testElements = 0;
    while (!trainIter.IsAtEnd())
    {
      if (trainIter.Get() > 0)
      {
        ++trainElements;
      }
      if (testIter.Get() > 0)
      {
        ++testElements;
      }
      ++trainIter;
      ++testIter;
    }
    trainIter.GoToBegin();
    testIter.GoToBegin();

    std::vector<itk::ImageRegionIterator<FeatureImageType> > featureIter;
    for (unsigned int i = 0; i < featureList.size(); ++i)
    {
      itk::ImageRegionIterator<FeatureImageType> iter(featureList[i].GetImage(), featureList[i].GetImage()->GetLargestPossibleRegion() );
      featureIter.push_back(iter);
    }

    vnl_vector<double> label(trainElements + testElements);
    vnl_matrix<double> feature(trainElements + testElements, featureList.size() );
    vnl_matrix<double> trainFeature(trainElements, featureList.size() );

    int index = 0;
    int trainIndex = 0;
    while (!trainIter.IsAtEnd())
    {
      if (trainIter.Get() > 0)
      {
        label(index) = 1;
        for (unsigned int i = 0; i < featureIter.size(); ++i)
        {
          feature(index, i) = featureIter[i].Get();
          trainFeature(trainIndex, i) = featureIter[i].Get();
        }
        ++index;
        ++trainIndex;
      }
      if (testIter.Get() > 0)
      {
        label(index) = 0;
        for (unsigned int i = 0; i < featureIter.size(); ++i)
        {
          feature(index, i) = featureIter[i].Get();
        }
        ++index;
      }

      ++trainIter;
      ++testIter;
      for (unsigned int i = 0; i < featureIter.size(); ++i)
      {
        ++(featureIter[i]);
      }
    }

    mitk::GeneralizedLinearModel glm(feature, label);
    vnl_vector<double> weightVector = glm.ExpMu(trainFeature);

    itk::ImageRegionIterator<FeatureImageType> weightIter(weight.GetImage(), weight.GetImage()->GetLargestPossibleRegion() );

    trainIter.GoToBegin();
    index = 0;
    //double ratio = (trainElements * 1.0) / (testElements * 1.0);
    //ratio = 1.0 / weightVector.one_norm()*weightVector.size();
    while(!trainIter.IsAtEnd())
    {
      if (trainIter.Get() > 0)
      {
        weightIter.Set(weightVector(index) );
        ++index;
      }
      ++trainIter;
      ++weightIter;
    }

    //std::stringstream s;
    // s<<"d:/tmp/img/datei"<<buchstabe<<".nrrd";
    //std::string blub =s.str();
    //MITK_INFO << blub;
    //FeatureImageType::Pointer image = weight.GetImage();
    //typedef  itk::ImageFileWriter< FeatureImageType  > WriterType;
    //WriterType::Pointer writer = WriterType::New();
    //writer->SetFileName(blub);
    //writer->SetInput(image);
    //writer->Update();
    //++buchstabe;

    ++train;
    ++test;
    ++weight;
    for (unsigned int i = 0; i < featureList.size(); ++i)
    {
      ++(featureList[i]);
    }
  }
}

void
  mitk::LRDensityEstimation::SetModalities(std::vector<std::string> modalities)
{
  m_Modalities = modalities;
}

std::vector<std::string>
  mitk::LRDensityEstimation::GetModalities()
{
  return m_Modalities;
}

//static std::size_t CountElementsInCollection(mitk::DataCollection::Pointer collection, std::string maskName)
//{
//  std::size_t count = 0;
//  mitk::DataCollectionImageIterator<unsigned char,3> iter(collection, maskName);
//  while (!iter.IsAtEnd())
//  {
//    if (iter.GetVoxel() > 0)
//    {
//      ++count;
//    }
//    ++iter;
//  }
//  return count;
//}

static void EnsureDataImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target)
{
  typedef itk::Image<double, 3> FeatureImage;
  typedef itk::Image<unsigned char, 3> LabelImage;

  mitk::DataCollectionImageIterator<unsigned char , 3> iter( collection, origin);
  while (!iter.IsAtEnd())
  {
    ++iter;
  }

  if (collection->HasElement(origin))
  {
    LabelImage::Pointer originImage = dynamic_cast<LabelImage*>(collection->GetData(origin).GetPointer());
    if (!collection->HasElement(target) && originImage.IsNotNull())
    {
      MITK_INFO << "New image necessary";
      FeatureImage::Pointer image = FeatureImage::New();
      image->SetRegions(originImage->GetLargestPossibleRegion());
      image->SetSpacing(originImage->GetSpacing());
      image->SetOrigin(originImage->GetOrigin());
      image->SetDirection(originImage->GetDirection());
      image->Allocate();

      collection->AddData(dynamic_cast<itk::DataObject*>(image.GetPointer()),target,"");
    }
  }
  for (std::size_t i = 0; i < collection->Size();++i)
  {
    mitk::DataCollection* newCol = dynamic_cast<mitk::DataCollection*>(collection->GetData(i).GetPointer());
    if (newCol != nullptr)
    {
      EnsureDataImageInCollection(newCol, origin, target);
    }
  }
}

void
  mitk::LRDensityEstimation::WeightsForAll(mitk::DataCollection::Pointer train, mitk::DataCollection::Pointer test)
{
  /// Ensuring that weighting image is present in data set.
  EnsureDataImageInCollection(train, m_TrainMask, m_WeightName);

  /// Create iterators for iterating over images
  DataCollectionImageIterator<unsigned char, 3> trainIter(train, m_TrainMask);
  DataCollectionImageIterator<unsigned char, 3> testIter (test, m_TestMask);
  DataCollectionImageIterator<double, 3> weightIter (train, m_WeightName);
  std::vector<DataCollectionImageIterator<double, 3> > trainFeatureList;
  std::vector<DataCollectionImageIterator<double, 3> > testFeatureList;
  for (unsigned int i = 0; i < m_Modalities.size(); ++i)
  {
    DataCollectionImageIterator<double, 3> iter(train, m_Modalities[i]);
    trainFeatureList.push_back(iter);
    DataCollectionImageIterator<double, 3> iter2(train, m_Modalities[i]);
    testFeatureList.push_back(iter2);
  }

  int trainElements = 0;
  int testElements = 0;
  while (!trainIter.IsAtEnd())
  {
    if (trainIter.GetVoxel() > 0)
      ++trainElements;
    ++trainIter;
  }
  while (!testIter.IsAtEnd() )
  {
    if (testIter.GetVoxel() > 0)
      ++testElements;
    ++testIter;
  }
  trainIter.ToBegin();
  testIter.ToBegin();

  vnl_vector<double> label(trainElements + testElements);
  vnl_matrix<double> feature(trainElements + testElements, trainFeatureList.size() );
  vnl_matrix<double> trainFeature(trainElements, trainFeatureList.size() );

  int trainIndex = 0;
  int testIndex = 0;
  while (!trainIter.IsAtEnd())
  {
    if (trainIter.GetVoxel() > 0)
    {
      label(trainIndex) = 1;
      for (unsigned int i = 0; i < trainFeatureList.size(); ++i)
      {
        feature(trainIndex, i) = trainFeatureList[i].GetVoxel();
        trainFeature(trainIndex, i) = trainFeatureList[i].GetVoxel();
      }

      ++trainIndex;
    }

    ++trainIter;
    for (unsigned int i = 0; i < trainFeatureList.size(); ++i)
    {
      ++(trainFeatureList[i]);
    }
  }

  while (!testIter.IsAtEnd())
  {
    if (testIter.GetVoxel() > 0)
    {
      label(trainIndex) = 1;
      for (unsigned int i = 0; i < testFeatureList.size(); ++i)
      {
        feature(trainIndex+testIndex, i) = testFeatureList[i].GetVoxel();
      }
      ++testIndex;
    }

    ++testIter;
    for (unsigned int i = 0; i < testFeatureList.size(); ++i)
    {
      ++(testFeatureList[i]);
    }
  }

  mitk::GeneralizedLinearModel glm(feature, label);
  vnl_vector<double> weightVector = glm.ExpMu(trainFeature);

  trainIter.ToBegin();
  int index = 0;
  while (!trainIter.IsAtEnd())
  {
    if (trainIter.GetVoxel() > 0)
    {
      weightIter.SetVoxel(weightVector(index));
      ++index;
    }
    ++trainIter;
    ++weightIter;
  }
}

void mitk::LRDensityEstimation::Predict()
{
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef itk::Image<double , 3> FeatureImageType;

  /// Ensuring that weighting image is present in data set.
  EnsureDataImageInCollection(m_Collection, m_TrainMask, m_WeightName);

  /// Create iterators for iterating over images
  DataCollectionSingleImageIterator<unsigned char, 3> train(m_Collection, m_TrainMask);
  DataCollectionSingleImageIterator<unsigned char, 3> test (m_Collection, m_TestMask);
  DataCollectionSingleImageIterator<double, 3> weight (m_Collection, m_WeightName);
  std::vector<DataCollectionSingleImageIterator<double, 3> > featureList;
  for (unsigned int i = 0; i < m_Modalities.size(); ++i)
  {
    DataCollectionSingleImageIterator<double, 3> iter(m_Collection, m_Modalities[i]);
    featureList.push_back(iter);
  }

  while (!train.IsAtEnd())
  {
    itk::ImageRegionIterator<MaskImageType> trainIter(train.GetImage(), train.GetImage()->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<MaskImageType> testIter(test.GetImage(), test.GetImage()->GetLargestPossibleRegion() );

    int trainElements = 0;
    int testElements = 0;
    while (!trainIter.IsAtEnd())
    {
      if (trainIter.Get() > 0)
      {
        ++trainElements;
      }
      if (testIter.Get() > 0)
      {
        ++testElements;
      }
      ++trainIter;
      ++testIter;
    }
    trainIter.GoToBegin();
    testIter.GoToBegin();

    std::vector<itk::ImageRegionIterator<FeatureImageType> > featureIter;
    for (unsigned int i = 0; i < featureList.size(); ++i)
    {
      itk::ImageRegionIterator<FeatureImageType> iter(featureList[i].GetImage(), featureList[i].GetImage()->GetLargestPossibleRegion() );
      featureIter.push_back(iter);
    }

    vnl_vector<double> label(trainElements + testElements);
    vnl_matrix<double> feature(trainElements + testElements, featureList.size() );
    vnl_matrix<double> trainFeature(trainElements, featureList.size() );

    int index = 0;
    int trainIndex = 0;
    while (!trainIter.IsAtEnd())
    {
      if (trainIter.Get() > 0)
      {
        label(index) = 1;
        for (unsigned int i = 0; i < featureIter.size(); ++i)
        {
          feature(index, i) = featureIter[i].Get();
          trainFeature(trainIndex, i) = featureIter[i].Get();
        }
        ++index;
        ++trainIndex;
      }
      if (testIter.Get() > 0)
      {
        label(index) = 0;
        for (unsigned int i = 0; i < featureIter.size(); ++i)
        {
          feature(index, i) = featureIter[i].Get();
        }
        ++index;
      }

      ++trainIter;
      ++testIter;
      for (unsigned int i = 0; i < featureIter.size(); ++i)
      {
        ++(featureIter[i]);
      }
    }

    mitk::GeneralizedLinearModel glm(feature, label);
    vnl_vector<double> weightVector = glm.Predict(trainFeature);

    itk::ImageRegionIterator<FeatureImageType> weightIter(weight.GetImage(), weight.GetImage()->GetLargestPossibleRegion() );

    trainIter.GoToBegin();
    index = 0;

    while(!trainIter.IsAtEnd())
    {
      if (trainIter.Get() > 0)
      {
        weightIter.Set(weightVector(index) );
        ++index;
      }
      ++trainIter;
      ++weightIter;
    }

    ++train;
    ++test;
    ++weight;
    for (unsigned int i = 0; i < featureList.size(); ++i)
    {
      ++(featureList[i]);
    }
  }
}
