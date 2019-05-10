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

#include "QmitkImageStatisticsTreeModel.h"

#include "QmitkImageStatisticsTreeItem.h"
#include "itkMutexLockHolder.h"
#include "mitkImageStatisticsContainerManager.h"
#include "mitkProportionalTimeGeometry.h"
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"

QmitkImageStatisticsTreeModel::QmitkImageStatisticsTreeModel(QObject *parent) : QmitkAbstractDataStorageModel(parent)
{
  m_RootItem = new QmitkImageStatisticsTreeItem();
}

QmitkImageStatisticsTreeModel ::~QmitkImageStatisticsTreeModel()
{
  // set data storage to nullptr so that the event listener gets removed
  this->SetDataStorage(nullptr);
  delete m_RootItem;
};

void QmitkImageStatisticsTreeModel::DataStorageChanged()
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::NodePredicateChanged()
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

int QmitkImageStatisticsTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  int columns = m_StatisticNames.size() + 1;
  return columns;
}

int QmitkImageStatisticsTreeModel::rowCount(const QModelIndex &parent) const
{
  QmitkImageStatisticsTreeItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = m_RootItem;
  else
    parentItem = static_cast<QmitkImageStatisticsTreeItem *>(parent.internalPointer());

  return parentItem->childCount();
}

QVariant QmitkImageStatisticsTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant();

  QmitkImageStatisticsTreeItem *item = static_cast<QmitkImageStatisticsTreeItem *>(index.internalPointer());

  return item->data(index.column());
}

QModelIndex QmitkImageStatisticsTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QmitkImageStatisticsTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = m_RootItem;
  else
    parentItem = static_cast<QmitkImageStatisticsTreeItem *>(parent.internalPointer());

  QmitkImageStatisticsTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QmitkImageStatisticsTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  QmitkImageStatisticsTreeItem *childItem = static_cast<QmitkImageStatisticsTreeItem *>(child.internalPointer());
  QmitkImageStatisticsTreeItem *parentItem = childItem->parentItem();

  if (parentItem == m_RootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags QmitkImageStatisticsTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  return QAbstractItemModel::flags(index);
}

QVariant QmitkImageStatisticsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) && (Qt::Horizontal == orientation))
  {
    if (section == 0)
    {
      return m_HeaderFirstColumn;
    }
    else
    {
      return QVariant(m_StatisticNames.at(section - 1).c_str());
    }
  }
  return QVariant();
}

void QmitkImageStatisticsTreeModel::SetImageNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes)
{
  std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> tempNodes;
  for (const auto &node : nodes)
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
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes)
{
  std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> tempNodes;
  for (const auto &node : nodes)
  {
    auto data = node->GetData();
    if (data)
    {
      auto timeSteps = data->GetTimeSteps();
      // special case: apply one mask to each timestep of an 4D image
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
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::Clear()
{
  emit beginResetModel();
  m_Statistics.clear();
  m_ImageNodes.clear();
  m_TimeStepResolvedImageNodes.clear();
  m_MaskNodes.clear();
  m_StatisticNames.clear();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::UpdateByDataStorage()
{
  StatisticsContainerVector newStatistics;

  auto datamanager = m_DataStorage.Lock();

  if (datamanager.IsNotNull())
  {
    for (const auto &image : m_ImageNodes)
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
        for (const auto &mask : m_MaskNodes)
        {
          auto stats =
            mitk::ImageStatisticsContainerManager::GetImageStatistics(datamanager, image->GetData(), mask->GetData());
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
  BuildHierarchicalModel();
}

void QmitkImageStatisticsTreeModel::BuildHierarchicalModel()
{
  // reset old model
  delete m_RootItem;
  m_RootItem = new QmitkImageStatisticsTreeItem();

  bool hasMask = false;
  bool hasMultipleTimesteps = false;

  std::map<mitk::DataNode::Pointer, QmitkImageStatisticsTreeItem *> dataNodeToTreeItem;

  for (auto statistic : m_Statistics)
  {
    // get the connected image data node/mask data node
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    auto imageOfStatisticsPredicate = imageRule->GetDestinationsDetector(statistic);
    auto imageCandidates = this->GetDataStorage()->GetSubset(imageOfStatisticsPredicate);

    auto maskRule = mitk::StatisticsToMaskRelationRule::New();
    auto maskOfStatisticsPredicate = maskRule->GetDestinationsDetector(statistic);
    auto maskCandidates = this->GetDataStorage()->GetSubset(maskOfStatisticsPredicate);

    if (imageCandidates->empty())
    {
      mitkThrow() << "no image found connected to statistic" << statistic << " Aborting.";
    }
    auto image = imageCandidates->front();
    // image: 1. hierarchy level
    QmitkImageStatisticsTreeItem *imageItem;
    auto search = dataNodeToTreeItem.find(image);
    // the tree item was created previously
    if (search != dataNodeToTreeItem.end())
    {
      imageItem = search->second;
    }
    // create the tree item
    else
    {
      QString imageLabel = QString::fromStdString(image->GetName());
      if (statistic->GetTimeSteps() == 1 && maskCandidates->empty())
      {
        auto statisticsObject = statistic->GetStatisticsForTimeStep(0);
        imageItem = new QmitkImageStatisticsTreeItem(statisticsObject, m_StatisticNames, imageLabel, m_RootItem);
      }
      else
      {
        imageItem = new QmitkImageStatisticsTreeItem(m_StatisticNames, imageLabel, m_RootItem);
      }
      m_RootItem->appendChild(imageItem);
      dataNodeToTreeItem.emplace(image, imageItem);
    }

    // mask: 2. hierarchy level (optional, only if mask exists)
    QmitkImageStatisticsTreeItem *lastParent;
    if (!maskCandidates->empty())
    {
      auto mask = maskCandidates->front();
      QString maskLabel = QString::fromStdString(mask->GetName());
      QmitkImageStatisticsTreeItem *maskItem;
      // add statistical values directly in this hierarchy level
      if (statistic->GetTimeSteps() == 1)
      {
        auto statisticsObject = statistic->GetStatisticsForTimeStep(0);
        maskItem = new QmitkImageStatisticsTreeItem(statisticsObject, m_StatisticNames, maskLabel, imageItem);
      }
      else
      {
        maskItem = new QmitkImageStatisticsTreeItem(m_StatisticNames, maskLabel, imageItem);
      }

      imageItem->appendChild(maskItem);
      lastParent = maskItem;
      hasMask = true;
    }
    else
    {
      lastParent = imageItem;
    }
    // 3. hierarchy level (optional, only if >1 timestep)
    if (statistic->GetTimeSteps() > 1)
    {
      for (unsigned int i = 0; i < statistic->GetTimeSteps(); i++)
      {
        QString timeStepLabel = "[" + QString::number(i) + "] " +
                                QString::number(statistic->GetTimeGeometry()->TimeStepToTimePoint(i)) + " ms";
        if (statistic->TimeStepExists(i))
        {
          auto statisticsItem = new QmitkImageStatisticsTreeItem(
                statistic->GetStatisticsForTimeStep(i), m_StatisticNames, timeStepLabel, lastParent);
          lastParent->appendChild(statisticsItem);
        }
      }
      hasMultipleTimesteps = true;
    }
  }
  QString headerString = "Images";
  if (hasMask)
  {
    headerString += "/Masks";
  }
  if (hasMultipleTimesteps)
  {
    headerString += "/Timesteps";
  }
  m_HeaderFirstColumn = headerString;
}

void QmitkImageStatisticsTreeModel::NodeRemoved(const mitk::DataNode *)
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::NodeAdded(const mitk::DataNode *)
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::NodeChanged(const mitk::DataNode *)
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}
