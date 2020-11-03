/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations UI module
#include "QmitkStatisticsCalculator.h"

// semantic relations module
#include <mitkNodePredicates.h>
#include <mitkRelationStorage.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsDataStorageAccess.h>
#include <mitkSemanticRelationsInference.h>

// mitk image statistics module
#include <mitkImageStatisticsContainerManager.h>
#include <mitkImageStatisticsContainerNodeHelper.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>

QmitkStatisticsCalculator::QmitkStatisticsCalculator()
  : m_CalculationJob(nullptr)
  , m_DataStorage(nullptr)
  , m_MaskVolume(0.0)
{
  m_CalculationJob = new QmitkImageStatisticsCalculationJob();

  connect(m_CalculationJob, &QmitkImageStatisticsCalculationJob::finished, this,
    &QmitkStatisticsCalculator::OnStatisticsCalculationEnds, Qt::QueuedConnection);
}

QmitkStatisticsCalculator::~QmitkStatisticsCalculator()
{
  if (!m_CalculationJob->isFinished())
  {
    m_CalculationJob->terminate();
    m_CalculationJob->wait();
  }
  m_CalculationJob->deleteLater();
}

void QmitkStatisticsCalculator::ComputeLesionVolume(mitk::LesionData& lesionData, const mitk::SemanticTypes::CaseID& caseID)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  std::vector<double> lesionVolume;
  mitk::SemanticTypes::Lesion lesion = lesionData.GetLesion();
  double volume = 0.0;

  mitk::SemanticTypes::ControlPointVector controlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(caseID);
  // sort the vector of control points for the timeline
  std::sort(controlPoints.begin(), controlPoints.end());
  mitk::SemanticTypes::InformationTypeVector informationTypes = mitk::RelationStorage::GetAllInformationTypesOfCase(caseID);
  for (const auto& informationType : informationTypes)
  {
    for (const auto& controlPoint : controlPoints)
    {
      mitk::SemanticRelationsDataStorageAccess semanticRelationsDataStorageAccess(dataStorage);
      mitk::DataNode::Pointer specificImage;
      mitk::DataNode::Pointer specificSegmentation;
      try
      {
        specificSegmentation = semanticRelationsDataStorageAccess.GetSpecificSegmentation(caseID, controlPoint, informationType, lesion);
        if (nullptr == specificSegmentation)
        {
          volume = 0.0;
        }
        else
        {
          // get parent node of the specific segmentation node with the node predicate
          auto parentNodes = dataStorage->GetSources(specificSegmentation, mitk::NodePredicates::GetImagePredicate(), false);
          for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
          {
            specificImage = it->Value();
          }

          volume = GetSegmentationMaskVolume(specificImage, specificSegmentation);
        }
      }
      catch (mitk::SemanticRelationException&)
      {
        volume = 0.0;
      }

      lesionVolume.push_back(volume);
    }
  }

  lesionData.SetLesionVolume(lesionVolume);
}

double QmitkStatisticsCalculator::GetSegmentationMaskVolume(mitk::DataNode::Pointer imageNode, mitk::DataNode::Pointer segmentationNode)
{
  m_MaskVolume = 0.0;

  if (m_DataStorage.IsExpired())
  {
    return m_MaskVolume;
  }

  auto dataStorage = m_DataStorage.Lock();

  if (imageNode.IsNull() || segmentationNode.IsNull())
  {
    return m_MaskVolume;
  }

  m_ImageNode = imageNode;
  m_SegmentationNode = segmentationNode;

  auto image = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());
  auto segmentation = dynamic_cast<mitk::Image*>(m_SegmentationNode->GetData());
  if (nullptr == image || nullptr == segmentation)
  {
    return m_MaskVolume;
  }

  // all nodes and images are valid, retrieve statistics
  mitk::ImageStatisticsContainer::ConstPointer imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(dataStorage, image, segmentation).GetPointer();

  bool imageStatisticsOlderThanInputs = false;
  if (imageStatistics && (imageStatistics->GetMTime() < image->GetMTime() || (imageStatistics->GetMTime() < segmentation->GetMTime())))
  {
    imageStatisticsOlderThanInputs = true;
  }
  // statistics need to be (re)computed
  if (!imageStatistics || imageStatisticsOlderThanInputs)
  {
    m_CalculationJob->Initialize(image, segmentation, nullptr);
    try
    {
      m_CalculationJob->start();
      return m_MaskVolume;
    }
    catch (const std::exception&)
    {
      return m_MaskVolume;
    }
  }

  // use a valid statistics object to get the volume of the image-segmentation pair
  mitk::ImageStatisticsContainer::ImageStatisticsObject statisticsObject;
  try
  {
    statisticsObject = imageStatistics->GetStatisticsForTimeStep(0);
  }
  catch (mitk::Exception&)
  {
    return m_MaskVolume;
  }
  try
  {
    if (statisticsObject.HasStatistic(mitk::ImageStatisticsConstants::VOLUME()))
    {
      auto valueVariant = statisticsObject.GetValueNonConverted(mitk::ImageStatisticsConstants::VOLUME());
      m_MaskVolume = boost::get<double>(valueVariant);
    }
  }
  catch (mitk::Exception&)
  {
    return m_MaskVolume;
  }

  return m_MaskVolume;
}

void QmitkStatisticsCalculator::OnStatisticsCalculationEnds()
{
  // taken from 'QmitkImageStatisticsView' (see measurementtoolbox plugin)
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  if (m_CalculationJob->GetStatisticsUpdateSuccessFlag())
  {
    auto statistic = m_CalculationJob->GetStatisticsData();
    auto image = m_CalculationJob->GetStatisticsImage();
    mitk::BaseData::ConstPointer mask = nullptr;
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(statistic, image);

    if (m_CalculationJob->GetMaskImage())
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      mask = m_CalculationJob->GetMaskImage();
      maskRule->Connect(statistic, mask);
    }

    auto imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(dataStorage, image, mask);

    // if statistics base data already exist: add to existing node
    if (nullptr != imageStatistics)
    {
      auto allDataNodes = dataStorage->GetAll()->CastToSTLConstContainer();
      for (auto node : allDataNodes)
      {
        auto nodeData = node->GetData();
        if (nullptr != nodeData && nodeData->GetUID() == imageStatistics->GetUID())
        {
          node->SetData(statistic);
        }
      }
    }
    // statistics base data does not exist: add new node
    else
    {
      auto statisticsNodeName = m_ImageNode->GetName();
      if (m_SegmentationNode)
      {
        statisticsNodeName += "_" + m_SegmentationNode->GetName();
      }
      statisticsNodeName += "_statistics";
      auto statisticsNode = mitk::CreateImageStatisticsNode(statistic, statisticsNodeName);
      dataStorage->Add(statisticsNode);
    }
  }
}
