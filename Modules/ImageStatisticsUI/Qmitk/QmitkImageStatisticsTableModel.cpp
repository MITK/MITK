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

#include "QmitkImageStatisticsTableModel.h"

#include "mitkProportionalTimeGeometry.h"
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkImageStatisticsContainerManager.h"
#include "itkMutexLockHolder.h"


QmitkImageStatisticsTableModel::QmitkImageStatisticsTableModel(QObject *parent) :
  QmitkAbstractDataStorageModel(parent)
{}

QmitkImageStatisticsTableModel
::~QmitkImageStatisticsTableModel()
{
  // set data storage to nullptr so that the event listener gets removed
  this->SetDataStorage(nullptr);
};

void QmitkImageStatisticsTableModel::DataStorageChanged()
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::NodePredicateChanged()
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
}


int QmitkImageStatisticsTableModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  return m_StatisticNames.size();
}

int QmitkImageStatisticsTableModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;

  if (m_ViewMode == ViewMode::imageXStatistic)
  {
    return static_cast<int>(m_TimeStepResolvedImageNodes.size());
  }
  else
  {
    mitkThrow() << "View mode ImageXMask is not implemented yet";
  }
}

std::pair<mitk::ImageStatisticsContainer::ConstPointer, unsigned int> QmitkImageStatisticsTableModel::GetRelevantStatsticsByIndex(const QModelIndex &index) const
{
  auto result = std::make_pair<mitk::ImageStatisticsContainer::ConstPointer, unsigned int>(nullptr, 0);

  if (m_ViewMode == ViewMode::imageXStatistic)
  {
    if (index.column() < static_cast<int>(m_TimeStepResolvedImageNodes.size()))
    {
      auto selectedImage = m_TimeStepResolvedImageNodes.at(static_cast<size_t>(index.column()));

      //get the right statistic
      auto rule = mitk::StatisticsToImageRelationRule::New();
      for (mitk::ImageStatisticsContainer::ConstPointer container : m_Statistics)
      {
        if (rule->HasRelation(container, selectedImage.first->GetData()) >= mitk::PropertyRelationRuleBase::RelationType::Connected_Data)
        {
          return std::make_pair(container, selectedImage.second);
        }
      }
    }
  }
  else
  {
    mitkThrow() << "View mode ImageXMask is not implemented yet";
  }

  return result;
}

QVariant QmitkImageStatisticsTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  QVariant result;
  if (m_ViewMode == ViewMode::imageXStatistic) {

    auto containerInfo = GetRelevantStatsticsByIndex(index);

    if (containerInfo.first.IsNotNull() && index.row() < rowCount(QModelIndex()))
    {
      if (Qt::DisplayRole == role)
      {
        auto statsObj = containerInfo.first->GetStatisticsForTimeStep(containerInfo.second);
        auto statisticKey = m_StatisticNames.at(index.row());
        std::stringstream ss;
        if (statsObj.HasStatistic(statisticKey))
        {
          ss << statsObj.GetValueNonConverted(statisticKey);
        }
        else
        {
          ss << "N/A";
        }
        result = QVariant(QString::fromStdString(ss.str()));
      }
      else if (Qt::UserRole == role)
      {
        result = QVariant(index.row());
      }

    }
  }

  return result;
}

QModelIndex QmitkImageStatisticsTableModel::index(int row, int column, const QModelIndex &parent) const
{
  bool hasIndex = this->hasIndex(row, column, parent);
  if (hasIndex)
  {
    return this->createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkImageStatisticsTableModel::parent(const QModelIndex &/*child*/) const
{
  return QModelIndex();
}


Qt::ItemFlags QmitkImageStatisticsTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  return flags;
}

QVariant QmitkImageStatisticsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) && (Qt::Horizontal == orientation))
  {
    if (!m_TimeStepResolvedImageNodes.empty()) {
      if (m_ViewMode == ViewMode::imageXStatistic) {
        std::stringstream ss;
        auto imageInfo = m_TimeStepResolvedImageNodes.at(section);
        ss << imageInfo.first->GetName();
        if (imageInfo.first->GetData() && imageInfo.first->GetData()->GetTimeSteps() > 1)
        {
          ss << " #" << imageInfo.second;
        }
        if (!m_MaskNodes.empty() && m_MaskNodes.size() == m_ImageNodes.size()) {
          auto maskInfo = m_TimeStepResolvedMaskNodes.at(section);
          ss << " / " << maskInfo.first->GetName();
        }
        return QVariant(ss.str().c_str());
      }
    }
  }
  else if ((Qt::DisplayRole == role) && (Qt::Vertical == orientation)) {
      if (m_ViewMode == ViewMode::imageXStatistic) {
        return QVariant(m_StatisticNames.at(section).c_str());
      }
  }
  return QVariant();
}

void QmitkImageStatisticsTableModel::SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes)
{
  std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int> > tempNodes;
  for (const auto& node: nodes)
  {
    auto data = node->GetData();
    if (data)
    {
      auto timeSteps = data->GetTimeSteps();
      for (unsigned int i = 0; i < timeSteps; i++)
      {
        tempNodes.push_back(std::make_pair(node, i));
      }
    }
  }

  emit beginResetModel();
  m_TimeStepResolvedImageNodes = std::move(tempNodes);
  m_ImageNodes = nodes;
  UpdateByDataStorage();
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes)
{
  std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> tempNodes;
  for (const auto &node : nodes)
  {
    auto data = node->GetData();
    if (data)
    {
      auto timeSteps = data->GetTimeSteps();
      //special case: apply one mask to each timestep of an 4D image
      if (timeSteps == 1 && m_TimeStepResolvedImageNodes.size() > 1)
      {
        timeSteps = m_TimeStepResolvedImageNodes.size();
      }
      for (unsigned int i = 0; i < timeSteps; i++)
      {
        tempNodes.push_back(std::make_pair(node, i));
      }
    }
  }

  emit beginResetModel();
  m_TimeStepResolvedMaskNodes = std::move(tempNodes);
  m_MaskNodes = nodes;
  UpdateByDataStorage();
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::SetViewMode(ViewMode m)
{
  emit beginResetModel();
  m_ViewMode = m;
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::Clear()
{
  emit beginResetModel();
  m_Statistics.clear();
  m_ImageNodes.clear();
  m_TimeStepResolvedImageNodes.clear();
  m_MaskNodes.clear();
  m_StatisticNames.clear();
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::UpdateByDataStorage()
{
  StatisticsContainerVector newStatistics;
  
  auto datamanager = m_DataStorage.Lock();

  if (datamanager.IsNotNull())
  {
    for (const auto& image : m_ImageNodes)
    {
      if (m_MaskNodes.empty())
      {
        auto stats = mitk::ImageStatisticsContainerManager::GetImageStatistics(datamanager, image->GetData());
        if (stats.IsNotNull())
        {
          newStatistics.emplace_back(stats);
        }
      }
      else
      {
        for (const auto& mask : m_MaskNodes)
        {
          auto stats = mitk::ImageStatisticsContainerManager::GetImageStatistics(datamanager, image->GetData(), mask->GetData());
          if (stats.IsNotNull())
          {
            newStatistics.emplace_back(stats);
          }
        }
      }
    }
    if (!newStatistics.empty())
    {
      emit dataAvailable();
    }
  }

  {
    itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_Mutex);
    m_Statistics = newStatistics;
  }

  m_StatisticNames = mitk::GetAllStatisticNames(m_Statistics);
}

void QmitkImageStatisticsTableModel::NodeRemoved(const mitk::DataNode *)
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::NodeAdded(const mitk::DataNode *)
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
}

void QmitkImageStatisticsTableModel::NodeChanged(const mitk::DataNode *)
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
}
