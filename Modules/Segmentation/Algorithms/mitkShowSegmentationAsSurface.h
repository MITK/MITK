/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#pragma once

#include <MitkSegmentationExports.h>

#include <itkImage.h>
#include <itkProcessObject.h>
#include <itkProgressAccumulator.h>

#include <vtkPolyData.h>

namespace mitk
{

class MITKSEGMENTATION_EXPORT ShowSegmentationAsSurface : public itk::ProcessObject
{

public:
  struct SurfaceComputingParameters
  {
    unsigned short medianKernelSize = 3u;
    bool applyMedian                = true;
    bool smooth                     = true;
    float gaussianSD                = 1.5f;
    bool decimateMesh               = true;
    float decimationRate            = .2f;
  };

  typedef ShowSegmentationAsSurface     Self;
  typedef itk::ProcessObject            Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro(ShowSegmentationAsSurface, itk::ProcessObject);

  static const unsigned int DIM = 3;
  typedef itk::Image<unsigned char, 3> InputImageType;

  itkGetObjectMacro(Output, vtkPolyData);

  void SetInput(InputImageType::Pointer input)
  {
    m_Input = input;
  }

  void setArgs(SurfaceComputingParameters args)
  {
    m_Args = args;
  }

  virtual void Update() override { this->UpdateOutputData(nullptr); }

protected:
  ShowSegmentationAsSurface();
  virtual ~ShowSegmentationAsSurface();

  virtual void GenerateData() ITK_OVERRIDE;

  InputImageType::Pointer m_Input;
  vtkSmartPointer<vtkPolyData> m_Output;

  SurfaceComputingParameters m_Args;

private:
  itk::ProgressAccumulator::Pointer m_ProgressAccumulator;

};

} // namespace


