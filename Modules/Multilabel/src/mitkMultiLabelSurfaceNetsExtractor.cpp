/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSurfaceNetsExtractor.h>

#include <mitkBaseGeometry.h>

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSurfaceNets3D.h>

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
}

mitk::MultiLabelSurfaceNetsExtractor::MultiLabelSurfaceNetsExtractor()
  : m_SurfaceNets(vtkSmartPointer<vtkSurfaceNets3D>::New()),
    m_NormalsFilter(vtkSmartPointer<vtkPolyDataNormals>::New()),
    m_Smoothing(true)
{
  m_SurfaceNets->SetOutputMeshTypeToTriangles();

  m_NormalsFilter->SetInputConnection(m_SurfaceNets->GetOutputPort());
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

void mitk::MultiLabelSurfaceNetsExtractor::ConfigureLabels(const std::vector<LabelValueType>& labelValues)
{
  m_SurfaceNets->SetSmoothing(m_Smoothing);
  m_SurfaceNets->SetNumberOfLabels(static_cast<int>(labelValues.size()));
  for (size_t i = 0; i < labelValues.size(); ++i)
    m_SurfaceNets->SetLabel(static_cast<int>(i), static_cast<double>(labelValues[i]));
}

vtkSmartPointer<vtkPolyData> mitk::MultiLabelSurfaceNetsExtractor::Extract(
  vtkImageData* groupImage,
  const std::vector<LabelValueType>& labelValues)
{
  auto output = vtkSmartPointer<vtkPolyData>::New();

  if (groupImage == nullptr || labelValues.empty())
  {
    return output;
  }

  PrepareInput(groupImage);

  m_SurfaceNets->SetInputData(groupImage);
  this->ConfigureLabels(labelValues);
  m_SurfaceNets->InitializeSelectedLabelsList();
  m_SurfaceNets->SetOutputStyleToDefault();
  // TODO(VTK 9.5.2): force re-extraction. vtkSurfaceNets3D::RequestData reuses its
  // cache based on Superclass::GetMTime(), which SetLabel/SetInputData do not always
  // bump; the cached path then leaves newScalars null and TransformMeshType crashes.
  // Drop this Modified() when VTK is upgraded past 9.5.2.
  m_SurfaceNets->Modified();

  m_NormalsFilter->Update();
  output->DeepCopy(m_NormalsFilter->GetOutput());

  return output;
}

std::map<mitk::MultiLabelSurfaceNetsExtractor::LabelValueType, vtkSmartPointer<vtkPolyData>>
mitk::MultiLabelSurfaceNetsExtractor::ExtractPerLabel(
  vtkImageData* groupImage,
  const std::vector<LabelValueType>& labelValues)
{
  std::map<LabelValueType, vtkSmartPointer<vtkPolyData>> results;
  if (groupImage == nullptr || labelValues.empty())
  {
    return results;
  }

  PrepareInput(groupImage);

  m_SurfaceNets->SetInputData(groupImage);
  this->ConfigureLabels(labelValues);
  m_SurfaceNets->SetOutputStyleToSelected();

  for (auto label : labelValues)
  {
    m_SurfaceNets->InitializeSelectedLabelsList();
    m_SurfaceNets->AddSelectedLabel(static_cast<double>(label));
    m_SurfaceNets->Modified(); // VTK 9.5.2 cache bust, see Extract().

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    m_NormalsFilter->Update();
    polydata->DeepCopy(m_NormalsFilter->GetOutput());

    if (polydata->GetNumberOfCells() > 0)
      results[label] = polydata;
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
