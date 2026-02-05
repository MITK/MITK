/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodeUidMapper.h"
#include <mitkStringProperty.h>

#include <stdexcept>

namespace mitk
{

NodeUidMapper::NodeUidMapper() = default;

NodeUidMapper::~NodeUidMapper()
{
  this->ClearDataStorage();
}

void NodeUidMapper::SetDataStorage(DataStorage* dataStorage)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // Disconnect from previous DataStorage
  if (m_DataStorage != nullptr && m_Connected)
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
      MessageDelegate1<NodeUidMapper, const DataNode*>(this, &NodeUidMapper::OnNodeAdded));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      MessageDelegate1<NodeUidMapper, const DataNode*>(this, &NodeUidMapper::OnNodeRemoved));
    m_Connected = false;
  }

  // Clear state
  m_UidToNode.clear();
  m_NodeToUid.clear();

  m_DataStorage = dataStorage;

  if (m_DataStorage != nullptr)
  {
    // Clear any existing restapi.uid properties (transient - fresh start)
    this->ClearRESTUIDProperty();

    // Subscribe to events
    m_DataStorage->AddNodeEvent.AddListener(
      MessageDelegate1<NodeUidMapper, const DataNode*>(this, &NodeUidMapper::OnNodeAdded));
    m_DataStorage->RemoveNodeEvent.AddListener(
      MessageDelegate1<NodeUidMapper, const DataNode*>(this, &NodeUidMapper::OnNodeRemoved));
    m_Connected = true;
  }
}

void NodeUidMapper::ClearDataStorage()
{
  this->SetDataStorage(nullptr);
}

void NodeUidMapper::ClearRESTUIDProperty()
{
  // Remove any existing restapi.uid properties from all nodes
  // This ensures UIDs are truly transient across sessions
  // Note: m_Mutex is already locked by caller
  if (m_DataStorage == nullptr)
  {
    return;
  }

  auto allNodes = m_DataStorage->GetAll();
  for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
  {
    auto* node = it->Value().GetPointer();
    if (node != nullptr)
    {
      node->GetPropertyList()->DeleteProperty(UID_PROPERTY_KEY);
    }
  }
}

std::string NodeUidMapper::GetOrCreateUid(const DataNode* node)
{
  if (node == nullptr)
  {
    throw std::invalid_argument("Node cannot be null");
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // Check cache first
  auto it = m_NodeToUid.find(node);
  if (it != m_NodeToUid.end())
  {
    return it->second;
  }

  // Generate new UID
  std::string uid = this->GenerateUid();

  // Store in cache
  m_UidToNode[uid] = const_cast<DataNode*>(node);
  m_NodeToUid[node] = uid;

  // Store as property for debugging visibility
  const_cast<DataNode*>(node)->SetStringProperty(UID_PROPERTY_KEY, uid.c_str());

  return uid;
}

DataNode* NodeUidMapper::FindNodeByUid(const std::string& uid) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  auto it = m_UidToNode.find(uid);
  if (it != m_UidToNode.end())
  {
    return it->second;
  }
  return nullptr;
}

std::optional<std::string> NodeUidMapper::GetUid(const DataNode* node) const
{
  if (node == nullptr)
  {
    return std::nullopt;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  auto it = m_NodeToUid.find(node);
  if (it != m_NodeToUid.end())
  {
    return it->second;
  }
  return std::nullopt;
}

bool NodeUidMapper::HasUid(const std::string& uid) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_UidToNode.find(uid) != m_UidToNode.end();
}

void NodeUidMapper::RestoreUid(const DataNode* node, const std::string& uid)
{
  if (node == nullptr)
  {
    throw std::invalid_argument("Node cannot be null");
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // Restore the mapping in both directions
  m_UidToNode[uid] = const_cast<DataNode*>(node);
  m_NodeToUid[node] = uid;

  // Restore as property for debugging visibility
  const_cast<DataNode*>(node)->SetStringProperty(UID_PROPERTY_KEY, uid.c_str());
}

std::string NodeUidMapper::GenerateUid()
{
  // Format: "node_<counter>" - simple, unique within session
  return "node_" + std::to_string(++m_UidCounter);
}

void NodeUidMapper::OnNodeAdded(const DataNode* node)
{
  if (nullptr != node)
  {
    // remove the uid property if existing. This could happen if a session is loaded again,
    // as currently session doesn't filter out the property on save.
    node->GetPropertyList()->DeleteProperty(UID_PROPERTY_KEY);
  }
}

void NodeUidMapper::OnNodeRemoved(const DataNode* node)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  auto it = m_NodeToUid.find(node);
  if (it != m_NodeToUid.end())
  {
    m_UidToNode.erase(it->second);
    m_NodeToUid.erase(it);
  }
}

}
