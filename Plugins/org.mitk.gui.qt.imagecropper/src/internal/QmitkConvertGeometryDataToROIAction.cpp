/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkConvertGeometryDataToROIAction.h"

#include <mitkGeometryData.h>
#include <mitkImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkROI.h>

#include <QMessageBox>

namespace
{
  void HandleInvalidNodeSelection()
  {
    auto message = QStringLiteral(
      "All selected bounding boxes must be child nodes of a single common reference image!");

    MITK_ERROR << message;
    QMessageBox::warning(nullptr, QStringLiteral("Convert to ROI"), message);
  }

  std::pair<std::vector<const mitk::DataNode*>, mitk::DataNode*> GetValidInput(const QList<mitk::DataNode::Pointer>& selectedNodes, const mitk::DataStorage* dataStorage)
  {
    std::pair<std::vector<const mitk::DataNode*>, mitk::DataNode*> result;
    result.first.reserve(selectedNodes.size());

    std::copy_if(selectedNodes.cbegin(), selectedNodes.cend(), std::back_inserter(result.first), [](const mitk::DataNode* node) {
      return node != nullptr && dynamic_cast<mitk::GeometryData*>(node->GetData()) != nullptr;
    });

    for (auto node : result.first)
    {
      auto sourceNodes = dataStorage->GetSources(node, mitk::TNodePredicateDataType<mitk::Image>::New());

      if (sourceNodes->size() != 1)
        mitkThrow();

      if (result.second == nullptr)
      {
        result.second = sourceNodes->front();
      }
      else if (result.second != sourceNodes->front())
      {
        mitkThrow();
      }
    }

    return result;
  }
}

QmitkConvertGeometryDataToROIAction::QmitkConvertGeometryDataToROIAction()
{
}

QmitkConvertGeometryDataToROIAction::~QmitkConvertGeometryDataToROIAction()
{
}

void QmitkConvertGeometryDataToROIAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  try
  {
    auto [nodes, referenceNode] = GetValidInput(selectedNodes, m_DataStorage);

    auto roi = mitk::ROI::New();
    roi->SetGeometry(referenceNode->GetData()->GetGeometry());

    unsigned int id = 0;

    for (auto node : nodes)
    {
      mitk::ROI::Element element(id++);
      element.SetProperty("name", mitk::StringProperty::New(node->GetName()));

      if (auto* color = node->GetProperty("Bounding Shape.Deselected Color"); color != nullptr)
        element.SetProperty("color", color);

      const auto* geometry = node->GetData()->GetGeometry();

      mitk::Vector3D origin = geometry->GetOrigin() - roi->GetGeometry()->GetOrigin();
      geometry->WorldToIndex(origin, origin);

      const auto bounds = geometry->GetBounds();

      mitk::Point3D min;
      mitk::Point3D max;

      for (size_t i = 0; i < 3; ++i)
      {
        min[i] = origin[i] + bounds[2 * i] - 0.5;
        max[i] = origin[i] + bounds[2 * i + 1] - 1 + 0.5;
      }

      element.SetMin(min);
      element.SetMax(max);

      roi->AddElement(element);
    }

    auto roiNode = mitk::DataNode::New();
    roiNode->SetName(referenceNode->GetName() + " ROI" + (roi->GetNumberOfElements() > 1 ? "s" : ""));
    roiNode->SetData(roi);

    m_DataStorage->Add(roiNode, referenceNode);
  }
  catch (const mitk::Exception&)
  {
    HandleInvalidNodeSelection();
  }
}

void QmitkConvertGeometryDataToROIAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkConvertGeometryDataToROIAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkConvertGeometryDataToROIAction::SetSmoothed(bool)
{
}

void QmitkConvertGeometryDataToROIAction::SetDecimated(bool)
{
}
