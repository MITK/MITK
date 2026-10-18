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
#include <mitkGeometry3D.h>
#include <mitkLabelSetImage.h>
#include <mitkMultiLabelSurfaceNetsExtractor.h>
#include <mitkProgressTask.h>
#include <mitkProperties.h>
#include <mitkSurface.h>

#include <vtkImageData.h>
#include <vtkImageBinaryThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <QApplication>

#include <map>

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

  Surface::Pointer CreateSurface(const TimeGeometry* referenceTimeGeometry)
  {
    // The polydata is baked into world space, so only the time bounds are taken over
    // from the source image while each time step keeps an identity spatial geometry.
    auto timeGeometry = referenceTimeGeometry->Clone();
    timeGeometry->ReplaceTimeStepGeometries(Geometry3D::New());

    auto surface = Surface::New();
    surface->SetTimeGeometry(timeGeometry);

    const auto timeSteps = static_cast<unsigned int>(timeGeometry->CountTimeSteps());
    surface->Expand(timeSteps);

    // Time steps without a boundary still get (empty) polydata, as the surface
    // writers reject time steps that have none.
    for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
      surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), timeStep);

    return surface;
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

  vtkSmartPointer<vtkPolyData> ApplyImageToWorld(vtkPolyData* poly, const BaseGeometry* geometry)
  {
    // The extractor emits polydata in image-local coordinates (vtkImageData has origin
    // (0,0,0), identity direction; spacing is already applied). Bake the [direction | origin]
    // transform into the polydata so the resulting Surface lives in world space and lines
    // up with the source segmentation.
    auto transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(MultiLabelSurfaceNetsExtractor::GetImageToWorldMatrix(geometry));

    auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputData(poly);
    transformFilter->SetTransform(transform);
    transformFilter->Update();

    auto result = vtkSmartPointer<vtkPolyData>::New();
    result->ShallowCopy(transformFilter->GetPolyDataOutput());
    return result;
  }

  vtkSmartPointer<vtkImageData> BinarizeMask(vtkImageData* mask)
  {
    // Normalize the foreground to 1 so masks with arbitrary non-zero foreground values
    // (commonly 0/255 binary masks) extract correctly when we ask vtkSurfaceNets3D for
    // label 1.
    auto thresholdFilter = vtkSmartPointer<vtkImageBinaryThreshold>::New();
    thresholdFilter->SetInputData(mask);
    thresholdFilter->SetThresholdFunction(vtkImageBinaryThreshold::THRESHOLD_UPPER);
    thresholdFilter->SetLowerThreshold(0.5);
    thresholdFilter->ReplaceInOn();
    thresholdFilter->SetInValue(1);
    thresholdFilter->ReplaceOutOn();
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

  // One step per time step of each group. The conversion runs on the GUI
  // thread, where the progress notification paints itself as it advances.
  const auto steps = nullptr != segmentation
    ? static_cast<unsigned int>(segmentation->GetNumberOfGroups() * segmentation->GetTimeSteps())
    : imageMask->GetTimeSteps();

  ProgressTask task(m_IsSmoothed ? "Creating smoothed polygon models" : "Creating polygon models", steps);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  const std::string parentName = selectedNode->GetName();

  try
  {
    MultiLabelSurfaceNetsExtractor extractor;
    extractor.SetSmoothing(m_IsSmoothed);

    if (nullptr != segmentation)
    {
      const auto numGroups = segmentation->GetNumberOfGroups();
      const auto groupTimeSteps = segmentation->GetTimeSteps();

      for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < numGroups; ++groupID)
      {
        auto groupImage = segmentation->GetGroupImage(groupID);
        if (nullptr == groupImage)
        {
          continue;
        }

        const auto labelValues = segmentation->GetLabelValuesByGroup(groupID);
        if (labelValues.empty())
        {
          continue;
        }

        // Created on the first time step in which a label has a boundary, so labels
        // that are empty throughout get no node.
        std::map<MultiLabelSegmentation::LabelValueType, Surface::Pointer> surfaces;

        const auto timeSteps = groupImage->GetTimeSteps();
        for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
        {
          task.SetProgress(static_cast<unsigned int>(groupID * groupTimeSteps + timeStep));

          auto results = extractor.ExtractPerLabel(groupImage->GetVtkImageData(timeStep), labelValues);
          for (const auto& [labelValue, polyData] : results)
          {
            if (nullptr == polyData || polyData->GetNumberOfCells() == 0)
            {
              continue;
            }

            auto& surface = surfaces[labelValue];
            if (surface.IsNull())
            {
              surface = CreateSurface(groupImage->GetTimeGeometry());
            }

            surface->SetVtkPolyData(ApplyImageToWorld(polyData, groupImage->GetGeometry(timeStep)), timeStep);
          }
        }

        for (const auto& [labelValue, surface] : surfaces)
        {
          const auto label = segmentation->GetLabel(labelValue);
          const Color labelColor = label != nullptr ? label->GetColor() : Color{};

          AddSurfaceNode(m_DataStorage, selectedNode, surface, parentName, LabelSuffix(label, labelValue), labelColor, m_IsSmoothed);
        }
      }
    }
    else
    {
      Surface::Pointer surface;

      const auto timeSteps = imageMask->GetTimeSteps();
      for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
      {
        task.SetProgress(timeStep);

        auto binarized = BinarizeMask(imageMask->GetVtkImageData(timeStep));
        auto results = extractor.ExtractPerLabel(binarized, {1});
        auto it = results.find(1);
        if (it == results.end() || it->second == nullptr || it->second->GetNumberOfCells() == 0)
        {
          continue;
        }

        if (surface.IsNull())
        {
          surface = CreateSurface(imageMask->GetTimeGeometry());
        }

        surface->SetVtkPolyData(ApplyImageToWorld(it->second, imageMask->GetGeometry(timeStep)), timeStep);
      }

      if (surface.IsNotNull())
      {
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
}

void QmitkCreatePolygonModelAction::SetDataStorage(DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreatePolygonModelAction::SetFunctionality(QtViewPart *)
{
}
