/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageHistoryModel.h>

#include "mitkWeakPointer.h"
#include <deque>
#include <algorithm>
#include <mutex>

/** Type of the internal history container.
 Remark: It stores raw pointer instead of mitk::WeakPointer
 because this lead to occasional crashes when the application was closed (see T24770 for a similar problem
 with the same reason*/
using NodeHistoryType = std::deque< const mitk::DataNode* >;

/** History of node selection. It is sorted from new to old.*/
NodeHistoryType _nodeHistory;
std::mutex _historyMutex;

QmitkDataStorageHistoryModel::QmitkDataStorageHistoryModel(QObject *parent) : QmitkDataStorageDefaultListModel(parent)
{
}

void QmitkDataStorageHistoryModel::UpdateModelData()
{
    std::vector<mitk::DataNode::Pointer> dataNodes;
    if (!m_DataStorage.IsExpired())
    {
        auto dataStorage = m_DataStorage.Lock();
        if (dataStorage.IsNotNull())
        {
          mitk::DataStorage::SetOfObjects::ConstPointer nodesCandidats;
          if (m_NodePredicate.IsNotNull())
          {
            nodesCandidats = dataStorage->GetSubset(m_NodePredicate);
          }
          else
          {
            nodesCandidats = dataStorage->GetAll();
          }

          const std::lock_guard<std::mutex> lock(_historyMutex);

          for (auto historyNode : _nodeHistory)
          {
            auto finding = std::find(nodesCandidats->begin(), nodesCandidats->end(), historyNode);
            if (finding != nodesCandidats->end())
            {
              dataNodes.push_back(*finding);
            }
          }
        }
    }

    // update the model, so that it will be filled with the nodes of the new data storage
    beginResetModel();
    m_DataNodes = dataNodes;
    endResetModel();
}

void QmitkDataStorageHistoryModel::AddNodeToHistory(mitk::DataNode* node)
{
    const std::lock_guard<std::mutex> lock(_historyMutex);

    auto finding = std::find(std::begin(_nodeHistory), std::end(_nodeHistory), node);
    while (finding != std::end(_nodeHistory))
    {
        _nodeHistory.erase(finding);
        finding = std::find(std::begin(_nodeHistory), std::end(_nodeHistory), node);
    }

    _nodeHistory.push_front(node);
}

void QmitkDataStorageHistoryModel::ResetHistory()
{
    const std::lock_guard<std::mutex> lock(_historyMutex);
    _nodeHistory.clear();
}
