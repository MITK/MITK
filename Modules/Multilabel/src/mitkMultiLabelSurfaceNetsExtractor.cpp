/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSurfaceNetsExtractor.h>

#include <vtkImageData.h>
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
  m_SurfaceNets->SetSmoothing(m_Smoothing);
  m_SurfaceNets->SetOutputStyleToDefault();
  m_SurfaceNets->InitializeSelectedLabelsList();
  m_SurfaceNets->SetNumberOfLabels(static_cast<int>(labelValues.size()));

  for (size_t i = 0; i < labelValues.size(); ++i)
    m_SurfaceNets->SetLabel(static_cast<int>(i), static_cast<double>(labelValues[i]));

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
  m_SurfaceNets->SetSmoothing(m_Smoothing);
  m_SurfaceNets->SetNumberOfLabels(static_cast<int>(labelValues.size()));

  for (size_t i = 0; i < labelValues.size(); ++i)
    m_SurfaceNets->SetLabel(static_cast<int>(i), static_cast<double>(labelValues[i]));

  m_SurfaceNets->SetOutputStyleToSelected();

  for (auto label : labelValues)
  {
    m_SurfaceNets->InitializeSelectedLabelsList();
    m_SurfaceNets->AddSelectedLabel(static_cast<double>(label));
    m_SurfaceNets->Modified();

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    m_NormalsFilter->Update();
    polydata->DeepCopy(m_NormalsFilter->GetOutput());

    if (polydata->GetNumberOfCells() > 0)
      results[label] = polydata;
  }

  return results;
}
