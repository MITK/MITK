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

#include <QMessageBox>

namespace
{
  void handleInvalidNodeSelection()
  {
    auto message = QStringLiteral("All selected bounding boxes must be child nodes of a single common reference image!");
    MITK_ERROR << message;
    QMessageBox::warning(nullptr, QStringLiteral("Convert to ROI"), message);
  }

  std::vector<const mitk::DataNode*> filterNodeSelection(const QList<mitk::DataNode::Pointer>& selectedNodes, const mitk::DataStorage* dataStorage)
  {
    std::vector<const mitk::DataNode*> result;

    std::copy_if(selectedNodes.cbegin(), selectedNodes.cend(), std::back_inserter(result), [](const mitk::DataNode* node) {
      return node != nullptr && dynamic_cast<mitk::GeometryData*>(node->GetData()) != nullptr;
      });

    const mitk::Image* referenceImage = nullptr;

    for (auto node : result)
    {
      auto sources = dataStorage->GetSources(node, mitk::TNodePredicateDataType<mitk::Image>::New());

      if (sources->size() != 1)
        mitkThrow();

      if (referenceImage == nullptr)
      {
        referenceImage = static_cast<mitk::Image*>(sources->front()->GetData());
      }
      else if (referenceImage != sources->front()->GetData())
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
    auto nodes = filterNodeSelection(selectedNodes, m_DataStorage);
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
