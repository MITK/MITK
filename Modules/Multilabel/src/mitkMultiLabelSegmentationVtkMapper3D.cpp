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
#include <vtkGPUVolumeRayCastMapper.h>
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

namespace mitk
{
  class MultiLabelSegmentationGroupMapping
  {
  public:
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_VolumeMapper;
    vtkSmartPointer<vtkVolume> m_Volume;

    /** In highlighting mode used for all labels that are faded out.*/
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_FadedVolumeMapper;
    /** In highlighting mode used for all labels that are faded out.*/
    vtkSmartPointer<vtkVolume> m_FadedVolume;

    vtkSmartPointer<vtkImageData> m_VtkImage;
    //indicated the index it was added to the actor in order to identify if there are changes
    //in sequence
    MultiLabelSegmentation::GroupIndexType m_ActorOrder = 0;

    MultiLabelSegmentationGroupMapping()
    {
      m_VtkImage = vtkSmartPointer<vtkImageData>::New();
      m_VolumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
      m_Volume = vtkSmartPointer<vtkVolume>::New();
      m_FadedVolumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
      m_FadedVolume = vtkSmartPointer<vtkVolume>::New();
    }
  };
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
  const bool highlightInvisibleLabels = boolProp.IsNull() ? false : boolProp->GetValue();
  const bool highlightingActive = !highlightedLabelValues.empty();
  localStorage->m_UseFadedPipeline = highlightingActive;

  double rgba[4];
  for (const auto& value : labelValues)
  {
    lookUpTable->GetTableValue(value, rgba);
    bool isHighlightedValue = false;

    if (highlightingActive)
    {
      isHighlightedValue = highlightEnd != std::find(highlightedLabelValues.begin(), highlightedLabelValues.end(), value);
      if (!isHighlightedValue)
      { //make all none highlighted values more transparent
        rgba[3] *= 0.01;
      }
      else
      {
        if (rgba[3] != 0 || highlightInvisibleLabels)
        { //highlight a label if it is visible or if also invisible labels should be highlighted
          rgba[3] = 1.;
        }
      }
    }
    lookUpTable->SetTableValue(value, rgba);

    localStorage->m_TransferFunction->AddRGBPoint(value, rgba[0], rgba[1], rgba[2]);

    const double opacityNormal = (isHighlightedValue || !highlightingActive) ? rgba[3] : 0.;
    const double opacityFaded = !(isHighlightedValue || !highlightingActive) ? rgba[3] : 0.;

    localStorage->m_OpacityTransferFunction->AddPoint(value, opacityNormal);
    localStorage->m_FadedOpacityTransferFunction->AddPoint(value, opacityFaded);
  }
  localStorage->m_LabelLookupTable->Modified(); // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is

  localStorage->m_TransferFunction->Build();
  localStorage->m_OpacityTransferFunction->Modified();
  localStorage->m_FadedOpacityTransferFunction->Modified();
}

mitk::MultiLabelSegmentationVtkMapper3D::OutdatedGroupVectorType
mitk::MultiLabelSegmentationVtkMapper3D::CheckForOutdatedGroups(mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage* ls,
  mitk::MultiLabelSegmentation* seg, bool fadedPipelineChanged)
{
  assert(seg && seg->IsInitialized());

  const auto nrOfGroups = seg->GetNumberOfGroups();
  OutdatedGroupVectorType result;
  OutdatedGroupVectorType positionChanges;
  std::unordered_set<mitk::Image*> existingGroupImages;

  for (mitk::MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < nrOfGroups; ++groupID)
  {
    const auto groupImage = seg->GetGroupImage(groupID);
    existingGroupImages.insert(groupImage);

    auto finding = ls->m_GroupPipelines.find(groupImage);

    if (finding != ls->m_GroupPipelines.end())
    { //group image has a pipeline
      const bool imageIsOutdated = groupImage->GetMTime() > ls->m_LastDataUpdateTime
        || groupImage->GetPipelineMTime() > ls->m_LastDataUpdateTime;
      const bool groupPositionHasChanged = groupID != finding->second->m_ActorOrder;
      if (imageIsOutdated || groupPositionHasChanged)
      {
        result.push_back({ groupID, groupImage });

        if (groupPositionHasChanged) positionChanges.push_back({ groupID, groupImage });
      }
    }
    else
    { //new group image, we need a pipeline for that
      auto newPipeline = ls->m_GroupPipelines.insert(std::make_pair(groupImage, std::make_unique<mitk::MultiLabelSegmentationGroupMapping>()));

      //pipeline->m_VtkImage will be set and connected in the update function
      auto& pipeline = newPipeline.first->second;

      //configure the normal pipeline
      pipeline->m_Volume->GetProperty()->ShadeOn();
      pipeline->m_Volume->GetProperty()->SetDiffuse(1.0);
      pipeline->m_Volume->GetProperty()->SetAmbient(0.4);
      pipeline->m_Volume->GetProperty()->SetSpecular(0.2);
      pipeline->m_Volume->GetProperty()->SetInterpolationTypeToNearest();
      pipeline->m_Volume->SetMapper(pipeline->m_VolumeMapper);

      //configure the pipeline for faded labels in highlight mode
      pipeline->m_FadedVolume->GetProperty()->ShadeOff();
      pipeline->m_FadedVolume->GetProperty()->SetDiffuse(0.7);
      pipeline->m_FadedVolume->GetProperty()->SetAmbient(0.3);
      pipeline->m_FadedVolume->GetProperty()->SetSpecular(0.0);
      pipeline->m_FadedVolume->GetProperty()->SetInterpolationTypeToNearest();
      pipeline->m_FadedVolume->SetMapper(pipeline->m_FadedVolumeMapper);

      pipeline->m_ActorOrder = groupID;

      //new pipelines are always outdated
      result.push_back({ groupID, groupImage });
      positionChanges.push_back({ groupID, groupImage });
    }
  }

  //find all pipelines that refer to image that are not needed anymore and remove them
  std::vector < const mitk::Image*> missing;
  for (auto const& [key, value] : ls->m_GroupPipelines)
  {
    if (std::find(existingGroupImages.begin(), existingGroupImages.end(), key)
      == existingGroupImages.end())
    {
      missing.push_back(key);
    }
  }
  //now remove it
  for (auto& key : missing)
  {
    ls->m_Actors->RemovePart(ls->m_GroupPipelines[key]->m_Volume);
    ls->m_Actors->RemovePart(ls->m_GroupPipelines[key]->m_FadedVolume);
    ls->m_GroupPipelines.erase(key);
  }


  if (!positionChanges.empty() || fadedPipelineChanged)
  {
    // connect actor from scratch with all pipelines as some positions have changed
    // (this includes the case where a new group has been added or a group was deleted in between).
    // or if the usage state of the faded pipeline has changed
    ls->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
    for (auto& [key, pipeline] : ls->m_GroupPipelines)
    {
      ls->m_Actors->AddPart(pipeline->m_Volume);
      if (ls->m_UseFadedPipeline) ls->m_Actors->AddPart(pipeline->m_FadedVolume);
    }
  }

  return result;
}

void mitk::MultiLabelSegmentationVtkMapper3D::UpdateVolumeMapping(LocalStorage* localStorage, const OutdatedGroupVectorType& outdatedGroups)
{
  mitk::DataNode* node = this->GetDataNode();
  auto* segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
  assert(segmentation && segmentation->IsInitialized());

  segmentation->Update();

  //Compute normalized orientation matrix of segmentation to ensure that the volume is shown
  //at the right spot (same geometry like segmentation)
  const auto geometry = segmentation->GetGeometry();
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

  const auto timeStep = this->GetTimestep();

  for (auto& [groupID, groupImage] : outdatedGroups)
  {
    auto finding = localStorage->m_GroupPipelines.find(groupImage);
    if (finding == localStorage->m_GroupPipelines.end())
    {
      MITK_ERROR << "MultiLabelSegmentationVtkMapper3D is in invalid state. Group image is indicated as outdated that has no pipeline: Group image pointer: " << groupImage;
      return;
    }
    auto& pipeline = finding->second;
    pipeline->m_Volume->SetUserMatrix(orientationMatrix);

    // we could also search for the nonConst groupImage in segmentation, but the const cast
    // is faster and legit as we have access to the non const segmentation anyways.
    auto nonConstImage = const_cast<Image*>(groupImage);
    pipeline->m_VtkImage = nonConstImage->GetVtkImageData(timeStep);
    pipeline->m_VolumeMapper->SetInputData(pipeline->m_VtkImage);
    pipeline->m_FadedVolumeMapper->SetInputData(pipeline->m_VtkImage);

    // Force VTK to recompute scalar range from the actual data
    auto scalars = pipeline->m_VtkImage->GetPointData()->GetScalars();
    if (scalars)
    {
      // This forces VTK to scan the actual data and recompute range
      scalars->Modified();
      scalars->GetRange(); // Forces recomputation
    }

    // Force the vtkImageData to update its cached range
    pipeline->m_VtkImage->Modified();
    pipeline->m_VtkImage->GetScalarRange(); // This should now be correct

    pipeline->m_VolumeMapper->Update();
    pipeline->m_FadedVolumeMapper->Update();

    pipeline->m_Volume->GetProperty()->SetColor(localStorage->m_TransferFunction);
    pipeline->m_Volume->GetProperty()->SetScalarOpacity(localStorage->m_OpacityTransferFunction);
    pipeline->m_Volume->Update();
    pipeline->m_FadedVolume->GetProperty()->SetColor(localStorage->m_TransferFunction);
    pipeline->m_FadedVolume->GetProperty()->SetScalarOpacity(localStorage->m_FadedOpacityTransferFunction);
    pipeline->m_FadedVolume->Update();
  }

  localStorage->m_Actors->Modified();
  localStorage->m_LastDataUpdateTime.Modified();
  localStorage->m_LastUpdateTimeStep = timeStep;
}

void mitk::MultiLabelSegmentationVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
  assert(image && image->IsInitialized());
  image->Update();

  const bool isLookupModified = localStorage->m_LabelLookupTable.IsNull() ||
    (localStorage->m_LabelLookupTable->GetMTime() < image->GetLookupTable()->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, LabelHighlightGuard::PROPERTY_NAME_LABELS_HIGHLIGHTED(), localStorage->m_LabelLookupTable->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, LabelHighlightGuard::PROPERTY_NAME_HIGHLIGHT_INVISIBLE(), localStorage->m_LabelLookupTable->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, "opacity", localStorage->m_LabelLookupTable->GetMTime());

  const auto oldUseFadedPipeline = localStorage->m_UseFadedPipeline;
  if (isLookupModified)
  {
    this->UpdateLookupTable(localStorage);
  }
  const auto fadedPipelineChanged = oldUseFadedPipeline != localStorage->m_UseFadedPipeline;

  auto outdatedGroups = this->CheckForOutdatedGroups(localStorage, image, fadedPipelineChanged);

  const bool isGeometryModified = (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
    (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime());

  // check if visibility has been switched on since last update
  const bool visibilityChanged =
    PropertyTimeStampIsNewer(node, renderer, "visible", localStorage->m_LastDataUpdateTime) ||
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.3D.hide", localStorage->m_LastDataUpdateTime);

  const bool timeStepChanged = this->GetTimestep() != localStorage->m_LastUpdateTimeStep;

  if (isGeometryModified || visibilityChanged || isLookupModified || timeStepChanged)
  {
    //if geometry is outdated, lookup table, timestep or visibility changed all groups need regeneration
    outdatedGroups.clear();
    MultiLabelSegmentation::GroupIndexType groupID = 0;
    for (auto& [key, pipeline] : localStorage->m_GroupPipelines)
    {
      outdatedGroups.emplace_back(groupID, key);
    }
  }

  if (!outdatedGroups.empty())
  {
    this->UpdateVolumeMapping(localStorage, outdatedGroups);
  }
}

namespace
{
  bool IsGPUMapperSupported(mitk::BaseRenderer* renderer)
  {
    vtkNew<vtkImageData> tiny;
    tiny->SetDimensions(2, 2, 2);
    tiny->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

    vtkNew<vtkGPUVolumeRayCastMapper> mapper;
    mapper->SetInputData(tiny);

    bool supported = (0 != mapper->IsRenderSupported(renderer->GetVtkRenderer()->GetRenderWindow(), nullptr));

    return supported;
  }

}

void mitk::MultiLabelSegmentationVtkMapper3D::Update(mitk::BaseRenderer *renderer)
{
  auto localStorage = m_LSH.GetLocalStorage(renderer);
  const auto* node = this->GetDataNode();

  if (nullptr == localStorage || nullptr == node)
  {
    return;
  }

  if (!localStorage->m_GPUCheckSuccessfull.has_value())
  {
    const auto hasGPU = IsGPUMapperSupported(renderer);
    localStorage->m_GPUCheckSuccessfull = hasGPU;
    if (!hasGPU)
    {
      MITK_INFO << "No GPU available. 3D rendering of MultilabelSegmentation is deactivated for node: " << node->GetName();
    }
  }

  if (!localStorage->m_GPUCheckSuccessfull.value())
  {
    return;
  }

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

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::LocalStorage() : m_LastUpdateTimeStep(0)
{
  // Do as much actions as possible in here to avoid double executions.
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_TransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_FadedOpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

  m_UseFadedPipeline = false;
  m_TransferFunction->AllowDuplicateScalarsOff();
  m_TransferFunction->SetColorSpaceToRGB();

  m_TransferFunction->SetClamping(0);  // Disable clamping
  m_OpacityTransferFunction->SetClamping(0);  // Disable clamping

  m_TransferFunction->AddRGBPoint(0, 0., 0., 0.);
  m_OpacityTransferFunction->AddPoint(0, 0.);
  m_FadedOpacityTransferFunction->AddPoint(0, 0.);

  m_TransferFunction->AddRGBPoint(mitk::Label::MAX_LABEL_VALUE, 1., 1., 1.);
  m_OpacityTransferFunction->AddPoint(mitk::Label::MAX_LABEL_VALUE, 0.0);
  m_FadedOpacityTransferFunction->AddPoint(mitk::Label::MAX_LABEL_VALUE, 0.0);
}
