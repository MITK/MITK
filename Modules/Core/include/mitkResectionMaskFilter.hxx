#pragma once
/*******************************************************************************
*
* This file is a part of AGTK
*
*
* Copyright 2016, SMedX LLC
*
******************************************************************************/

#include <vtkMath.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>

#include <mitkImplicitFieldImageFilter.h>

namespace mitk
{
template <typename TImageType>
ResectionMaskFilter<TImageType>::ResectionMaskFilter()
{
  m_Logger = itk::Logger::New();
  m_ImplicitDistance = mitk::ImplicitPolyDataSweepDistance::New();
  m_ProgressAccumulator = itk::ProgressAccumulator::New();
}

template <typename TImageType>
ResectionMaskFilter<TImageType>::~ResectionMaskFilter()
{
}

template <typename TImageType>
void ResectionMaskFilter<TImageType>::GenerateData()
{
  typename TImageType::ConstPointer input = this->GetInput();
  m_Logger->Info("Start\n");
  if (!input) {
    m_Logger->Info("Initial image wasn't defined\n");
    return;
  }
  m_ProgressAccumulator->ResetProgress();
  m_ProgressAccumulator->SetMiniPipelineFilter(this);
  m_ProgressAccumulator->UnregisterAllFilters();

  if (setupIntersection()) {
    computeIntersection();
  } else {
    m_Logger->Error("PolyData was not defined\n");
  }

  m_ProgressAccumulator->SetMiniPipelineFilter(nullptr);
}

template <typename TImageType>
bool ResectionMaskFilter<TImageType>::setupIntersection()
{
  if (!m_InputPoints) {
    return false;
  }

  int numberOfPoints = m_InputPoints->GetNumberOfPoints();
  if (numberOfPoints > 2) {
    m_ImplicitDistance->setViewportMatrix(m_ViewportMatrix);
    m_ImplicitDistance->SetInputPolyPoints(m_InputPoints);
  }
  return true;
}

template <typename TImageType>
void ResectionMaskFilter<TImageType>::computeIntersection()
{
  typename TImageType::ConstPointer input = this->GetInput();

  typename TImageType::RegionType region = input->GetLargestPossibleRegion();

  using ImplicitFieldFilter = ImplicitFieldImageFilter<TImageType>;
  typename ImplicitFieldFilter::Pointer deform = ImplicitFieldFilter::New();
  m_ProgressAccumulator->RegisterInternalFilter(deform, 0.5);
  deform->SetInput(input);
  deform->SetRegion(region);
  deform->SetImplicitFunction(m_ImplicitDistance);
  if (!itk::InPlaceImageFilter<TImageType, itk::Image<char, 3>>::GetInPlace()) {
    deform->InPlaceOff();
  }
  deform->Update();

  this->GraftOutput(deform->GetOutput());
}

template <typename TImageType>
void ResectionMaskFilter<TImageType>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << std::endl;
  m_Logger->Print(os, indent);
  os << std::endl;
}

template <typename TImageType>
void ResectionMaskFilter<TImageType>::setViewportMatrix(vtkMatrix4x4* matrix)
{
  m_ViewportMatrix = matrix;
}
}

