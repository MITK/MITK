/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "vtkMaskedGlyph3D.h"

#include "vtkMaskPoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkMaskedGlyph3D);

vtkMaskedGlyph3D::vtkMaskedGlyph3D()
{
  this->SetColorModeToColorByScalar();
  this->SetScaleModeToScaleByVector();
  this->MaskPoints = vtkMaskPoints::New();
  this->MaximumNumberOfPoints = 5000;
  this->UseMaskPoints = 1;
}

vtkMaskedGlyph3D::~vtkMaskedGlyph3D()
{
  if (this->MaskPoints)
  {
    this->MaskPoints->Delete();
  }
}

void vtkMaskedGlyph3D::SetInput(vtkDataSet *input)
{
  this->MaskPoints->SetInputData(input);
  this->Superclass::SetInputConnection(this->MaskPoints->GetOutputPort());
}

void vtkMaskedGlyph3D::SetInputConnection(vtkAlgorithmOutput *input)
{
  this->MaskPoints->SetInputConnection(input);
  this->Superclass::SetInputConnection(this->MaskPoints->GetOutputPort());
}

void vtkMaskedGlyph3D::SetRandomMode(int mode)
{
  this->MaskPoints->SetRandomMode(mode);
}

int vtkMaskedGlyph3D::GetRandomMode()
{
  return this->MaskPoints->GetRandomMode();
}

int vtkMaskedGlyph3D::RequestData(vtkInformation *request,
                                  vtkInformationVector **inputVector,
                                  vtkInformationVector *outputVector)
{
  if (this->UseMaskPoints)
  {
    this->Superclass::SetInputConnection(this->MaskPoints->GetOutputPort());
    vtkIdType numPts = this->MaskPoints->GetPolyDataInput(0)->GetNumberOfPoints();
    this->MaskPoints->SetMaximumNumberOfPoints(MaximumNumberOfPoints);
    this->MaskPoints->SetOnRatio(numPts / MaximumNumberOfPoints);
    this->MaskPoints->Update();
  }
  else
  {
    this->Superclass::SetInputData(this->MaskPoints->GetInput());
  }

  return this->Superclass::RequestData(request, inputVector, outputVector);
}

void vtkMaskedGlyph3D::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "MaximumNumberOfPoints: " << this->GetMaximumNumberOfPoints() << endl;

  os << indent << "UseMaskPoints: " << (this->UseMaskPoints ? "on" : "off") << endl;
}
