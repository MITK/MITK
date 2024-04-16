/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationVtkMapper3D.h"

// MITK
#include <mitkDataNode.h>
#include <mitkProperties.h>
#include <mitkVectorProperty.h>

// MITK Rendering

// VTK
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

namespace
{
  itk::ModifiedTimeType PropertyTimeStampIsNewer(const mitk::IPropertyProvider* provider, mitk::BaseRenderer* renderer, const std::string& propName, itk::ModifiedTimeType refMT)
  {
    const std::string context = renderer != nullptr ? renderer->GetName() : "";
    auto prop = provider->GetConstProperty(propName, context);
    if (prop != nullptr)
    {
      return prop->GetTimeStamp() > refMT;
    }
    return false;
  }
}

mitk::MultiLabelSegmentationVtkMapper3D::MultiLabelSegmentationVtkMapper3D()
{
}

mitk::MultiLabelSegmentationVtkMapper3D::~MultiLabelSegmentationVtkMapper3D()
{
}

vtkProp *mitk::MultiLabelSegmentationVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  // return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actors;
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage *mitk::MultiLabelSegmentationVtkMapper3D::GetLocalStorage(
  mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

void mitk::MultiLabelSegmentationVtkMapper3D::GenerateLookupTable(mitk::BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  assert(image && image->IsInitialized());

  localStorage->m_LabelLookupTable = image->GetLookupTable()->Clone();
  const auto labelValues = image->GetAllLabelValues();

  std::string propertyName = "org.mitk.multilabel.labels.highlighted";

  mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(node->GetNonConstProperty(propertyName));
  if (nullptr != prop)
  {
    const auto highlightedLabelValues = prop->GetValue();

    if (!highlightedLabelValues.empty())
    {
      auto lookUpTable = localStorage->m_LabelLookupTable->GetVtkLookupTable();
      auto highlightEnd = highlightedLabelValues.cend();

      double rgba[4];
      for (const auto& value : labelValues)
      {
        lookUpTable->GetTableValue(value, rgba);
        if (highlightEnd == std::find(highlightedLabelValues.begin(), highlightedLabelValues.end(), value))
        { //make all none highlighted values more transparent
          rgba[3] *= 0.15;
        }
        else if (rgba[3] != 0)
        { //if highlighted values are visible set them to opaque to pop out
          rgba[3] = 1.;
        }
        lookUpTable->SetTableValue(value, rgba);
      }
      localStorage->m_LabelLookupTable->Modified(); // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is
      // executed


      localStorage->m_TransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
      localStorage->m_OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

      localStorage->m_TransferFunction->AddRGBPoint(0, 0., 0., 1.);
      localStorage->m_OpacityTransferFunction->AddPoint(0, 0.);

      for (const auto& value : labelValues)
      {
        double* color = lookUpTable->GetTableValue(value);
        localStorage->m_TransferFunction->AddRGBPoint(value, color[0], color[1], color[2]);

        localStorage->m_OpacityTransferFunction->AddPoint(value, color[3]);
      }
    }
  }
}

void mitk::MultiLabelSegmentationVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode *node = this->GetDataNode();
  auto *image = dynamic_cast<mitk::LabelSetImage *>(node->GetData());
  assert(image && image->IsInitialized());

  bool isLookupModified = localStorage->m_LabelLookupTable.IsNull() ||
    (localStorage->m_LabelLookupTable->GetMTime() < image->GetLookupTable()->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.labels.highlighted", localStorage->m_LabelLookupTable->GetMTime());

  if (isLookupModified)
  {
    this->GenerateLookupTable(renderer);
  }

  bool isDataModified = (localStorage->m_LastDataUpdateTime < image->GetMTime()) ||
    (localStorage->m_LastDataUpdateTime < image->GetPipelineMTime()) ||
    (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
    (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime());

  if (isDataModified)
  {
    auto hasValidContent = this->GenerateVolumeMapping(renderer);
    if (!hasValidContent) return;
  }

  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  auto numberOfLayers = image->GetNumberOfLayers();
  for (int lidx = 0; lidx < numberOfLayers; ++lidx)
  {
    localStorage->m_LayerVolumeProperties[lidx]->SetColor(localStorage->m_TransferFunction);
    localStorage->m_LayerVolumeProperties[lidx]->SetScalarOpacity(localStorage->m_OpacityTransferFunction);
    localStorage->m_LayerVolumes[lidx]->SetMapper(localStorage->m_LayerVolumeMappers[lidx]);
    localStorage->m_LayerVolumes[lidx]->SetProperty(localStorage->m_LayerVolumeProperties[lidx]);
  }
}

bool mitk::MultiLabelSegmentationVtkMapper3D::GenerateVolumeMapping(mitk::BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  assert(image && image->IsInitialized());

  image->Update();

  auto numberOfLayers = image->GetNumberOfLayers();

  if (numberOfLayers != localStorage->m_NumberOfLayers)
  {
    localStorage->m_NumberOfLayers = numberOfLayers;
    localStorage->m_LayerImages.clear();
    localStorage->m_LayerVolumeMappers.clear();
    localStorage->m_LayerVolumeProperties.clear();
    localStorage->m_LayerVolumes.clear();

    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

    for (int lidx = 0; lidx < numberOfLayers; ++lidx)
    {
      localStorage->m_LayerImages.push_back(vtkSmartPointer<vtkImageData>::New());
      localStorage->m_LayerVolumeMappers.push_back(vtkSmartPointer<vtkSmartVolumeMapper>::New());
      localStorage->m_LayerVolumeProperties.push_back(vtkSmartPointer<vtkVolumeProperty>::New());
      localStorage->m_LayerVolumes.push_back(vtkSmartPointer<vtkVolume>::New());

      localStorage->m_Actors->AddPart(localStorage->m_LayerVolumes[lidx]);
    }
  }

  for (int lidx = 0; lidx < numberOfLayers; ++lidx)
  {
    const auto layerImage = image->GetGroupImage(lidx);

    localStorage->m_LayerImages[lidx] = layerImage->GetVtkImageData(this->GetTimestep());
    localStorage->m_LayerVolumeMappers[lidx]->SetInputData(localStorage->m_LayerImages[lidx]);
    localStorage->m_LayerVolumeProperties[lidx]->ShadeOff();
    localStorage->m_LayerVolumeProperties[lidx]->SetInterpolationTypeToNearest();
  }
  localStorage->m_LastDataUpdateTime.Modified();
  return true;
}

void mitk::MultiLabelSegmentationVtkMapper3D::Update(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  const DataNode *node = this->GetDataNode();
  node->GetVisibility(visible, renderer, "visible");

  if (!visible)
    return;

  auto *image = dynamic_cast<mitk::LabelSetImage *>(node->GetData());

  if (image == nullptr || image->IsInitialized() == false)
    return;

  // Calculate time step of the image data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = image->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimeStep(this->GetTimestep())))
  {
    return;
  }

  image->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // check if something important has changed and we need to re-render

  if (localStorage->m_LabelLookupTable.IsNull() ||
      (localStorage->m_LabelLookupTable->GetMTime() < image->GetLookupTable()->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < image->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < image->GetPipelineMTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < image->GetPropertyList()->GetMTime()))
  {
    this->GenerateDataForRenderer(renderer);
    localStorage->m_LastPropertyUpdateTime.Modified();
  }
}

void mitk::MultiLabelSegmentationVtkMapper3D::SetDefaultProperties(mitk::DataNode *node,
                                                          mitk::BaseRenderer *renderer,
                                                          bool overwrite)
{
  // add/replace the following properties
  node->SetProperty("opacity", FloatProperty::New(1.0f), renderer);

  node->SetProperty("labelset.contour.active", BoolProperty::New(true), renderer);
  node->SetProperty("labelset.contour.width", FloatProperty::New(2.0), renderer);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::~LocalStorage()
{
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::LocalStorage()
{
  // Do as much actions as possible in here to avoid double executions.
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

  m_NumberOfLayers = 0;
}
