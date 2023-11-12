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
  void handleInvalidNodeSelection()
  {
    auto message = QStringLiteral("All selected bounding boxes must be child nodes of a single common reference image with a non-rotated geometry!");
    MITK_ERROR << message;
    QMessageBox::warning(nullptr, QStringLiteral("Convert to ROI"), message);
  }

  bool isRotated(const mitk::BaseGeometry* geometry)
  {
    const auto* matrix = geometry->GetVtkMatrix();

    for (int j = 0; j < 3; ++j)
    {
      for (int i = 0; i < 3; ++i)
      {
        if (i != j && matrix->GetElement(i, j) > mitk::eps)
          return true;
      }
    }

    return false;
  }

  std::pair<std::vector<const mitk::DataNode*>, mitk::DataNode*> getValidInput(const QList<mitk::DataNode::Pointer>& selectedNodes, const mitk::DataStorage* dataStorage)
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
        if (isRotated(sourceNodes->front()->GetData()->GetGeometry()))
          mitkThrow();

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
    auto [nodes, referenceNode] = getValidInput(selectedNodes, m_DataStorage);

    auto roi = mitk::ROI::New();
    roi->SetClonedGeometry(referenceNode->GetData()->GetGeometry());

    unsigned int id = 0;

    for (auto node : nodes)
    {
      mitk::ROI::Element element(id++);
      element.SetProperty("name", mitk::StringProperty::New(node->GetName()));

      if (auto* color = node->GetProperty("Bounding Shape.Deselected Color"); color != nullptr)
        element.SetProperty("color", color);

      const auto* geometry = node->GetData()->GetGeometry();
      const auto origin = geometry->GetOrigin() - roi->GetGeometry()->GetOrigin();
      const auto spacing = geometry->GetSpacing();
      const auto bounds = geometry->GetBounds();

      mitk::Point3D min;
      mitk::Point3D max;

      for (size_t i = 0; i < 3; ++i)
      {
        min[i] = origin[i] / spacing[i] + bounds[2 * i];
        max[i] = origin[i] / spacing[i] + bounds[2 * i + 1] - 1;
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
    handleInvalidNodeSelection();
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
