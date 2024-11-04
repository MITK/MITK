/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodeSelectionService_h
#define mitkNodeSelectionService_h

#include <mitkINodeSelectionService.h>

#include <mutex>
#include <unordered_map>

namespace mitk
{
  class NodeSelectionService : public INodeSelectionService
  {
  public:
    NodeSelectionService();
    ~NodeSelectionService() override;

    bool AddListener(const std::string& context, INodeSelectionListener* listener) override;
    bool RemoveListener(const std::string& context, const INodeSelectionListener* listener) override;
    bool RemoveListener(const INodeSelectionListener* listener) override;

    bool SendSelection(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) const override;

  private:
    std::unordered_multimap<std::string, INodeSelectionListener*> m_Listeners;
    mutable std::mutex m_Mutex;
  };
}

#endif
