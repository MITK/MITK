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

#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <QApplication>

using namespace berry;
using namespace mitk;
using namespace std;

namespace
{
  std::string MakeNodeName(const std::string& parentName, const std::string& labelSuffix, bool smoothed)
  {
    auto base = parentName.empty()
      ? std::string("segmentation")
      : parentName;

    if (!labelSuffix.empty())
      base += "_" + labelSuffix;

    if (smoothed)
      base += "_smoothed";

    return base;
  }

  std::string LabelSuffix(const Label* label, MultiLabelSegmentation::LabelValueType value)
  {
    if (label != nullptr && !label->GetName().empty())
      return label->GetName();

    return "label_" + std::to_string(value);
  }

  void AddSurfaceNode(
    DataStorage* dataStorage,
    DataNode* parentNode,
    Surface::Pointer surface,
    const std::string& parentName,
    const std::string& labelSuffix,
    const Color& color,
    bool smoothed)
  {
    auto node = DataNode::New();
    node->SetData(surface);
    node->SetName(MakeNodeName(parentName, labelSuffix, smoothed));
    node->SetColor(color);
    node->SetProperty("scalar visibility", BoolProperty::New(false));
    dataStorage->Add(node, parentNode);
  }

  vtkSmartPointer<vtkImageData> BinarizeMask(vtkImageData* mask)
  {
    // Normalize the foreground to 1 so masks with arbitrary non-zero foreground values
    // (commonly 0/255 binary masks) extract correctly when we ask vtkSurfaceNets3D for
    // label 1. Mirrors the threshold(0.5) behavior of the previous mitkShowSegmentationAsSurface
    // path.
    auto thresholdFilter = vtkSmartPointer<vtkImageThreshold>::New();
    thresholdFilter->SetInputData(mask);
    thresholdFilter->ThresholdByUpper(0.5);
    thresholdFilter->SetInValue(1);
    thresholdFilter->SetOutValue(0);
    thresholdFilter->SetOutputScalarTypeToUnsignedChar();
    thresholdFilter->Update();

    auto result = vtkSmartPointer<vtkImageData>::New();
    result->ShallowCopy(thresholdFilter->GetOutput());
    return result;
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
          const Color labelColor = label != nullptr ? label->GetColor() : Color{};

          AddSurfaceNode(m_DataStorage, selectedNode, surface, parentName, LabelSuffix(label, labelValue), labelColor, m_IsSmoothed);
        }
      }
    }
    else
    {
      auto binarized = BinarizeMask(imageMask->GetVtkImageData(0));
      auto results = extractor.ExtractPerLabel(binarized, {1});
      auto it = results.find(1);
      if (it != results.end() && it->second != nullptr && it->second->GetNumberOfCells() > 0)
      {
        auto surface = Surface::New();
        surface->SetVtkPolyData(it->second);

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
