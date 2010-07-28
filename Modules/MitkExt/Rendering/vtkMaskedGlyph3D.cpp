/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMaskedGlyph3D.h"

#include "vtkMaskPoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkMaskedGlyph3D);
vtkCxxRevisionMacro(vtkMaskedGlyph3D, "");

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
  if(this->MaskPoints)
    {
    this->MaskPoints->Delete();
    }
}

void vtkMaskedGlyph3D::SetInput(vtkDataSet *input)
{
  this->MaskPoints->SetInput(input);
  this->Superclass::SetInputConnection(this->MaskPoints->GetOutputPort());
}

void vtkMaskedGlyph3D::SetInputConnection(vtkAlgorithmOutput* input)
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

void vtkMaskedGlyph3D::Execute()
{
  if (this->UseMaskPoints)
    {
    vtkIdType numPts = this->MaskPoints->GetPolyDataInput(0)->GetNumberOfPoints();
    this->MaskPoints->SetMaximumNumberOfPoints( MaximumNumberOfPoints );
    this->MaskPoints->SetOnRatio( numPts / MaximumNumberOfPoints );
    this->MaskPoints->Update();
    this->Superclass::SetInput(this->MaskPoints->GetOutput());
    }
  else
    {
    this->Superclass::SetInput(this->MaskPoints->GetInput());
    }
  
  this->Superclass::Execute();
}

int vtkMaskedGlyph3D::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if (this->UseMaskPoints)
  {
    this->Superclass::SetInput(this->MaskPoints->GetOutput());
    vtkIdType numPts = this->MaskPoints->GetPolyDataInput(0)->GetNumberOfPoints();
    this->MaskPoints->SetMaximumNumberOfPoints( MaximumNumberOfPoints );
    this->MaskPoints->SetOnRatio( numPts / MaximumNumberOfPoints );
    this->MaskPoints->Update();
  }
  else
  {
    this->Superclass::SetInput(this->MaskPoints->GetInput());
  }

  return this->Superclass::RequestData(
    request,inputVector,outputVector);
}

void vtkMaskedGlyph3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "MaximumNumberOfPoints: " << this->GetMaximumNumberOfPoints()
     << endl;

  os << indent << "UseMaskPoints: " << (this->UseMaskPoints?"on":"off") << endl;
}
