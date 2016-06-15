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

#include "vtkMaskedProgrammableGlyphFilter.h"

#include "vtkMaskPoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkMaskedProgrammableGlyphFilter);

vtkMaskedProgrammableGlyphFilter::vtkMaskedProgrammableGlyphFilter()
{
  //this->SetColorModeToColorByScalar();
  //this->SetScaleModeToScaleByVector();
  this->MaskPoints = vtkMaskPoints::New();
  this->MaximumNumberOfPoints = 5000;
  this->UseMaskPoints = 1;
}

vtkMaskedProgrammableGlyphFilter::~vtkMaskedProgrammableGlyphFilter()
{
  if(this->MaskPoints)
    {
    this->MaskPoints->Delete();
    }
}

void vtkMaskedProgrammableGlyphFilter::SetInput(vtkDataSet *input)
{
  this->MaskPoints->SetInputData(input);
  this->Superclass::SetInputData(this->MaskPoints->GetOutput());
}

void vtkMaskedProgrammableGlyphFilter::SetRandomMode(int mode)
{
  this->MaskPoints->SetRandomMode(mode);
}

int vtkMaskedProgrammableGlyphFilter::GetRandomMode()
{
  return this->MaskPoints->GetRandomMode();
}

int vtkMaskedProgrammableGlyphFilter::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if (this->UseMaskPoints)
  {
    vtkIdType numPts = this->MaskPoints->GetPolyDataInput(0)->GetNumberOfPoints();
    this->MaskPoints->SetMaximumNumberOfPoints(MaximumNumberOfPoints);
    this->MaskPoints->SetOnRatio(numPts / MaximumNumberOfPoints);
    this->Superclass::SetInputConnection(this->MaskPoints->GetOutputPort());
    this->MaskPoints->Update();
  }
  else
  {
    this->Superclass::SetInputData(this->MaskPoints->GetInput());
  }

  return this->Superclass::RequestData(
    request,inputVector,outputVector);
}


void vtkMaskedProgrammableGlyphFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  //os << indent << "InputScalarsSelection: "
  //   << (this->InputScalarsSelection ? this->InputScalarsSelection : "(none)")
  //   << endl;

  //os << indent << "InputVectorsSelection: "
  //   << (this->InputVectorsSelection ? this->InputVectorsSelection : "(none)")
  //   << endl;

  //os << indent << "InputNormalsSelection: "
  //   << (this->InputNormalsSelection ? this->InputNormalsSelection : "(none)")
  //   << endl;

  os << indent << "MaximumNumberOfPoints: " << this->GetMaximumNumberOfPoints()
     << endl;

  os << indent << "UseMaskPoints: " << (this->UseMaskPoints?"on":"off") << endl;
}
