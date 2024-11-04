/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodeSelectionService.h"
#include <mitkINodeSelectionListener.h>

mitk::NodeSelectionService::NodeSelectionService()
{
}

mitk::NodeSelectionService::~NodeSelectionService()
{
}

bool mitk::NodeSelectionService::AddListener(const std::string& context, INodeSelectionListener* listener)
{
  if (context.empty() || listener == nullptr)
    return false;

  std::scoped_lock lock(m_Mutex);

  auto range = m_Listeners.equal_range(context);

  auto it = std::find_if(range.first, range.second, [listener](const auto& pair) {
    return pair.second == listener;
  });

  if (it == range.second)
    m_Listeners.emplace(context, listener);

  return true;
}

bool mitk::NodeSelectionService::RemoveListener(const std::string& context, const INodeSelectionListener* listener)
{
  if (context.empty() || listener == nullptr)
    return false;

  std::scoped_lock lock(m_Mutex);

  auto range = m_Listeners.equal_range(context);

  auto it = std::find_if(range.first, range.second, [listener](const auto& pair) {
    return pair.second == listener;
  });

  if (it != range.second)
  {
    m_Listeners.erase(it);
    return true;
  }

  return false;
}

bool mitk::NodeSelectionService::RemoveListener(const INodeSelectionListener* listener)
{
  if (listener == nullptr)
    return false;

  std::scoped_lock lock(m_Mutex);

  bool foundListener = false;

  for (auto it = m_Listeners.begin(); it != m_Listeners.end(); )
  {
    if (it->second == listener)
    {
      it = m_Listeners.erase(it);
      foundListener = true;
    }
    else
    {
      ++it;
    }
  }

  return foundListener;
}

bool mitk::NodeSelectionService::SendSelection(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) const
{
  if (context.empty())
    return false;

  std::scoped_lock lock(m_Mutex);

  auto range = m_Listeners.equal_range(context);

  for (auto it = range.first; it != range.second; ++it)
    it->second->OnSelectionReceived(context, selection);

  return true;
}
