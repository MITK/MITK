/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSurfaceNetsExtractor.h>

#include <mitkBaseGeometry.h>

#include <vtkCellData.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSurfaceNets3D.h>
#include <vtkSurfaceNetsAtlas.h>

namespace
{
  void PrepareInput(vtkImageData* groupImage)
  {
    // The vtkImageData wraps an MITK-owned buffer via SetVoidArray; in-place edits do
    // not bump the wrapper's MTime. Force VTK to treat the data as modified so the
    // pipeline re-executes against the current contents.
    if (auto* scalars = groupImage->GetPointData()->GetScalars())
      scalars->Modified();

    groupImage->Modified();
  }

  vtkSmartPointer<vtkPolyData> ComputeNormals(vtkPolyDataNormals* normalsFilter, vtkPolyData* mesh)
  {
    normalsFilter->SetInputData(mesh);
    normalsFilter->Update();

    auto output = vtkSmartPointer<vtkPolyData>::New();
    output->DeepCopy(normalsFilter->GetOutput());
    return output;
  }
}

mitk::MultiLabelSurfaceNetsExtractor::MultiLabelSurfaceNetsExtractor()
  : m_SurfaceNets(vtkSmartPointer<vtkSurfaceNets3D>::New()),
    m_NormalsFilter(vtkSmartPointer<vtkPolyDataNormals>::New()),
    m_Smoothing(true)
{
  m_SurfaceNets->SetOutputMeshTypeToTriangles();

  m_NormalsFilter->SplittingOn();
  m_NormalsFilter->ConsistencyOn();
  m_NormalsFilter->AutoOrientNormalsOn();
  m_NormalsFilter->ComputePointNormalsOn();
  m_NormalsFilter->ComputeCellNormalsOff();
  m_NormalsFilter->SetFeatureAngle(30.0);
}

mitk::MultiLabelSurfaceNetsExtractor::~MultiLabelSurfaceNetsExtractor() = default;

void mitk::MultiLabelSurfaceNetsExtractor::SetSmoothing(bool smoothing)
{
  m_Smoothing = smoothing;
}

bool mitk::MultiLabelSurfaceNetsExtractor::GetSmoothing() const
{
  return m_Smoothing;
}

void mitk::MultiLabelSurfaceNetsExtractor::UpdateSurfaceNets(
  vtkImageData* groupImage,
  const std::vector<LabelValueType>& labelValues)
{
  PrepareInput(groupImage);

  m_SurfaceNets->SetInputData(groupImage);
  m_SurfaceNets->SetSmoothing(m_Smoothing);
  m_SurfaceNets->SetNumberOfLabels(static_cast<int>(labelValues.size()));
  for (size_t i = 0; i < labelValues.size(); ++i)
    m_SurfaceNets->SetLabel(static_cast<int>(i), static_cast<double>(labelValues[i]));

  // vtkSurfaceNets3D reuses its boundary cache unless its own MTime advanced, but
  // SetLabel and SetInputData only touch the contour values and the input producer.
  // A reused cache skips the "BoundaryLabels" allocation and TransformMeshType then
  // dereferences null in the smoothed path (still the case in VTK 9.7).
  m_SurfaceNets->Modified();
  m_SurfaceNets->Update();
}

vtkSmartPointer<vtkPolyData> mitk::MultiLabelSurfaceNetsExtractor::Extract(
  vtkImageData* groupImage,
  const std::vector<LabelValueType>& labelValues)
{
  if (groupImage == nullptr || labelValues.empty())
    return vtkSmartPointer<vtkPolyData>::New();

  this->UpdateSurfaceNets(groupImage, labelValues);

  return ComputeNormals(m_NormalsFilter, m_SurfaceNets->GetOutput());
}

std::map<mitk::MultiLabelSurfaceNetsExtractor::LabelValueType, vtkSmartPointer<vtkPolyData>>
mitk::MultiLabelSurfaceNetsExtractor::ExtractPerLabel(
  vtkImageData* groupImage,
  const std::vector<LabelValueType>& labelValues)
{
  std::map<LabelValueType, vtkSmartPointer<vtkPolyData>> results;
  if (groupImage == nullptr || labelValues.empty())
    return results;

  this->UpdateSurfaceNets(groupImage, labelValues);

  // The atlas splits the shared surface net into per-label regions without re-running
  // the extraction, so the smoothing stays consistent across adjacent labels.
  auto atlas = vtkSmartPointer<vtkSurfaceNetsAtlas>::New();
  atlas->SetInputConnection(m_SurfaceNets->GetOutputPort());
  atlas->SetBackgroundLabel(static_cast<vtkIdType>(m_SurfaceNets->GetBackgroundLabel()));
  atlas->SetExtractionModeToLabelSet();
  atlas->SetOutputStyleToAll();
  atlas->GeneratePatchesOff();
  atlas->Update(); // Builds the label database; the empty selection emits no regions yet.

  for (auto label : labelValues)
  {
    const auto atlasLabel = static_cast<vtkIdType>(label);
    if (!atlas->HasLabel(atlasLabel))
      continue;

    atlas->ClearSelectedLabels();
    atlas->AddSelectedLabel(atlasLabel);
    atlas->Update();

    auto* regions = atlas->GetOutput();
    if (regions->GetNumberOfPartitionedDataSets() == 0 || regions->GetNumberOfPartitions(0) == 0)
      continue;

    auto* region = vtkPolyData::SafeDownCast(regions->GetPartitionAsDataObject(0, 0));
    if (region == nullptr || region->GetNumberOfCells() == 0)
      continue;

    auto polyData = ComputeNormals(m_NormalsFilter, region);

    // Atlas bookkeeping that is not part of this class's output contract.
    polyData->GetCellData()->RemoveArray("Label");
    polyData->GetCellData()->RemoveArray("LID");
    polyData->GetFieldData()->RemoveArray("AdjacentLabels");
    polyData->GetFieldData()->RemoveArray("PatchIDs");

    results[label] = polyData;
  }

  return results;
}

vtkSmartPointer<vtkMatrix4x4> mitk::MultiLabelSurfaceNetsExtractor::GetImageToWorldMatrix(const BaseGeometry* geometry)
{
  auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  matrix->Identity();
  if (geometry == nullptr)
    return matrix;

  // GetVtkMatrix() returns the index-to-world matrix where the rotation columns are
  // pre-scaled by spacing. vtkSurfaceNets3D consumes a vtkImageData whose spacing is
  // already applied (origin (0,0,0), identity direction), so the polydata it emits is
  // in mm-scaled image-local coordinates. Strip the spacing here to leave a pure
  // [direction | origin] transform that maps those coordinates into world space.
  matrix->DeepCopy(geometry->GetVtkMatrix());
  const auto spacing = geometry->GetSpacing();
  for (int i = 0; i < 3; ++i)
  {
    matrix->SetElement(i, 0, matrix->GetElement(i, 0) / spacing[0]);
    matrix->SetElement(i, 1, matrix->GetElement(i, 1) / spacing[1]);
    matrix->SetElement(i, 2, matrix->GetElement(i, 2) / spacing[2]);
  }
  return matrix;
}
