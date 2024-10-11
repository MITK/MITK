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
#include <mitkLabelHighlightGuard.h>

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
  auto lookUpTable = localStorage->m_LabelLookupTable->GetVtkLookupTable();

  const auto labelValues = image->GetAllLabelValues();

  mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(node->GetNonConstProperty(LabelHighlightGuard::PROPERTY_NAME_LABELS_HIGHLIGHTED()));

  if (nullptr != prop)
  {
    const auto highlightedLabelValues = prop->GetValue();
    mitk::BoolProperty::Pointer boolProp = dynamic_cast<mitk::BoolProperty*>(node->GetNonConstProperty(LabelHighlightGuard::PROPERTY_NAME_HIGHLIGHT_INVISIBLE()));
    bool higlightInvisible = boolProp.IsNull() ? false : boolProp->GetValue();

    if (!highlightedLabelValues.empty())
    {
      auto highlightEnd = highlightedLabelValues.cend();

      double rgba[4];
      for (const auto& value : labelValues)
      {
        lookUpTable->GetTableValue(value, rgba);
        if (highlightEnd == std::find(highlightedLabelValues.begin(), highlightedLabelValues.end(), value))
        { //make all none highlighted values more transparent
          rgba[3] *= 0.05;
        }
        else
        {
          if (higlightInvisible || rgba[3] != 0)
          {
            rgba[3] = 1.;
          }
        }
        lookUpTable->SetTableValue(value, rgba);
      }
      localStorage->m_LabelLookupTable->Modified(); // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is
      // executed
    }
  }

  const auto nrOfGroups = image->GetNumberOfLayers();
  for (unsigned int groupID = 0; groupID < nrOfGroups; ++groupID)
  {
    localStorage->m_TransferFunctions[groupID] = vtkSmartPointer<vtkColorTransferFunction>::New();
    localStorage->m_OpacityTransferFunctions[groupID] = vtkSmartPointer<vtkPiecewiseFunction>::New();

    localStorage->m_TransferFunctions[groupID]->AddRGBPoint(0, 0., 0., 1.);
    localStorage->m_OpacityTransferFunctions[groupID]->AddPoint(0, 0.);

    for (const auto& value : image->GetLabelValuesByGroup(groupID))
    {
      double* color = lookUpTable->GetTableValue(value);
      localStorage->m_TransferFunctions[groupID]->AddRGBPoint(value, color[0], color[1], color[2]);

      localStorage->m_OpacityTransferFunctions[groupID]->AddPoint(value, color[3]);
    }
  }
}

namespace
{
  std::vector<mitk::LabelSetImage::GroupIndexType> GetOutdatedGroups(const mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage* ls, const mitk::LabelSetImage* seg)
  {
    const auto nrOfGroups = seg->GetNumberOfLayers();
    std::vector<mitk::LabelSetImage::GroupIndexType> result;

    for (mitk::LabelSetImage::GroupIndexType groupID = 0; groupID < nrOfGroups; ++groupID)
    {
      const auto groupImage = seg->GetGroupImage(groupID);
      if (groupImage->GetMTime() > ls->m_LastDataUpdateTime
        || groupImage->GetPipelineMTime() > ls->m_LastDataUpdateTime
        || ls->m_GroupImageIDs.size() <= groupID
        || groupImage != ls->m_GroupImageIDs[groupID])
      {
        result.push_back(groupID);
      }
    }
    return result;
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
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.labels.highlighted", localStorage->m_LabelLookupTable->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.highlight_invisible", localStorage->m_LabelLookupTable->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, "opacity", localStorage->m_LabelLookupTable->GetMTime());

  auto outdatedGroups = GetOutdatedGroups(localStorage, image);

  bool isGeometryModified = (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
    (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime());

  // check if visibility has been switched on since last update
  bool visibilityChanged =
    PropertyTimeStampIsNewer(node, renderer, "visible", localStorage->m_LastDataUpdateTime) ||
    PropertyTimeStampIsNewer(node, renderer, "multilabel.3D.visualize", localStorage->m_LastDataUpdateTime);

  if (isGeometryModified || visibilityChanged)
  {
    //if geometry is outdated or visibility changed all groups need regeneration
    outdatedGroups.resize(image->GetNumberOfLayers());
    std::iota(outdatedGroups.begin(), outdatedGroups.end(), 0);
  }

  if (!outdatedGroups.empty())
  {
    auto hasValidContent = this->GenerateVolumeMapping(renderer, outdatedGroups);
    if (!hasValidContent) return;
  }

  if (isLookupModified)
  {
    this->GenerateLookupTable(renderer);
  }

  if (isLookupModified)
  {
    //if lookup table is modified all groups need a new color mapping
    outdatedGroups.resize(image->GetNumberOfLayers());
    std::iota(outdatedGroups.begin(), outdatedGroups.end(), 0);
  }

  for (const auto groupID : outdatedGroups)
  {
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetColor(localStorage->m_TransferFunctions[groupID]);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetScalarOpacity(localStorage->m_OpacityTransferFunctions[groupID]);
    localStorage->m_LayerVolumes[groupID]->Update();
  }
}

bool mitk::MultiLabelSegmentationVtkMapper3D::GenerateVolumeMapping(mitk::BaseRenderer* renderer, const std::vector<mitk::LabelSetImage::GroupIndexType>& outdatedGroupIDs)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  assert(image && image->IsInitialized());

  image->Update();

  const auto numberOfGroups = image->GetNumberOfLayers();

  if (numberOfGroups != localStorage->m_NumberOfGroups)
  {
    if (numberOfGroups > localStorage->m_NumberOfGroups)
    {
      for (unsigned int groupID = localStorage->m_NumberOfGroups; groupID < numberOfGroups; ++groupID)
      {
        localStorage->m_GroupImageIDs.push_back(nullptr);
        localStorage->m_LayerImages.push_back(vtkSmartPointer<vtkImageData>::New());
        localStorage->m_LayerVolumeMappers.push_back(vtkSmartPointer<vtkSmartVolumeMapper>::New());
        localStorage->m_LayerVolumes.push_back(vtkSmartPointer<vtkVolume>::New());

        localStorage->m_TransferFunctions.push_back(vtkSmartPointer<vtkColorTransferFunction>::New());
        localStorage->m_OpacityTransferFunctions.push_back(vtkSmartPointer<vtkPiecewiseFunction>::New());
      }
    }
    else
    {
      localStorage->m_GroupImageIDs.resize(numberOfGroups);
      localStorage->m_LayerImages.resize(numberOfGroups);
      localStorage->m_LayerVolumeMappers.resize(numberOfGroups);
      localStorage->m_LayerVolumes.resize(numberOfGroups);

      localStorage->m_TransferFunctions.resize(numberOfGroups);
      localStorage->m_OpacityTransferFunctions.resize(numberOfGroups);
    }

    localStorage->m_NumberOfGroups = numberOfGroups;

    //Compute normalized orientation matrix of image to ensure that the volume is shown
    //at the right spot (same geometry like image)
    const auto geometry = image->GetGeometry();
    auto spacing = geometry->GetSpacing();
    auto orientationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    orientationMatrix->DeepCopy(geometry->GetVtkMatrix());
    //normalize orientationMatrix
    for (int i = 0; i < 3; ++i)
    {
      orientationMatrix->SetElement(i, 0, orientationMatrix->GetElement(i, 0) / spacing[0]);
      orientationMatrix->SetElement(i, 1, orientationMatrix->GetElement(i, 1) / spacing[1]);
      orientationMatrix->SetElement(i, 2, orientationMatrix->GetElement(i, 2) / spacing[2]);
    }

    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

    for (unsigned int groupID = 0; groupID < numberOfGroups; ++groupID)
      localStorage->m_LayerVolumes[groupID]->SetUserMatrix(orientationMatrix);
  }

  if (localStorage->m_Actors->GetParts()->GetNumberOfItems() == 0)
  {
    for (unsigned int groupID = 0; groupID < numberOfGroups; ++groupID)
      localStorage->m_Actors->AddPart(localStorage->m_LayerVolumes[groupID]);
  }

  for (const auto groupID : outdatedGroupIDs)
  {
    const auto groupImage = image->GetGroupImage(groupID);
    localStorage->m_GroupImageIDs[groupID] = groupImage;

    localStorage->m_LayerImages[groupID] = groupImage->GetVtkImageData(this->GetTimestep());

    //need to recreate the volumeMapper because otherwise label data was still rendered even
    //if a label was removed. There must be a cleaner way to do it. Exchanging the whole mapper
    //is a ugly workaround for now.
    localStorage->m_LayerVolumeMappers[groupID] = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    localStorage->m_LayerVolumeMappers[groupID]->SetInputData(localStorage->m_LayerImages[groupID]);


    localStorage->m_LayerVolumes[groupID]->GetProperty()->ShadeOn();
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetDiffuse(1.0);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetAmbient(0.4);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetSpecular(0.2);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetInterpolationTypeToNearest();

    localStorage->m_LayerVolumes[groupID]->SetMapper(localStorage->m_LayerVolumeMappers[groupID]);
  }
  localStorage->m_LastDataUpdateTime.Modified();
  return true;
}

void mitk::MultiLabelSegmentationVtkMapper3D::Update(mitk::BaseRenderer *renderer)
{
  auto localStorage = m_LSH.GetLocalStorage(renderer);
  const auto* node = this->GetDataNode();

  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");

  bool has3Dvisualize = false;
  node->GetBoolProperty("multilabel.3D.visualize", has3Dvisualize, renderer);

  if (!visible || !has3Dvisualize)
  {
    // Nothing to see. Clear the actor. We regenerate its contents later if necessary.
    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
    return;
  }

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
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  // add/replace the following properties
  node->SetProperty("multilabel.3D.visualize", BoolProperty::New(false), renderer);
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::~LocalStorage()
{
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::LocalStorage()
{
  // Do as much actions as possible in here to avoid double executions.
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

  m_NumberOfGroups = 0;
}
