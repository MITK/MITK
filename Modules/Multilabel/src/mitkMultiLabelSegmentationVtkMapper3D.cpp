/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSegmentationVtkMapper3D.h>

// MITK
#include <mitkDataNode.h>
#include <mitkLabelHighlightGuard.h>
#include <mitkMultiLabelSurfaceNetsExtractor.h>
#include <mitkProperties.h>
#include <mitkVectorProperty.h>

#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

// VTK
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>

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
    std::unique_ptr<MultiLabelSurfaceNetsExtractor> m_Extractor;
    vtkSmartPointer<vtkPolyDataMapper>              m_PolyMapper;
    vtkSmartPointer<vtkActor>                       m_Actor;
    vtkSmartPointer<vtkImageData>                   m_VtkImage;

    // The actor index in the group order. Used to detect group reordering.
    MultiLabelSegmentation::GroupIndexType m_ActorOrder = 0;

    MultiLabelSegmentationGroupMapping()
      : m_Extractor(std::make_unique<MultiLabelSurfaceNetsExtractor>()),
        m_PolyMapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
        m_Actor(vtkSmartPointer<vtkActor>::New()),
        m_VtkImage(vtkSmartPointer<vtkImageData>::New())
    {
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
  auto sourceLookUpTable = localStorage->m_LabelLookupTable->GetVtkLookupTable();

  const auto labelValues = image->GetAllLabelValues();

  mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(node->GetNonConstProperty(LabelHighlightGuard::PROPERTY_NAME_LABELS_HIGHLIGHTED()));
  const auto highlightedLabelValues = prop.IsNotNull() ? prop->GetValue() : std::vector<int>({});
  auto highlightEnd = highlightedLabelValues.cend();

  mitk::BoolProperty::Pointer boolProp = dynamic_cast<mitk::BoolProperty*>(node->GetNonConstProperty(LabelHighlightGuard::PROPERTY_NAME_HIGHLIGHT_INVISIBLE()));
  const bool highlightInvisibleLabels = boolProp.IsNull() ? false : boolProp->GetValue();
  const bool highlightingActive = !highlightedLabelValues.empty();
  localStorage->m_UseFadedPipeline = highlightingActive;

  float nodeOpacity = 1.0f;
  node->GetFloatProperty("opacity", nodeOpacity);

  // Reset the entire LUT so labels that were removed since the last update
  // map to fully transparent. The MAX_LABEL_VALUE+1 entries are configured
  // once in the LocalStorage constructor; here we just clear the contents.
  auto& lut = localStorage->m_VtkLookupTable;
  for (vtkIdType i = 0; i <= mitk::Label::MAX_LABEL_VALUE; ++i)
  {
    lut->SetTableValue(i, 0.0, 0.0, 0.0, 0.0);
  }

  double rgba[4];
  for (const auto& value : labelValues)
  {
    sourceLookUpTable->GetTableValue(value, rgba);

    if (highlightingActive)
    {
      const bool isHighlightedValue = highlightEnd != std::find(highlightedLabelValues.begin(), highlightedLabelValues.end(), value);
      if (!isHighlightedValue)
      { //make all none highlighted values more transparent
        rgba[3] *= 0.3;
      }
      else
      {
        if (rgba[3] != 0 || highlightInvisibleLabels)
        {
          rgba[3] = 1.0;
        }
      }
    }

    // Node-level "opacity" multiplies into the per-label alpha. The actor's own
    // opacity is fixed at 1.0 so the LUT alpha is the sole source of transparency.
    rgba[3] *= nodeOpacity;

    lut->SetTableValue(value, rgba);
  }

  localStorage->m_LabelLookupTable->Modified();
  lut->Modified();
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
    {
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
    {
      auto newPipeline = ls->m_GroupPipelines.insert(std::make_pair(groupImage, std::make_unique<mitk::MultiLabelSegmentationGroupMapping>()));

      auto& pipeline = newPipeline.first->second;

      // Configure the per-group polydata mapper. The mapper's input is set per render
      // by UpdateSurfaceMapping with the polydata returned by the surface-nets extractor.
      pipeline->m_PolyMapper->SetScalarModeToUseCellFieldData();
      pipeline->m_PolyMapper->SelectColorArray("BoundaryLabels");
      // BoundaryLabels component 0 always holds a foreground label: vtkSurfaceNets3D
      // places the background value into component 1 for boundary cells, and orders
      // foreground values ascending for interior (label-vs-label) cells. Component 0
      // is therefore the right scalar to drive label-color lookup.
      pipeline->m_PolyMapper->SetArrayComponent(0);
      pipeline->m_PolyMapper->SetUseLookupTableScalarRange(true);
      pipeline->m_PolyMapper->SetLookupTable(ls->m_VtkLookupTable);
      pipeline->m_PolyMapper->ScalarVisibilityOn();
      pipeline->m_PolyMapper->InterpolateScalarsBeforeMappingOff();

      pipeline->m_Actor->SetMapper(pipeline->m_PolyMapper);
      pipeline->m_Actor->GetProperty()->SetAmbient(0.4);
      pipeline->m_Actor->GetProperty()->SetDiffuse(1.0);
      pipeline->m_Actor->GetProperty()->SetSpecular(0.2);
      pipeline->m_Actor->GetProperty()->SetSpecularPower(20.0);
      pipeline->m_Actor->GetProperty()->SetInterpolationToPhong();
      pipeline->m_Actor->GetProperty()->SetBackfaceCulling(false);
      // Opacity is encoded in the lookup table; keep the actor opacity at 1.0 so it
      // does not stack multiplicatively with the per-label alpha.
      pipeline->m_Actor->GetProperty()->SetOpacity(1.0);

      pipeline->m_ActorOrder = groupID;

      // New pipelines are always outdated.
      result.push_back({ groupID, groupImage });
      positionChanges.push_back({ groupID, groupImage });
    }
  }

  // Find pipelines that refer to images no longer in the segmentation and remove them.
  std::vector<const mitk::Image*> missing;
  for (auto const& [key, value] : ls->m_GroupPipelines)
  {
    if (std::find(existingGroupImages.begin(), existingGroupImages.end(), key)
      == existingGroupImages.end())
    {
      missing.push_back(key);
    }
  }
  for (auto& key : missing)
  {
    ls->m_Actors->RemovePart(ls->m_GroupPipelines[key]->m_Actor);
    ls->m_GroupPipelines.erase(key);
  }

  if (!positionChanges.empty()
    || fadedPipelineChanged
    || (!ls->m_GroupPipelines.empty() && ls->m_Actors->GetParts()->GetNumberOfItems() == 0)
    )
  {
    // Reassemble the actor set from scratch: positions changed (group added, removed,
    // reordered), the highlight-mode flag flipped, or rendering was previously empty
    // (e.g., the 3D-rendering preference was just turned on).
    ls->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
    for (auto& [key, pipeline] : ls->m_GroupPipelines)
    {
      ls->m_Actors->AddPart(pipeline->m_Actor);
    }
  }

  return result;
}

void mitk::MultiLabelSegmentationVtkMapper3D::UpdateSurfaceMapping(LocalStorage* localStorage, const OutdatedGroupVectorType& outdatedGroups)
{
  mitk::DataNode* node = this->GetDataNode();
  auto* segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
  assert(segmentation && segmentation->IsInitialized());

  segmentation->Update();

  // Compute normalized orientation matrix of segmentation to ensure that the surface
  // is shown at the right spot (same geometry as the segmentation).
  const auto geometry = segmentation->GetGeometry();
  auto spacing = geometry->GetSpacing();
  auto orientationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  orientationMatrix->DeepCopy(geometry->GetVtkMatrix());
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

    pipeline->m_Actor->SetUserMatrix(orientationMatrix);

    // we could also search for the nonConst groupImage in segmentation, but the const cast
    // is faster and legit as we have access to the non const segmentation anyways.
    auto nonConstImage = const_cast<Image*>(groupImage);
    pipeline->m_VtkImage = nonConstImage->GetVtkImageData(timeStep);

    pipeline->m_Extractor->SetSmoothing(localStorage->m_LastSmoothed);

    const auto groupLabels = segmentation->GetLabelValuesByGroup(groupID);
    auto polyData = pipeline->m_Extractor->Extract(pipeline->m_VtkImage, groupLabels);
    pipeline->m_PolyMapper->SetInputData(polyData);
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

  const bool visibilityChanged =
    PropertyTimeStampIsNewer(node, renderer, "visible", localStorage->m_LastDataUpdateTime) ||
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.3D.hide", localStorage->m_LastDataUpdateTime);

  const bool timeStepChanged = this->GetTimestep() != localStorage->m_LastUpdateTimeStep;

  // Resolve the smoothing state from the per-node property if set, otherwise from the
  // segmentation preference. A change in the resolved state forces all groups to re-extract
  // (the smoothing flag is applied to vtkSurfaceNets3D in UpdateSurfaceMapping).
  bool currentSmoothed = true;
  if (!node->GetBoolProperty("org.mitk.multilabel.3D.smoothed", currentSmoothed, renderer))
  {
    if (nullptr != localStorage->m_SegPreferences)
    {
      currentSmoothed = localStorage->m_SegPreferences->GetBool("3D rendering smoothed", true);
    }
  }
  const bool smoothedChanged = currentSmoothed != localStorage->m_LastSmoothed;
  localStorage->m_LastSmoothed = currentSmoothed;

  // Lookup-only changes (color, alpha, highlight, per-label visibility via alpha=0) do not
  // require surface re-extraction: the polydata mapper picks up the LUT change automatically.
  if (isGeometryModified || visibilityChanged || timeStepChanged || smoothedChanged)
  {
    outdatedGroups.clear();
    for (auto& [key, pipeline] : localStorage->m_GroupPipelines)
    {
      outdatedGroups.emplace_back(pipeline->m_ActorOrder, key);
    }
  }

  if (!outdatedGroups.empty())
  {
    this->UpdateSurfaceMapping(localStorage, outdatedGroups);
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

  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");

  bool hide3Dvisualize = false;
  node->GetBoolProperty("org.mitk.multilabel.3D.hide", hide3Dvisualize, renderer);

  const auto pref3DRendering = nullptr != localStorage->m_SegPreferences ? localStorage->m_SegPreferences->GetBool("activate 3D rendering", true) : true;
  const auto changed3DRendering = pref3DRendering != localStorage->m_3DRenderingPreference;
  localStorage->m_3DRenderingPreference = pref3DRendering;

  // Detect a change in the resolved smoothing state so a preference flip
  // (without any per-node property change) still triggers re-extraction.
  bool resolvedSmoothed = true;
  if (!node->GetBoolProperty("org.mitk.multilabel.3D.smoothed", resolvedSmoothed, renderer))
  {
    if (nullptr != localStorage->m_SegPreferences)
    {
      resolvedSmoothed = localStorage->m_SegPreferences->GetBool("3D rendering smoothed", true);
    }
  }
  const auto changedSmoothed = resolvedSmoothed != localStorage->m_LastSmoothed;

  if (!visible
    || hide3Dvisualize
    || !pref3DRendering)
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

  if (localStorage->m_LabelLookupTable.IsNull() ||
      (localStorage->m_LabelLookupTable->GetMTime() < segmentation->GetLookupTable()->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < segmentation->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < segmentation->GetPipelineMTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < segmentation->GetPropertyList()->GetMTime()) ||
      changed3DRendering ||
      changedSmoothed)
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

  node->SetProperty("org.mitk.multilabel.3D.hide", BoolProperty::New(false), renderer);
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::~LocalStorage()
{
}

mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage::LocalStorage() : m_LastUpdateTimeStep(0)
{
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

  m_VtkLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_VtkLookupTable->SetNumberOfTableValues(static_cast<vtkIdType>(mitk::Label::MAX_LABEL_VALUE) + 1);
  m_VtkLookupTable->SetTableRange(0.0, static_cast<double>(mitk::Label::MAX_LABEL_VALUE));
  m_VtkLookupTable->IndexedLookupOff();
  // Initialize all entries to fully transparent so unused label values do not pick up
  // a default ramp color when referenced by surface-nets cell scalars.
  for (vtkIdType i = 0; i <= mitk::Label::MAX_LABEL_VALUE; ++i)
  {
    m_VtkLookupTable->SetTableValue(i, 0.0, 0.0, 0.0, 0.0);
  }
  m_VtkLookupTable->Build();

  m_UseFadedPipeline = false;

  m_SegPreferences = nullptr;
  m_3DRenderingPreference = true;
  m_LastSmoothed = true;

  auto prefService = mitk::CoreServices::GetPreferencesService();
  if (nullptr != prefService)
  {
    auto systemPref = prefService->GetSystemPreferences();
    if (nullptr != systemPref)
    {
      m_SegPreferences = systemPref->Node("/org.mitk.views.segmentation");
    }
  }
}
