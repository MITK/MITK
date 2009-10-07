#include "vtkMaskedProgrammableGlyphFilter.h"

#include "vtkMaskPoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkMaskedProgrammableGlyphFilter);
vtkCxxRevisionMacro(vtkMaskedProgrammableGlyphFilter, "");

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
  this->MaskPoints->SetInput(input);
  this->Superclass::SetInput(this->MaskPoints->GetOutput());
}

void vtkMaskedProgrammableGlyphFilter::SetRandomMode(int mode)
{
  this->MaskPoints->SetRandomMode(mode);
}

int vtkMaskedProgrammableGlyphFilter::GetRandomMode()
{
  return this->MaskPoints->GetRandomMode();
}

void vtkMaskedProgrammableGlyphFilter::Execute()
{
  if (this->UseMaskPoints)
    {
    this->Superclass::SetInput(this->MaskPoints->GetOutput());
    vtkIdType numPts = this->MaskPoints->GetPolyDataInput(0)->GetNumberOfPoints();
    this->MaskPoints->SetMaximumNumberOfPoints(MaximumNumberOfPoints);
    double onRatio = MaximumNumberOfPoints != 0.0 ? numPts / MaximumNumberOfPoints : 1.0;
    this->MaskPoints->SetOnRatio(onRatio);
    this->MaskPoints->Update();
    }
  else
    {
      this->Superclass::SetInput(this->MaskPoints->GetInput());
    }
  
  this->Superclass::Execute();
}

int vtkMaskedProgrammableGlyphFilter::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
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
