#include "vtkMaskedGlyph2D.h"

#include "vtkMaskPoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkMaskedGlyph2D);
vtkCxxRevisionMacro(vtkMaskedGlyph2D, "");

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
  if(this->MaskPoints)
    {
    this->MaskPoints->Delete();
    }
}

void vtkMaskedGlyph2D::SetInput(vtkDataSet *input)
{
  this->MaskPoints->SetInput(input);
  this->Superclass::SetInput(this->MaskPoints->GetOutput());
}

void vtkMaskedGlyph2D::SetRandomMode(int mode)
{
  this->MaskPoints->SetRandomMode(mode);
}

int vtkMaskedGlyph2D::GetRandomMode()
{
  return this->MaskPoints->GetRandomMode();
}

void vtkMaskedGlyph2D::Execute()
{
  if (this->UseMaskPoints)
    {
    this->Superclass::SetInput(this->MaskPoints->GetOutput());
    vtkIdType numPts = this->MaskPoints->GetPolyDataInput(0)->GetNumberOfPoints();
    this->MaskPoints->SetMaximumNumberOfPoints(MaximumNumberOfPoints);
    this->MaskPoints->SetOnRatio(numPts / MaximumNumberOfPoints);
    this->MaskPoints->Update();
    }
  else
    {
    this->Superclass::SetInput(this->MaskPoints->GetInput());
    }
  
  this->Superclass::Execute();
}

void vtkMaskedGlyph2D::PrintSelf(ostream& os, vtkIndent indent)
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
