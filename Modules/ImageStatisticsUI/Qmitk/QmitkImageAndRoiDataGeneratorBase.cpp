/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageAndRoiDataGeneratorBase.h"

QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType
QmitkImageAndRoiDataGeneratorBase::GetROINodes() const
{
  return m_ROINodes;
}

QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType
QmitkImageAndRoiDataGeneratorBase::GetImageNodes() const
{
  return m_ImageNodes;
}

void
QmitkImageAndRoiDataGeneratorBase::SetImageNodes(const ConstNodeVectorType& imageNodes)
{
  if (m_ImageNodes != imageNodes)
  {
    {
      std::lock_guard<std::mutex> mutexguard(m_DataMutex);
      m_ImageNodes = imageNodes;
    }

    if (m_AutoUpdate)
    {
      this->EnsureRecheckingAndGeneration();
    }
  }
}

void
QmitkImageAndRoiDataGeneratorBase::SetImageNodes(const NodeVectorType& imageNodes)
{
  ConstNodeVectorType constInput;
  constInput.resize(imageNodes.size());
  std::copy(imageNodes.begin(), imageNodes.end(), constInput.begin());
  this->SetImageNodes(constInput);
}

void
QmitkImageAndRoiDataGeneratorBase::SetROINodes(const ConstNodeVectorType& roiNodes)
{
  if (m_ROINodes != roiNodes)
  {
    {
      std::lock_guard<std::mutex> mutexguard(m_DataMutex);
      m_ROINodes = roiNodes;
    }

    if (m_AutoUpdate)
    {
      this->EnsureRecheckingAndGeneration();
    }
  }
}

void
QmitkImageAndRoiDataGeneratorBase::SetROINodes(const NodeVectorType& roiNodes)
{
  ConstNodeVectorType constInput;
  constInput.resize(roiNodes.size());
  std::copy(roiNodes.begin(), roiNodes.end(), constInput.begin());
  this->SetROINodes(constInput);
}

bool
QmitkImageAndRoiDataGeneratorBase::ChangedNodeIsRelevant(const mitk::DataNode* changedNode) const
{
  if (m_AutoUpdate)
  {
    auto finding = std::find(m_ImageNodes.begin(), m_ImageNodes.end(), changedNode);
    if (finding != m_ImageNodes.end())
    {
      return true;
    }

    finding = std::find(m_ROINodes.begin(), m_ROINodes.end(), changedNode);
    if (finding != m_ROINodes.end())
    {
      return true;
    }
  }

  return false;
}

QmitkImageAndRoiDataGeneratorBase::InputPairVectorType
QmitkImageAndRoiDataGeneratorBase::GetAllImageROICombinations() const
{
  std::lock_guard<std::mutex> mutexguard(m_DataMutex);

  InputPairVectorType allCombinations;
  for (const auto& imageNode : m_ImageNodes)
  {
    if (m_ROINodes.empty())
    {
      allCombinations.emplace_back(imageNode, nullptr);
    }
    else
    {
      for (const auto& roiNode : m_ROINodes)
      {
        allCombinations.emplace_back(imageNode, roiNode);
      }
    }
  }
  return allCombinations;
}
