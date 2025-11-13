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
#include <vtkPointData.h>
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

void mitk::MultiLabelSegmentationVtkMapper3D::UpdateLookupTable(LocalStorage* localStorage)
{
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
  assert(image && image->IsInitialized());

  localStorage->m_LabelLookupTable = image->GetLookupTable()->Clone();
  auto lookUpTable = localStorage->m_LabelLookupTable->GetVtkLookupTable();

  const auto labelValues = image->GetAllLabelValues();

  mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(node->GetNonConstProperty(LabelHighlightGuard::PROPERTY_NAME_LABELS_HIGHLIGHTED()));
  const auto highlightedLabelValues = prop.IsNotNull() ? prop->GetValue() : std::vector<int>({});
  auto highlightEnd = highlightedLabelValues.cend();

  mitk::BoolProperty::Pointer boolProp = dynamic_cast<mitk::BoolProperty*>(node->GetNonConstProperty(LabelHighlightGuard::PROPERTY_NAME_HIGHLIGHT_INVISIBLE()));
  const bool higlightInvisible = boolProp.IsNull() ? false : boolProp->GetValue();

  double rgba[4];
  for (const auto& value : labelValues)
  {
    lookUpTable->GetTableValue(value, rgba);
    if (!highlightedLabelValues.empty() && highlightEnd == std::find(highlightedLabelValues.begin(), highlightedLabelValues.end(), value))
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

    localStorage->m_TransferFunction->AddRGBPoint(value, rgba[0], rgba[1], rgba[2]);
    localStorage->m_OpacityTransferFunction->AddPoint(value, rgba[3]);
  }
  localStorage->m_LabelLookupTable->Modified(); // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is

  localStorage->m_TransferFunction->Build();
  localStorage->m_OpacityTransferFunction->Modified();
}

namespace
{
  std::vector<mitk::MultiLabelSegmentation::GroupIndexType> GetOutdatedGroups(const mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage* ls, const mitk::MultiLabelSegmentation* seg)
  {
    const auto nrOfGroups = seg->GetNumberOfGroups();
    std::vector<mitk::MultiLabelSegmentation::GroupIndexType> result;

    for (mitk::MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < nrOfGroups; ++groupID)
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
  auto *image = dynamic_cast<mitk::MultiLabelSegmentation *>(node->GetData());
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
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.3D.hide", localStorage->m_LastDataUpdateTime);

  if (isGeometryModified || visibilityChanged)
  {
    //if geometry is outdated or visibility changed all groups need regeneration
    outdatedGroups.resize(image->GetNumberOfGroups());
    std::iota(outdatedGroups.begin(), outdatedGroups.end(), 0);
  }

  if (!outdatedGroups.empty())
  {
    auto hasValidContent = this->GenerateVolumeMapping(localStorage, outdatedGroups);
    if (!hasValidContent) return;
  }

  if (isLookupModified)
  {
    this->UpdateLookupTable(localStorage);
  }

  if (isLookupModified)
  {
    //if lookup table is modified all groups need a new color mapping
    outdatedGroups.resize(image->GetNumberOfGroups());
    std::iota(outdatedGroups.begin(), outdatedGroups.end(), 0);
  }

  for (const auto groupID : outdatedGroups)
  {
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetColor(localStorage->m_TransferFunction);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetScalarOpacity(localStorage->m_OpacityTransferFunction);
    localStorage->m_LayerVolumes[groupID]->Update();
  }
}

bool mitk::MultiLabelSegmentationVtkMapper3D::GenerateVolumeMapping(LocalStorage* localStorage, const std::vector<mitk::MultiLabelSegmentation::GroupIndexType>& outdatedGroupIDs)
{
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
  assert(image && image->IsInitialized());

  image->Update();

  const auto currentNumberOfGroups = localStorage->m_GroupImageIDs.size();
  const auto numberOfGroups = image->GetNumberOfGroups();

  if (numberOfGroups != currentNumberOfGroups)
  {
    localStorage->m_GroupImageIDs.resize(numberOfGroups);
    localStorage->m_LayerImages.resize(numberOfGroups);
    localStorage->m_LayerVolumeMappers.resize(numberOfGroups);
    localStorage->m_LayerVolumes.resize(numberOfGroups);

    if (numberOfGroups > currentNumberOfGroups)
    {
      for (unsigned int groupID = currentNumberOfGroups; groupID < numberOfGroups; ++groupID)
      {
        localStorage->m_GroupImageIDs[groupID] = nullptr;
        localStorage->m_LayerImages[groupID] = vtkSmartPointer<vtkImageData>::New();
        localStorage->m_LayerVolumeMappers[groupID] = vtkSmartPointer<vtkSmartVolumeMapper>::New();
        localStorage->m_LayerVolumes[groupID] = vtkSmartPointer<vtkVolume>::New();
      }
    }

    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  }

  //Compute normalized orientation matrix of segmentation to ensure that the volume is shown
  //at the right spot (same geometry like segmentation)
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

  for (unsigned int groupID = 0; groupID < numberOfGroups; ++groupID)
    localStorage->m_LayerVolumes[groupID]->SetUserMatrix(orientationMatrix);

  for (const auto groupID : outdatedGroupIDs)
  {
    const auto groupImage = image->GetGroupImage(groupID);
    localStorage->m_GroupImageIDs[groupID] = groupImage;

    localStorage->m_LayerImages[groupID] = groupImage->GetVtkImageData(this->GetTimestep());

    // Force VTK to recompute scalar range from the actual data
    auto scalars = localStorage->m_LayerImages[groupID]->GetPointData()->GetScalars();
    if (scalars)
    {
      // This forces VTK to scan the actual data and recompute range
      scalars->Modified();
      scalars->GetRange(); // Forces recomputation
    }

    // Force the vtkImageData to update its cached range
    localStorage->m_LayerImages[groupID]->Modified();
    localStorage->m_LayerImages[groupID]->GetScalarRange(); // This should now be correct


    localStorage->m_LayerVolumeMappers[groupID]->SetInputData(localStorage->m_LayerImages[groupID]);



    // DIAGNOSTIC: Check the actual data
    double actualRange[2];
    localStorage->m_LayerImages[groupID]->GetScalarRange(actualRange);
    std::cout << "Group " << groupID << " vtkImageData scalar range: ["
      << actualRange[0] << ", " << actualRange[1] << "]" << std::endl;

    // DIAGNOSTIC: Check what the mapper sees
    localStorage->m_LayerVolumeMappers[groupID]->Update();
    double mapperRange[2];

    localStorage->m_LayerVolumeMappers[groupID]->GetInput()->GetScalarRange(mapperRange);
    // DIAGNOSTIC END

    std::cout << "Mapper sees scalar range: ["
      << mapperRange[0] << ", " << mapperRange[1] << "]" << std::endl;

    localStorage->m_LayerVolumes[groupID]->GetProperty()->ShadeOn();
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetDiffuse(1.0);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetAmbient(0.4);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetSpecular(0.2);
    localStorage->m_LayerVolumes[groupID]->GetProperty()->SetInterpolationTypeToNearest();

    localStorage->m_LayerVolumes[groupID]->SetMapper(localStorage->m_LayerVolumeMappers[groupID]);
  }

  if (localStorage->m_Actors->GetParts()->GetNumberOfItems() == 0)
  {
    for (unsigned int groupID = 0; groupID < numberOfGroups; ++groupID)
    {
      localStorage->m_Actors->AddPart(localStorage->m_LayerVolumes[groupID]);
    }
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

  bool hide3Dvisualize = false;
  node->GetBoolProperty("org.mitk.multilabel.3D.hide", hide3Dvisualize, renderer);

  if (!visible || hide3Dvisualize)
  {
    // Nothing to see. Clear the actor. We regenerate its contents later if necessary.
    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
    return;
  }

  auto *segmentation = dynamic_cast<mitk::MultiLabelSegmentation *>(node->GetData());

  if (segmentation == nullptr || segmentation->IsInitialized() == false)
  {
    // Nothing to see. Clear the actor. We regenerate its contents later if necessary.
    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
    return;
  }

  // Calculate time step of the segmentation data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = segmentation->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimeStep(this->GetTimestep())))
  {
    return;
  }

  segmentation->UpdateOutputInformation();

  // check if something important has changed and we need to re-render

  if (localStorage->m_LabelLookupTable.IsNull() ||
      (localStorage->m_LabelLookupTable->GetMTime() < segmentation->GetLookupTable()->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < segmentation->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < segmentation->GetPipelineMTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < segmentation->GetPropertyList()->GetMTime()))
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
  node->SetProperty("org.mitk.multilabel.3D.hide", BoolProperty::New(false), renderer);
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::~LocalStorage()
{
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::LocalStorage()
{
  // Do as much actions as possible in here to avoid double executions.
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_TransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

  m_TransferFunction->AllowDuplicateScalarsOff();
  m_TransferFunction->SetColorSpaceToRGB();

  m_TransferFunction->SetClamping(0);  // Disable clamping
  m_OpacityTransferFunction->SetClamping(0);  // Disable clamping

  m_TransferFunction->AddRGBPoint(0, 0., 0., 0.);
  m_OpacityTransferFunction->AddPoint(0, 0.);
  m_TransferFunction->AddRGBPoint(mitk::Label::MAX_LABEL_VALUE, 1., 1., 1.);
  m_OpacityTransferFunction->AddPoint(mitk::Label::MAX_LABEL_VALUE, 0.5);

}
