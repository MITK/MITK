/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "vtkMaskedGlyph2D.h"

#include "vtkMaskPoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkMaskedGlyph2D);

vtkMaskedGlyph2D::vtkMaskedGlyph2D()
{
  this->SetColorModeToColorByScalar();
  this->SetScaleModeToScaleByVector();
  this->MaskPoints = vtkMaskPoints::New();
  this->MaximumNumberOfPoints = 5000;
  this->UseMaskPoints = 1;
}

vtkMaskedGlyph2D::~vtkMaskedGlyph2D()
{
  if (this->MaskPoints)
  {
    this->MaskPoints->Delete();
  }
}

void vtkMaskedGlyph2D::SetInput(vtkDataSet *input)
{
  this->MaskPoints->SetInputData(input);
  this->Superclass::SetInputConnection(this->MaskPoints->GetOutputPort());
}

void vtkMaskedGlyph2D::SetRandomMode(int mode)
{
  this->MaskPoints->SetRandomMode(mode);
}

int vtkMaskedGlyph2D::GetRandomMode()
{
  return this->MaskPoints->GetRandomMode();
}

int vtkMaskedGlyph2D::RequestData(vtkInformation *info,
                                  vtkInformationVector **inInfoVec,
                                  vtkInformationVector *outInfoVec)
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

  return this->Superclass::RequestData(info, inInfoVec, outInfoVec);
}

void vtkMaskedGlyph2D::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // os << indent << "InputScalarsSelection: "
  //   << (this->InputScalarsSelection ? this->InputScalarsSelection : "(none)")
  //   << endl;

  // os << indent << "InputVectorsSelection: "
  //   << (this->InputVectorsSelection ? this->InputVectorsSelection : "(none)")
  //   << endl;

  // os << indent << "InputNormalsSelection: "
  //   << (this->InputNormalsSelection ? this->InputNormalsSelection : "(none)")
  //   << endl;

  os << indent << "MaximumNumberOfPoints: " << this->GetMaximumNumberOfPoints() << endl;

  os << indent << "UseMaskPoints: " << (this->UseMaskPoints ? "on" : "off") << endl;
}
