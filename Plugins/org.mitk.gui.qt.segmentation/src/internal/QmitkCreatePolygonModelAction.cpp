/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkCreatePolygonModelAction.h"

// MITK
#include <mitkColorProperty.h>
#include <mitkDataStorage.h>
#include <mitkLabelSetImage.h>
#include <mitkMultiLabelSurfaceNetsExtractor.h>
#include <mitkProperties.h>
#include <mitkStatusBar.h>
#include <mitkSurface.h>

#include <vtkPolyData.h>

#include <QApplication>

using namespace berry;
using namespace mitk;
using namespace std;

namespace
{
  std::string MakeNodeName(const std::string& parentName, const std::string& labelName, bool smoothed)
  {
    auto base = parentName.empty()
      ? std::string("segmentation")
      : parentName;

    if (!labelName.empty())
      base += "_" + labelName;

    if (smoothed)
      base += "_smoothed";

    return base;
  }

  void AddSurfaceNode(
    DataStorage* dataStorage,
    DataNode* parentNode,
    Surface::Pointer surface,
    const std::string& parentName,
    const std::string& labelName,
    const Color& color,
    bool smoothed)
  {
    auto node = DataNode::New();
    node->SetData(surface);
    node->SetName(MakeNodeName(parentName, labelName, smoothed));
    node->SetColor(color);
    node->SetProperty("scalar visibility", BoolProperty::New(false));
    dataStorage->Add(node, parentNode);
  }
}

QmitkCreatePolygonModelAction::QmitkCreatePolygonModelAction()
{
}

QmitkCreatePolygonModelAction::~QmitkCreatePolygonModelAction()
{
}

void QmitkCreatePolygonModelAction::Run(const QList<DataNode::Pointer> &selectedNodes)
{
  if (selectedNodes.empty() || m_DataStorage.IsNull())
  {
    return;
  }

  DataNode::Pointer selectedNode = selectedNodes[0];
  auto segmentation = dynamic_cast<MultiLabelSegmentation*>(selectedNode->GetData());
  auto imageMask = dynamic_cast<Image*>(selectedNode->GetData());

  if (nullptr == segmentation && nullptr == imageMask)
  {
    return;
  }

  StatusBar::GetInstance()->DisplayText(
    m_IsSmoothed ? "Smoothed surface creation started..." : "Surface creation started...");
  QApplication::setOverrideCursor(Qt::WaitCursor);

  const std::string parentName = selectedNode->GetName();

  try
  {
    MultiLabelSurfaceNetsExtractor extractor;
    extractor.SetSmoothing(m_IsSmoothed);

    if (nullptr != segmentation)
    {
      const TimeStepType timeStep = 0;

      const auto numGroups = segmentation->GetNumberOfGroups();
      for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < numGroups; ++groupID)
      {
        auto groupImage = const_cast<Image*>(segmentation->GetGroupImage(groupID));
        if (nullptr == groupImage)
        {
          continue;
        }

        auto vtkImage = groupImage->GetVtkImageData(timeStep);
        const auto labelValues = segmentation->GetLabelValuesByGroup(groupID);
        if (labelValues.empty())
        {
          continue;
        }

        auto results = extractor.ExtractPerLabel(vtkImage, labelValues);
        for (const auto& [labelValue, polyData] : results)
        {
          if (nullptr == polyData || polyData->GetNumberOfCells() == 0)
          {
            continue;
          }

          auto surface = Surface::New();
          surface->SetVtkPolyData(polyData);

          const auto label = segmentation->GetLabel(labelValue);
          const std::string labelName = label != nullptr ? label->GetName() : std::string();
          const Color labelColor = label != nullptr ? label->GetColor() : Color{};

          AddSurfaceNode(m_DataStorage, selectedNode, surface, parentName, labelName, labelColor, m_IsSmoothed);
        }
      }
    }
    else
    {
      // Plain binary mask: treat the foreground as a single label with value 1.
      auto vtkImage = imageMask->GetVtkImageData(0);
      const std::vector<MultiLabelSegmentation::LabelValueType> labels{1};
      auto results = extractor.ExtractPerLabel(vtkImage, labels);
      for (const auto& [labelValue, polyData] : results)
      {
        if (nullptr == polyData || polyData->GetNumberOfCells() == 0)
        {
          continue;
        }

        auto surface = Surface::New();
        surface->SetVtkPolyData(polyData);

        Color color;
        color.Set(1.0f, 1.0f, 1.0f);
        AddSurfaceNode(m_DataStorage, selectedNode, surface, parentName, std::string(), color, m_IsSmoothed);
      }
    }
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Surface creation failed: " << e.what();
  }
  catch (...)
  {
    MITK_ERROR << "Surface creation failed.";
  }

  QApplication::restoreOverrideCursor();
  StatusBar::GetInstance()->Clear();
}

void QmitkCreatePolygonModelAction::SetDataStorage(DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreatePolygonModelAction::SetFunctionality(QtViewPart *)
{
}
